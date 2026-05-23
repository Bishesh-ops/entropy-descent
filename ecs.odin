package main

World :: struct {
	entities: #soa[dynamic]Entity,
}

spawn_entity :: proc(world: ^World) -> Entity_ID {
	for i in 0 ..< len(world.entities) {
		if !world.entities[i].active {
			world.entities[i] = Entity {
				active = true,
			}
			return Entity_ID(i)
		}
	}

	append(&world.entities, Entity{active = true})
	return Entity_ID(len(world.entities) - 1)
}

process_destroys :: proc(world: ^World) {
	for i in 0 ..< len(world.entities) {
		if .Pending_Destroy in world.entities[i].components {
			world.entities[i] = Entity{}
		}
	}
}

update_entropy_tier :: proc(gs: ^Game_State) {
	pct := f32(gs.entropy.entropy) / f32(gs.entropy.max_entropy)
	if pct >= 1.00 {
		gs.entropy.tier = .Overflow
	} else if pct >= 0.76 {
		gs.entropy.tier = .Critical
	} else if pct >= 0.51 {
		gs.entropy.tier = .Fractured
	} else if pct >= 0.26 {
		gs.entropy.tier = .Unstable
	} else {
		gs.entropy.tier = .Calm
	}
}

