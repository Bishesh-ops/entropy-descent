package main
import sdl "vendor:sdl3"

sys_render_map :: proc(renderer: ^sdl.Renderer, game_map: ^Map, cam: Camera) {
	for x in 0 ..< MAP_WIDTH {
		for y in 0 ..< MAP_HEIGHT {
			tile := game_map.tiles[x][y]
			if !tile.explored do continue

			rect := sdl.FRect {
				x = f32(x * TILE_SIZE) - cam.x,
				y = f32(y * TILE_SIZE) - cam.y,
				w = f32(TILE_SIZE),
				h = f32(TILE_SIZE),
			}

			if rect.x > f32(CAMERA_VIEW_W) || rect.y > f32(CAMERA_VIEW_H) do continue
			if rect.x + rect.w < 0 || rect.y + rect.h < 0 do continue

			base_r, base_g, base_b: u8
			switch tile.type {
			case .Floor:
				base_r, base_g, base_b = 40, 40, 45
			case .Wall:
				base_r, base_g, base_b = 95, 95, 105
			}

			factor: f32 = 1.0 if tile.visible else 0.25

			sdl.SetRenderDrawColor(
				renderer,
				u8(f32(base_r) * factor),
				u8(f32(base_g) * factor),
				u8(f32(base_b) * factor),
				255,
			)
			sdl.RenderFillRect(renderer, &rect)

			grid_alpha := u8(16) if tile.visible else u8(5)
			sdl.SetRenderDrawColor(renderer, 60, 60, 60, grid_alpha)
			sdl.RenderRect(renderer, &rect)
		}
	}
}

sys_render_entities :: proc(renderer: ^sdl.Renderer, world: ^World, cam: Camera, gs: ^Game_State) {
	for &entity in world.entities {
		if !entity.active do continue
		if .Pending_Destroy in entity.components do continue
		if .Render_Color not_in entity.components do continue

		if .Enemy in entity.components {
			if !gs.game_map.tiles[entity.position.x][entity.position.y].visible do continue
		}

		sx := entity.transform.x - cam.x
		sy := entity.transform.y - cam.y

		if .Player in entity.components && gs.player_texture != nil {
			dst := sdl.FRect {
				x = sx,
				y = sy,
				w = 32,
				h = 32,
			}
			sdl.SetRenderDrawBlendMode(renderer, cast(sdl.BlendMode)sdl.BLENDMODE_BLEND)
			sdl.RenderTexture(renderer, gs.player_texture, nil, &dst)

			facing := entity.facing
			dot := sdl.FRect {
				x = sx + f32(TILE_SIZE) / 2 + f32(facing.x) * 10 - 1,
				y = sy + f32(TILE_SIZE) / 2 + f32(facing.y) * 10 - 1,
				w = 2,
				h = 2,
			}
			sdl.SetRenderDrawColor(renderer, 255, 255, 180, 200)
			sdl.RenderFillRect(renderer, &dot)

		} else {
			// Draw enemy body
			rect := sdl.FRect {
				x = sx,
				y = sy,
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

			if .Health in entity.components && entity.max_health > 0 {
				hp_pct := f32(entity.health) / f32(entity.max_health)
				bar_w := entity.hitbox.w
				bar_h: f32 = 2

				bg := sdl.FRect {
					x = sx,
					y = sy - 4,
					w = bar_w,
					h = bar_h,
				}
				sdl.SetRenderDrawColor(renderer, 50, 10, 10, 255)
				sdl.RenderFillRect(renderer, &bg)

				fill := sdl.FRect {
					x = sx,
					y = sy - 4,
					w = bar_w * hp_pct,
					h = bar_h,
				}
				sdl.SetRenderDrawColor(renderer, 220, 60, 60, 255)
				sdl.RenderFillRect(renderer, &fill)
			}
		}
	}
}

