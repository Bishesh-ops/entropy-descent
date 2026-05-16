package main

import sdl "vendor:sdl3"

sys_input :: proc(world: ^World) {
	keyboard_state := sdl.GetKeyboardState(nil)

	for e, i in world.entities {
		if !e.active do continue

		if .Player in e.components && .Velocity in e.components {
			world.entities[i].vel.dx = 0
			world.entities[i].vel.dy = 0

			if keyboard_state[sdl.Scancode.W] != false ||
			   keyboard_state[sdl.Scancode.UP] != false {
				world.entities[i].vel.dy = -1
			}
			if keyboard_state[sdl.Scancode.S] != false ||
			   keyboard_state[sdl.Scancode.DOWN] != false {
				world.entities[i].vel.dy = 1
			}
			if keyboard_state[sdl.Scancode.A] != false ||
			   keyboard_state[sdl.Scancode.LEFT] != false {
				world.entities[i].vel.dx = -1
			}
			if keyboard_state[sdl.Scancode.D] != false ||
			   keyboard_state[sdl.Scancode.RIGHT] != false {
				world.entities[i].vel.dx = 1
			}
		}
	}
}

