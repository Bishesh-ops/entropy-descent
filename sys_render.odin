package main

import sdl "vendor:sdl3"

sys_render :: proc(world: ^World, renderer: ^sdl.Renderer) {
	sdl.SetRenderDrawColor(renderer, 3, 21, 31, 255)
	sdl.RenderClear(renderer)

	for e in world.entities {
		if !e.active do continue

		if .Position in e.components && .Render_Color in e.components {
			rect := sdl.FRect {
				x = f32(e.pos.x),
				y = f32(e.pos.y),
				w = 16.0,
				h = 16.0,
			}

			if .Hitbox in e.components {
				rect.w = e.hitbox.width
				rect.h = e.hitbox.height
			}

			sdl.SetRenderDrawColor(renderer, e.color.r, e.color.g, e.color.b, e.color.a)
			sdl.RenderFillRect(renderer, &rect)
		}
	}

	sdl.RenderPresent(renderer)
}

