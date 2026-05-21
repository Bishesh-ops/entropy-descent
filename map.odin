package main

import "core:math/rand"

TILE_SIZE :: 16
MAP_WIDTH :: 80
MAP_HEIGHT :: 60
WALL_FILL_PERCENT :: 52 // sweet spot: cave-dominant but still navigable

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

Map :: struct {
	tiles: [MAP_WIDTH][MAP_HEIGHT]Tile,
}

init_map :: proc(seed: u64 = 0) -> Map {
	s := seed
	if s == 0 do s = u64(rand.uint64())
	rand.reset(s)

	m: Map
	for x in 0 ..< MAP_WIDTH {
		for y in 0 ..< MAP_HEIGHT {
			if x == 0 || x == MAP_WIDTH - 1 || y == 0 || y == MAP_HEIGHT - 1 {
				m.tiles[x][y].type = .Wall
			} else {
				m.tiles[x][y].type = .Wall if rand.int_max(100) < WALL_FILL_PERCENT else .Floor
			}
		}
	}

	// First 4 passes: standard smoothing (forms the cave shapes)
	for _ in 0 ..< 4 {
		m = smooth_pass(&m, 4)
	}
	// Last 3 passes: stricter threshold (fills in thin noise, widens passages)
	for _ in 0 ..< 3 {
		m = smooth_pass(&m, 3)
	}

	remove_small_regions(&m)
	return m
}

smooth_pass :: proc(old: ^Map, threshold: int) -> Map {
	new := old^
	for x in 1 ..< MAP_WIDTH - 1 {
		for y in 1 ..< MAP_HEIGHT - 1 {
			walls := count_wall_neighbors(old, x, y)
			new.tiles[x][y].type = .Wall if walls > threshold else .Floor
		}
	}
	return new
}

count_wall_neighbors :: proc(m: ^Map, x, y: int) -> int {
	count := 0
	for i in -1 ..= 1 {
		for j in -1 ..= 1 {
			if i == 0 && j == 0 do continue
			nx, ny := x + i, y + j
			if nx < 0 || nx >= MAP_WIDTH || ny < 0 || ny >= MAP_HEIGHT {
				count += 1
			} else if m.tiles[nx][ny].type == .Wall {
				count += 1
			}
		}
	}
	return count
}

flood_fill :: proc(m: ^Map, start_x, start_y: int, tile_type: Tile_Type) -> [dynamic][2]int {
	visited := [MAP_WIDTH][MAP_HEIGHT]bool{}
	result := make([dynamic][2]int)
	stack := make([dynamic][2]int)
	defer delete(stack)

	append(&stack, [2]int{start_x, start_y})
	visited[start_x][start_y] = true

	for len(stack) > 0 {
		curr := pop(&stack)
		append(&result, curr)

		offsets := [4][2]int{{0, 1}, {0, -1}, {1, 0}, {-1, 0}}
		for off in offsets {
			nx := curr[0] + off[0] // [0] not .x
			ny := curr[1] + off[1] // [1] not .y
			if nx < 0 || nx >= MAP_WIDTH || ny < 0 || ny >= MAP_HEIGHT do continue
			if visited[nx][ny] do continue
			if m.tiles[nx][ny].type != tile_type do continue
			visited[nx][ny] = true
			append(&stack, [2]int{nx, ny})
		}
	}
	return result
}

remove_small_regions :: proc(m: ^Map) {
	// Pass 1: keep only the largest floor region
	{
		visited := [MAP_WIDTH][MAP_HEIGHT]bool{}
		largest: [dynamic][2]int

		for x in 1 ..< MAP_WIDTH - 1 {
			for y in 1 ..< MAP_HEIGHT - 1 {
				if visited[x][y] || m.tiles[x][y].type != .Floor do continue
				region := flood_fill(m, x, y, .Floor)
				for t in region {visited[t[0]][t[1]] = true}
				if len(region) > len(largest) {
					delete(largest)
					largest = region
				} else {
					delete(region)
				}
			}
		}

		keep := [MAP_WIDTH][MAP_HEIGHT]bool{}
		for t in largest {keep[t[0]][t[1]] = true}
		delete(largest)

		for x in 1 ..< MAP_WIDTH - 1 {
			for y in 1 ..< MAP_HEIGHT - 1 {
				if m.tiles[x][y].type == .Floor && !keep[x][y] {
					m.tiles[x][y].type = .Wall
				}
			}
		}
	}

	// Pass 2: remove floating wall blobs (not connected to border)
	{
		visited := [MAP_WIDTH][MAP_HEIGHT]bool{}

		// Pre-mark border as visited so they're never culled
		for x in 0 ..< MAP_WIDTH {
			visited[x][0] = true
			visited[x][MAP_HEIGHT - 1] = true
		}
		for y in 0 ..< MAP_HEIGHT {
			visited[0][y] = true
			visited[MAP_WIDTH - 1][y] = true
		}

		for x in 1 ..< MAP_WIDTH - 1 {
			for y in 1 ..< MAP_HEIGHT - 1 {
				if visited[x][y] || m.tiles[x][y].type != .Wall do continue
				region := flood_fill(m, x, y, .Wall)
				for t in region {visited[t[0]][t[1]] = true}
				if len(region) < 6 {
					for t in region {
						m.tiles[t[0]][t[1]].type = .Floor
					}
				}
				delete(region)
			}
		}
	}
}

