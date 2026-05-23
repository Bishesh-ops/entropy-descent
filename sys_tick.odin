package main

sys_tick :: proc(gs: ^Game_State) {
	action := gs.player_action
	gs.tick_count += action.cost

	switch action.type {
	case .Move:
		target_x := gs.world.entities[gs.player_id].position.x + action.direction.x
		target_y := gs.world.entities[gs.player_id].position.y + action.direction.y

		bumped_id := -1
		for id in 0 ..< len(gs.world.entities) {
			if !gs.world.entities[id].active do continue
			if .Enemy not_in gs.world.entities[id].components do continue
			if gs.world.entities[id].position.x == target_x &&
			   gs.world.entities[id].position.y == target_y {
				bumped_id = id
				break
			}
		}

		if bumped_id != -1 {
			damage := gs.world.entities[gs.player_id].attack
			gs.world.entities[bumped_id].health -= damage

			gs.screen_shake = 0.15
			spawn_particle_burst(gs, target_x, target_y, {255, 255, 200, 255}, 10) // Impact sparks

			if gs.world.entities[bumped_id].health <= 0 {
				gs.world.entities[bumped_id].components += {.Pending_Destroy}
				spawn_particle_burst(gs, target_x, target_y, {200, 40, 40, 255}, 25) // Blood burst
			}
		} else {
			try_move(gs, gs.player_id, action.direction.x, action.direction.y)
		}
	case .Melee_Attack:
	case .Spell:
	case .Wait:
	case .None:
	}
	loop_safeguard := 0
	safety_limit := len(gs.world.entities) * 4

	for {
		acted := false
		for id in 0 ..< len(gs.world.entities) {
			if !gs.world.entities[id].active do continue
			if .Enemy not_in gs.world.entities[id].components do continue

			if gs.world.entities[id].next_action_tick <= gs.tick_count {
				enemy_take_turn(gs, Entity_ID(id))
				gs.world.entities[id].next_action_tick += gs.world.entities[id].tick_threshold
				acted = true
				break
			}
		}

		if !acted do break

		loop_safeguard += 1
		if loop_safeguard >= safety_limit {
			break
		}
	}

	if gs.entropy.tick_rate > 0 && gs.tick_count % gs.entropy.tick_rate == 0 {
		gs.entropy.entropy -= 1
		if gs.entropy.entropy < 0 do gs.entropy.entropy = 0
		update_entropy_tier(gs)
	}
	if gs.entropy.entropy >= gs.entropy.max_entropy {
		trigger_entropy_event(gs)
		update_entropy_tier(gs)
	}
}

enemy_take_turn :: proc(gs: ^Game_State, enemy_id: Entity_ID) {
	player := gs.world.entities[gs.player_id]
	enemy := gs.world.entities[enemy_id]

	dx := player.position.x - enemy.position.x
	dy := player.position.y - enemy.position.y

	if abs(dx) + abs(dy) == 1 {
		damage := enemy.attack
		gs.world.entities[gs.player_id].health -= damage
		gs.screen_shake = 0.25
		gs.flash = {
			timer = 0.15,
			r     = 255,
			g     = 0,
			b     = 0,
		}

		if gs.world.entities[gs.player_id].health <= 0 {
			gs.game_over = true
		}
		return
	}

	if !gs.game_map.tiles[enemy.position.x][enemy.position.y].visible {
		return // Enemy stands still (or wanders) in the darkness
	}

	move_x, move_y, found := find_next_step(
		gs,
		enemy.position.x,
		enemy.position.y,
		player.position.x,
		player.position.y,
	)

	if found {
		try_move(gs, enemy_id, move_x, move_y)
	}
}

find_next_step :: proc(gs: ^Game_State, ex, ey, px, py: int) -> (int, int, bool) {
	visited := [MAP_WIDTH][MAP_HEIGHT]bool{}
	parent := [MAP_WIDTH][MAP_HEIGHT][2]int{}

	queue: [dynamic][2]int
	defer delete(queue)

	append(&queue, [2]int{ex, ey})
	visited[ex][ey] = true

	head := 0
	found := false
	dirs := [4][2]int{{0, -1}, {0, 1}, {-1, 0}, {1, 0}}

	for head < len(queue) {
		curr := queue[head]
		head += 1

		if curr[0] == px && curr[1] == py {
			found = true
			break
		}

		for d in dirs {
			nx := curr[0] + d[0]
			ny := curr[1] + d[1]

			if nx < 0 || nx >= MAP_WIDTH || ny < 0 || ny >= MAP_HEIGHT do continue
			if visited[nx][ny] do continue
			if gs.game_map.tiles[nx][ny].type == .Wall do continue

			visited[nx][ny] = true
			parent[nx][ny] = curr
			append(&queue, [2]int{nx, ny})
		}
	}

	if found {
		curr := [2]int{px, py}
		target := [2]int{ex, ey}

		for parent[curr[0]][curr[1]] != target {
			curr = parent[curr[0]][curr[1]]
		}
		return curr[0] - ex, curr[1] - ey, true
	}
	return 0, 0, false
}
sign :: proc(x: int) -> int {
	if x > 0 do return 1
	if x < 0 do return -1
	return 0
}

abs :: proc(x: int) -> int {
	if x < 0 do return -x
	return x
}
trigger_entropy_event :: proc(gs: ^Game_State) {
	gs.entropy.entropy = 0
	gs.entropy.max_entropy += 20

	gs.flash = {
		timer = 0.6,
		r     = 130,
		g     = 0,
		b     = 200,
	}
	gs.screen_shake = 0.45

	if gs.entropy.fov_radius > 3 {
		gs.entropy.fov_radius -= 1
	}

	px := gs.world.entities[gs.player_id].position.x
	py := gs.world.entities[gs.player_id].position.y

	spawn := find_spawn_point(&gs.game_map, px + 7, py + 7)

	eid := spawn_entity(&gs.world)
	e := &gs.world.entities[eid]
	e.position = spawn
	e.transform = {f32(spawn.x * TILE_SIZE), f32(spawn.y * TILE_SIZE)}
	e.render_color = {220, 40, 255, 255} // Glowing neon purple
	e.hitbox = {14, 14}

	e.health = 50
	e.max_health = 50
	e.attack = 18
	e.speed = 180
	e.tick_threshold = 1 // Moves almost every player turn
	e.components += {.Position, .Transform, .Render_Color, .Enemy, .Health, .Combat}
}

