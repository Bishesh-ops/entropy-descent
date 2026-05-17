package main

sys_tick :: proc(gs: ^Game_State) {
	action := gs.player_action

	gs.tick_count += action.cost

	switch action.type {
	case .Move:
		try_move(gs, gs.player_id, action.direction.x, action.direction.y)
	case .Melee_Attack:
		target_x := gs.world.entities[gs.player_id].position.x + action.direction.x
		target_y := gs.world.entities[gs.player_id].position.y + action.direction.y

		for id in 0 ..< len(gs.world.entities) {
			if !gs.world.entities[id].active do continue
			if .Enemy not_in gs.world.entities[id].components do continue

			if gs.world.entities[id].position.x == target_x &&
			   gs.world.entities[id].position.y == target_y {
				gs.world.entities[id].components += {.Pending_Destroy}
				spawn_particle_burst(gs, target_x, target_y, {255, 100, 100, 255}, 20)
				break
			}
		}
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
		gs.entropy.entropy += 1
	}}

enemy_take_turn :: proc(gs: ^Game_State, enemy_id: Entity_ID) {
	player_x := gs.world.entities[gs.player_id].position.x
	player_y := gs.world.entities[gs.player_id].position.y

	enemy_x := gs.world.entities[enemy_id].position.x
	enemy_y := gs.world.entities[enemy_id].position.y

	dx := player_x - enemy_x
	dy := player_y - enemy_y
	if abs(dx) + abs(dy) == 1 {
		return
	}

	move_x, move_y := 0, 0
	if abs(dx) > abs(dy) {
		move_x = sign(dx)
	} else {
		move_y = sign(dy)
	}
	if !try_move(gs, enemy_id, move_x, move_y) {
		if move_x != 0 {
			try_move(gs, enemy_id, 0, sign(dy))
		} else {
			try_move(gs, enemy_id, sign(dx), 0)
		}
	}
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

