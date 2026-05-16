package main

import sdl "vendor:sdl3"

TILE_SIZE :: 32
MAP_WIDTH :: 25
MAP_HEIGHT :: 18

Tile_Type :: enum {
	Floor,
	Wall,
}

Map :: struct {
	tiles: [MAP_WIDTH][MAP_HEIGHT]Tile_Type,
}

init_map :: proc() -> Map {
	m: Map

	for x in 0 ..< MAP_WIDTH {
		for y in 0 ..< MAP_HEIGHT {
			if x == 0 || x == MAP_WIDTH - 1 || y == 0 || y == MAP_HEIGHT - 1 {
				m.tiles[x][y] = .Wall
			} else {
				m.tiles[x][y] = .Floor
			}
		}
	}
	m.tiles[10][8] = .Wall
	m.tiles[10][9] = .Wall
	m.tiles[15][8] = .Wall

	return m
}

sys_render_map :: proc(m: ^Map, renderer: ^sdl.Renderer) {
	for x in 0 ..< MAP_WIDTH {
		for y in 0 ..< MAP_HEIGHT {
			rect := sdl.FRect {
				x = f32(x * TILE_SIZE),
				y = f32(y * TILE_SIZE),
				w = f32(TILE_SIZE),
				h = f32(TILE_SIZE),
			}

			if m.tiles[x][y] == .Wall {
				sdl.SetRenderDrawColor(renderer, 120, 120, 130, 255)
			} else {
				sdl.SetRenderDrawColor(renderer, 30, 30, 35, 255)
			}

			sdl.RenderFillRect(renderer, &rect)

			sdl.SetRenderDrawColor(renderer, 0, 0, 0, 50)
			sdl.RenderFillRect(renderer, &rect)
		}
	}
}

