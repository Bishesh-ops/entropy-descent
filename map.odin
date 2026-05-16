// map.odin
package main

TILE_SIZE :: 16

Tile_Type :: enum {
	Floor,
	Wall,
}

Tile_State :: enum {
	Neutral,
	Scorched,
	Frozen,
	Charged,
}

Tile :: struct {
	type:       Tile_Type,
	state:      Tile_State,
	state_life: int,
}

MAP_WIDTH :: 20
MAP_HEIGHT :: 15

Map :: struct {
	tiles: [MAP_WIDTH][MAP_HEIGHT]Tile,
}

init_map :: proc() -> Map {
	m: Map
	for x in 0 ..< MAP_WIDTH {
		for y in 0 ..< MAP_HEIGHT {
			if x == 0 || x == MAP_WIDTH - 1 || y == 0 || y == MAP_HEIGHT - 1 {
				m.tiles[x][y] = Tile {
					type       = .Wall,
					state      = .Neutral,
					state_life = 0,
				}
			} else {
				m.tiles[x][y] = Tile {
					type       = .Floor,
					state      = .Neutral,
					state_life = 0,
				}
			}
		}
	}
	return m
}

