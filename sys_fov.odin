package main

import "core:math"

sys_update_fov :: proc(gs: ^Game_State) {
	player := gs.world.entities[gs.player_id]
	px := player.position.x
	py := player.position.y
	radius := gs.entropy.fov_radius

	for x in 0 ..< MAP_WIDTH {
		for y in 0 ..< MAP_HEIGHT {
			gs.game_map.tiles[x][y].visible = false
		}
	}

	gs.game_map.tiles[px][py].visible = true
	gs.game_map.tiles[px][py].explored = true

	start_x := clamp(px - radius, 0, MAP_WIDTH - 1)
	end_x := clamp(px + radius, 0, MAP_WIDTH - 1)
	start_y := clamp(py - radius, 0, MAP_HEIGHT - 1)
	end_y := clamp(py + radius, 0, MAP_HEIGHT - 1)

	for x in start_x ..= end_x {
		for y in start_y ..= end_y {
			if x == px && y == py do continue

			dx := x - px
			dy := y - py
			dist_sq := dx * dx + dy * dy
			if dist_sq > radius * radius do continue
			if check_los(&gs.game_map, px, py, x, y) {
				gs.game_map.tiles[x][y].visible = true
				gs.game_map.tiles[x][y].explored = true
			}
		}
	}
}

check_los :: proc(m: ^Map, x0, y0, x1, y1: int) -> bool {
	dx := f32(x1 - x0)
	dy := f32(y1 - y0)
	steps := int(math.max(math.abs(dx), math.abs(dy)))

	x_inc := dx / f32(steps)
	y_inc := dy / f32(steps)

	cx := f32(x0)
	cy := f32(y0)
	for i in 0 ..< steps {
		cx += x_inc
		cy += y_inc

		check_x := int(math.round(cx))
		check_y := int(math.round(cy))
		if check_x == x1 && check_y == y1 do return true
		if m.tiles[check_x][check_y].type == .Wall {
			return false
		}
	}
	return true
}

