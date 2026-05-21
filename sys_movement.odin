// sys_movement.odin
package main

try_move :: proc(gs: ^Game_State, entity_id: Entity_ID, dx, dy: int) -> bool {
	if !(.Position in gs.world.entities[entity_id].components) do return false

	new_x := gs.world.entities[entity_id].position.x + dx
	new_y := gs.world.entities[entity_id].position.y + dy

	if new_x < 0 || new_x >= MAP_WIDTH || new_y < 0 || new_y >= MAP_HEIGHT do return false

	tile := gs.game_map.tiles[new_x][new_y]
	if tile.type == .Wall do return false

	for i in 0 ..< len(gs.world.entities) {
		if !gs.world.entities[i].active || Entity_ID(i) == entity_id do continue
		if .Position not_in gs.world.entities[i].components do continue
		if gs.world.entities[i].position.x == new_x && gs.world.entities[i].position.y == new_y {
			return false
		}
	}

	gs.world.entities[entity_id].position.x = new_x
	gs.world.entities[entity_id].position.y = new_y
	gs.world.entities[entity_id].transform.x = f32(new_x * TILE_SIZE)
	gs.world.entities[entity_id].transform.y = f32(new_y * TILE_SIZE)
	return true
}

