// sys_input.odin
package main

import sdl "vendor:sdl3"

event_loop :: proc(gs: ^Game_State, running: ^bool) {
	for {
		ev: sdl.Event
		if !sdl.PollEvent(&ev) {
			break
		}
		#partial switch ev.type {
		case .QUIT:
			running^ = false
		case .KEY_DOWN:
			handle_key_down(gs, ev.key)
		}
	}
}

handle_key_down :: proc(gs: ^Game_State, key: sdl.KeyboardEvent) {
	if key.repeat {
		return
	}

	dir := [2]int{0, 0}
	action_type := Action_Type.None
	cost := 0

	#partial switch key.scancode {
	case .W, .UP:
		dir = {0, -1}; action_type = .Move; cost = 1
	case .S, .DOWN:
		dir = {0, 1}; action_type = .Move; cost = 1
	case .A, .LEFT:
		dir = {-1, 0}; action_type = .Move; cost = 1
	case .D, .RIGHT:
		dir = {1, 0}; action_type = .Move; cost = 1
	case .SPACE:
		action_type = .Wait; cost = 1
	case .E:
		action_type = .Melee_Attack; cost = 2
		dir = {0, 0} // placeholder
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

