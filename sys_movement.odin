package main

try_move :: proc(gs: ^Game_State, entity_id: Entity_ID, dx, dy: int) -> bool {
	if !(.Position in gs.world.entities[entity_id].components) {
		return false
	}

	new_x := gs.world.entities[entity_id].position.x + dx
	new_y := gs.world.entities[entity_id].position.y + dy

	if new_x < 0 || new_x >= MAP_WIDTH || new_y < 0 || new_y >= MAP_HEIGHT {
		return false
	}
	tile := gs.game_map.tiles[new_x][new_y]
	if tile.type == .Wall {
		return false
	}
	gs.world.entities[entity_id].position.x = new_x
	gs.world.entities[entity_id].position.y = new_y
	gs.world.entities[entity_id].transform.x = f32(new_x * TILE_SIZE)
	gs.world.entities[entity_id].transform.y = f32(new_y * TILE_SIZE)
	return true
}

