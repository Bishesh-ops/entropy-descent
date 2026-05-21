package main

import sdl "vendor:sdl3"

sys_render_map :: proc(renderer: ^sdl.Renderer, game_map: ^Map, cam: Camera) {
	for x in 0 ..< MAP_WIDTH {
		for y in 0 ..< MAP_HEIGHT {
			rect := sdl.FRect {
				x = f32(x * TILE_SIZE) - cam.x,
				y = f32(y * TILE_SIZE) - cam.y,
				w = f32(TILE_SIZE),
				h = f32(TILE_SIZE),
			}
			if rect.x > f32(GAME_W) || rect.y > f32(GAME_H) do continue
			if rect.x + rect.w < 0 || rect.y + rect.h < 0 do continue

			tile := game_map.tiles[x][y]
			switch tile.type {
			case .Floor:
				//				  R   G   B    A				
				sdl.SetRenderDrawColor(renderer, 40, 40, 40, 255)
			case .Wall:
				sdl.SetRenderDrawColor(renderer, 100, 100, 100, 255)
			}
			sdl.RenderFillRect(renderer, &rect)
			sdl.SetRenderDrawColor(renderer, 60, 60, 60, 18)
			sdl.RenderRect(renderer, &rect)
		}
	}
}

sys_render_entities :: proc(renderer: ^sdl.Renderer, world: ^World, cam: Camera) {
	for &entity in world.entities {
		if !entity.active do continue
		if .Render_Color not_in entity.components do continue
		if .Pending_Destroy in entity.components do continue

		rect := sdl.FRect {
			x = entity.transform.x - cam.x,
			y = entity.transform.y - cam.y,
			w = entity.hitbox.w,
			h = entity.hitbox.h,
		}
		sdl.SetRenderDrawColor(
			renderer,
			entity.render_color.r,
			entity.render_color.g,
			entity.render_color.b,
			entity.render_color.a,
		)
		sdl.RenderFillRect(renderer, &rect)
	}
}

