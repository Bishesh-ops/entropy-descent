package main

sys_movement :: proc(world: ^World, dt: f32) {
	for e, i in world.entities {
		if !e.active do continue

		if .Transform in e.components && .Velocity in e.components && .Position in e.components {
			world.entities[i].transform.x += e.vel.dx * e.vel.speed * dt
			world.entities[i].transform.y += e.vel.dy * e.vel.speed * dt

			world.entities[i].pos.x = int(world.entities[i].transform.x)
			world.entities[i].pos.y = int(world.entities[i].transform.y)
		}
	}
}

