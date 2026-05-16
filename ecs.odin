package main

Entity :: struct {
	active:     bool,
	components: Component_Set,
	pos:        Position,
	transform:  Transform,
	vel:        Velocity,
	hitbox:     Hitbox,
	health:     Health,
	combat:     Combat_Stats,
	color:      Render_Color,
	effect:     Item_effect,
	inventory:  Inventory,
	entropy:    Entropy_State,
	projectile: Projectile,
	particle:   Particle,
	enemy_data: Enemy_Data,
}

World :: struct {
	entities: #soa[dynamic]Entity,
}

init_world :: proc() -> World {
	return World{entities = make_soa(#soa[dynamic]Entity)}
}

destroy_world :: proc(world: ^World) {
	for e in world.entities {
		if .Inventory in e.components {
			delete(e.inventory.items)
		}
	}
	delete_soa(world.entities)
}

spawn_entity :: proc(world: ^World) -> Entity_ID {
	for e, i in world.entities {
		if !e.active {
			world.entities[i].active = true
			world.entities[i].components = {}
			return i
		}
	}
	append_soa(&world.entities, Entity{active = true})
	return len(world.entities) - 1
}

queue_destroy :: proc(world: ^World, id: Entity_ID) {
	if id >= 0 && id < len(world.entities) {
		world.entities[id].components += {.Pending_Destroy}
	}
}

process_destroys :: proc(world: ^World) {
	for e, i in world.entities {
		if !e.active do continue

		if .Pending_Destroy in e.components {
			if .Inventory in e.components {
				delete(world.entities[i].inventory.items)
				world.entities[i].inventory.items = nil
			}
			world.entities[i].active = false
			world.entities[i].components = {}
		}
	}
}

