// sys_movement.odin
package main

try_move :: proc(gs: ^Game_State, entity_id: Entity_ID, dx, dy: int) -> bool {
	entity := &gs.world.entities[entity_id]
	if !(.Position in entity.components) {
		return false
	}

	new_x := entity.position.x + dx
	new_y := entity.position.y + dy

	if new_x < 0 || new_x >= MAP_WIDTH || new_y < 0 || new_y >= MAP_HEIGHT {
		return false
	}
	tile := gs.game_map.tiles[new_x][new_y]
	if tile.type == .Wall {
		return false
	}

	entity.position.x = new_x
	entity.position.y = new_y
	entity.transform.x = f32(new_x * TILE_SIZE)
	entity.transform.y = f32(new_y * TILE_SIZE)
	return true
}

