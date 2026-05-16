package main

World :: struct {
	entities: [dynamic]Entity,
}

spawn_entity :: proc(world: ^World) -> Entity_ID {
	for &e, id in world.entities {
		if !e.active {
			e = Entity {
				active = true,
				hitbox = {16, 16},
			}
			return Entity_ID(id)
		}
	}
	append(&world.entities, Entity{active = true, hitbox = {16, 16}})
	return Entity_ID(len(world.entities) - 1)
}

destroy_entity :: proc(world: ^World, id: Entity_ID) {
	if int(id) >= 0 && int(id) < len(world.entities) {
		world.entities[id].components += {.Pending_Destroy}
	}
}

process_destroys :: proc(world: ^World) {
	for &e in world.entities {
		if .Pending_Destroy in e.components {
			e = Entity{} // resets active to false — slot is reclaimable
		}
	}
}

