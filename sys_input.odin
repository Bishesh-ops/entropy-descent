package main
import sdl "vendor:sdl3"

event_loop :: proc(gs: ^Game_State, running: ^bool) {
	for {
		ev: sdl.Event
		if !sdl.PollEvent(&ev) do break
		#partial switch ev.type {
		case .QUIT:
			running^ = false
		case .KEY_DOWN:
			handle_key_down(gs, ev.key, running)
		}
	}
}

handle_key_down :: proc(gs: ^Game_State, key: sdl.KeyboardEvent, running: ^bool) {
	if key.repeat do return

	if gs.game_over {
		#partial switch key.scancode {
		case .R:
			restart_game(gs)
		case .ESCAPE:
			running^ = false
		}
		return
	}

	dir := [2]int{0, 0}
	action_type := Action_Type.None
	cost := 0

	#partial switch key.scancode {
	case .W, .UP:
		dir = {0, -1}; action_type = .Move; cost = 1
		gs.world.entities[gs.player_id].facing = {0, -1}
	case .S, .DOWN:
		dir = {0, 1}; action_type = .Move; cost = 1
		gs.world.entities[gs.player_id].facing = {0, 1}
	case .A, .LEFT:
		dir = {-1, 0}; action_type = .Move; cost = 1
		gs.world.entities[gs.player_id].facing = {-1, 0}
	case .D, .RIGHT:
		dir = {1, 0}; action_type = .Move; cost = 1
		gs.world.entities[gs.player_id].facing = {1, 0}
	case .SPACE:
		action_type = .Wait; cost = 1
	case .E:
		action_type = .Melee_Attack; cost = 2
		dir = gs.world.entities[gs.player_id].facing
	case .ESCAPE:
		running^ = false
	}

	if action_type != .None {
		gs.player_action = Action {
			type      = action_type,
			direction = dir,
			cost      = cost,
		}
		gs.has_action = true
	}
}

