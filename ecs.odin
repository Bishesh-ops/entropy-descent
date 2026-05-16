// ecs.odin
package main

World :: struct {
	entities: #soa[dynamic]Entity,
}

spawn_entity :: proc(world: ^World) -> Entity_ID {
	for &e, id in world.entities {
		if .Pending_Destroy in e.components {
			e = Entity{}
			return Entity_ID(id)
		}
	}

	e := Entity {
		position         = {0, 0},
		transform        = {0, 0},
		velocity         = {0, 0},
		hitbox           = {16, 16},
		render_color     = {255, 255, 255, 255},
		components       = {},
		speed            = 0,
		next_action_tick = 0,
		tick_threshold   = 1,
	}
	append(&world.entities, e)
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
			e.components = {}
		}
	}
}

