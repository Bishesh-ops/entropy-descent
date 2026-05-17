package main

World :: struct {
	entities: #soa[dynamic]Entity,
}

spawn_entity :: proc(world: ^World) -> Entity_ID {
	for i in 0 ..< len(world.entities) {
		if !world.entities[i].active {
			world.entities[i] = Entity {
				active = true,
				hitbox = {16, 26},
			}
			return Entity_ID(i)
		}
	}
	append(&world.entities, Entity{active = true, hitbox = {16, 16}})
	return Entity_ID(len(world.entities) - 1)
}

process_destroys :: proc(world: ^World) {
	for i in 0 ..< len(world.entities) {
		if .Pending_Destroy in world.entities[i].components {
			world.entities[i] = Entity{}
		}
	}
}

