package main

is_wall :: proc(m: ^Map, px_x, px_y: f32) -> bool {
	grid_x := int(px_x) / TILE_SIZE
	grid_y := int(px_y) / TILE_SIZE

	if grid_x < 0 || grid_x >= MAP_WIDTH || grid_y < 0 || grid_y >= MAP_HEIGHT {
		return true
	}
	return m.tiles[grid_x][grid_y] == .Wall
}

sys_movement :: proc(world: ^World, m: ^Map, dt: f32) {
	for e, i in world.entities {
		if !e.active do continue

		if .Transform in e.components &&
		   .Velocity in e.components &&
		   .Position in e.components &&
		   .Hitbox in e.components {

			next_x := e.transform.x + e.vel.dx * e.vel.speed * dt
			next_y := e.transform.y + e.vel.dy * e.vel.speed * dt

			collision_x := false
			if e.vel.dx != 0 {
				left := next_x + e.hitbox.offset_x
				right := next_x + e.hitbox.offset_x + e.hitbox.width - 0.1
				top := e.transform.y + e.hitbox.offset_y
				bottom := e.transform.y + e.hitbox.offset_y + e.hitbox.height - 0.1

				if e.vel.dx > 0 {
					if is_wall(m, right, top) || is_wall(m, right, bottom) do collision_x = true
				} else {
					if is_wall(m, left, top) || is_wall(m, left, bottom) do collision_x = true
				}
			}

			if !collision_x {
				world.entities[i].transform.x = next_x
			} else {
				world.entities[i].vel.dx = 0
			}

			collision_y := false
			if e.vel.dy != 0 {
				left := world.entities[i].transform.x + e.hitbox.offset_x
				right := world.entities[i].transform.x + e.hitbox.offset_x + e.hitbox.width - 0.1
				top := next_y + e.hitbox.offset_y
				bottom := next_y + e.hitbox.offset_y + e.hitbox.height - 0.1

				if e.vel.dy > 0 {
					if is_wall(m, left, bottom) || is_wall(m, right, bottom) do collision_y = true
				} else {
					if is_wall(m, left, top) || is_wall(m, right, top) do collision_y = true
				}
			}

			if !collision_y {
				world.entities[i].transform.y = next_y
			} else {
				world.entities[i].vel.dy = 0
			}
			world.entities[i].pos.x = int(world.entities[i].transform.x)
			world.entities[i].pos.y = int(world.entities[i].transform.y)
		}
	}
}

