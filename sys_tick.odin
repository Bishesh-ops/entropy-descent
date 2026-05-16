// sys_tick.odin
package main

sys_tick :: proc(gs: ^Game_State) {
	action := gs.player_action

	// 1. Advance global tick counter
	gs.tick_count += action.cost

	// 2. Execute player action
	// (no #partial switch here — use full switch, or just if)
	switch action.type {
	case .Move:
		try_move(gs, gs.player_id, action.direction.x, action.direction.y)
	case .Melee_Attack:
	// placeholder
	case .Wait:
	// nothing
	case .None:
	// shouldn't happen
	}

	// 3. Enemy turns
	max_iters := 1000
	for _ in 0 ..< max_iters {
		acted := false
		for id in 0 ..< len(gs.world.entities) {
			enemy := &gs.world.entities[id]
			if .Enemy not_in enemy.components {
				continue
			}
			if enemy.next_action_tick <= gs.tick_count {
				enemy_take_turn(gs, Entity_ID(id))
				enemy.next_action_tick += enemy.tick_threshold
				acted = true
				break
			}
		}
		if !acted {
			break
		}
	}

	// 4. Passive entropy rise
	if gs.entropy.tick_rate > 0 && gs.tick_count % gs.entropy.tick_rate == 0 {
		gs.entropy.entropy += 1
	}
}

enemy_take_turn :: proc(gs: ^Game_State, enemy_id: Entity_ID) {
	enemy := &gs.world.entities[enemy_id]
	player := &gs.world.entities[gs.player_id]

	dx := player.position.x - enemy.position.x
	dy := player.position.y - enemy.position.y

	if abs(dx) + abs(dy) == 1 {
		// Melee attack placeholder
		return
	}

	// Move toward player
	move_x := 0
	move_y := 0
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
	else if x < 0 do return -1
	return 0
}
abs :: proc(x: int) -> int {
	if x < 0 do return -x
	return x
}

