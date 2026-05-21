package main

import "core:fmt"
import "core:math"
import sdl "vendor:sdl3"
import sdl_image "vendor:sdl3/image"

GAME_W :: 1280
GAME_H :: 720

CAMERA_VIEW_W :: 320
CAMERA_VIEW_H :: 240

main :: proc() {
	if !sdl.Init({.VIDEO}) {
		fmt.eprintln("SDL init failed:", sdl.GetError())
		return
	}
	defer sdl.Quit()

	window := sdl.CreateWindow("Entropy Descent", 0, 0, {.FULLSCREEN})
	if window == nil {
		fmt.eprintln("Window creation failed:", sdl.GetError())
		return
	}
	defer sdl.DestroyWindow(window)

	renderer := sdl.CreateRenderer(window, nil)
	if renderer == nil {
		fmt.eprintln("Renderer creation failed:", sdl.GetError())
		return
	}
	defer sdl.DestroyRenderer(renderer)

	sdl.SetRenderLogicalPresentation(renderer, CAMERA_VIEW_W, CAMERA_VIEW_H, .LETTERBOX)
	gs: Game_State

	gs.player_texture = sdl_image.LoadTexture(renderer, "assets/player/Player.png")
	if gs.player_texture == nil {
		fmt.eprintln("Failed to load player texture:", sdl.GetError())
	}

	gs.world.entities = make(#soa[dynamic]Entity)
	defer delete(gs.world.entities)
	gs.particle_sys.particles = make([dynamic]Particle)
	defer delete(gs.particle_sys.particles)

	gs.floor_depth = 1
	gs.game_map = init_map(u64(gs.floor_depth))
	gs.tick_count = 0
	gs.has_action = false
	gs.entropy = Entropy_State {
		entropy          = 0,
		max_entropy      = 100,
		tick_rate        = 10,
		fov_radius       = 8,
		bonus_aoe        = 0,
		has_passive_aura = false,
		health_locked    = false,
	}

	// Find valid floor tiles before spawning anything
	player_spawn := find_spawn_point(&gs.game_map, 1, 1)
	enemy_spawn := find_spawn_point(&gs.game_map, MAP_WIDTH / 2, MAP_HEIGHT / 2)

	player_id := spawn_entity(&gs.world)
	gs.world.entities[player_id].position = player_spawn
	gs.world.entities[player_id].transform = {
		f32(player_spawn.x * TILE_SIZE),
		f32(player_spawn.y * TILE_SIZE),
	}
	gs.world.entities[player_id].render_color = {0, 255, 0, 255}
	gs.world.entities[player_id].hitbox = {14, 14}
	gs.world.entities[player_id].components += {.Position, .Transform, .Render_Color, .Player}
	gs.player_id = player_id

	enemy_id := spawn_entity(&gs.world)
	gs.world.entities[enemy_id].position = enemy_spawn
	gs.world.entities[enemy_id].transform = {
		f32(enemy_spawn.x * TILE_SIZE),
		f32(enemy_spawn.y * TILE_SIZE),
	}
	gs.world.entities[enemy_id].render_color = {255, 0, 0, 255}
	gs.world.entities[enemy_id].hitbox = {14, 14}
	gs.world.entities[enemy_id].components += {.Position, .Transform, .Render_Color, .Enemy}
	gs.world.entities[enemy_id].speed = 100
	gs.world.entities[enemy_id].tick_threshold = int(225 / 100)
	gs.world.entities[enemy_id].next_action_tick = 0

	// Init camera centered on player
	gs.camera = Camera {
		x = f32(player_spawn.x * TILE_SIZE) - f32(CAMERA_VIEW_W) / 2,
		y = f32(player_spawn.y * TILE_SIZE) - f32(CAMERA_VIEW_H) / 2,
	}

	running := true
	for running {
		event_loop(&gs, &running)

		if gs.has_action {
			sys_tick(&gs)
			gs.has_action = false
		}

		// Camera follows player every frame with lerp (smooth)
		px := gs.world.entities[gs.player_id].transform.x
		py := gs.world.entities[gs.player_id].transform.y

		target_x := px - f32(CAMERA_VIEW_W) / 2
		target_y := py - f32(CAMERA_VIEW_H) / 2

		// Clamp to map bounds
		max_cam_x := f32(MAP_WIDTH * TILE_SIZE - CAMERA_VIEW_W)
		max_cam_y := f32(MAP_HEIGHT * TILE_SIZE - CAMERA_VIEW_H)
		target_x = clamp(target_x, 0, max_cam_x)
		target_y = clamp(target_y, 0, max_cam_y)

		// Lerp: smooth follow, not instant snap
		gs.camera.x += (target_x - gs.camera.x) * 0.15
		gs.camera.y += (target_y - gs.camera.y) * 0.15

		sys_update_particles(&gs, 0.016)

		sdl.SetRenderDrawColor(renderer, 0, 0, 0, 255)
		sdl.RenderClear(renderer)

		sys_render_map(renderer, &gs.game_map, gs.camera)
		sys_render_entities(renderer, &gs.world, gs.camera, &gs)
		sys_render_particles(renderer, &gs)

		sdl.RenderPresent(renderer)
		sdl.Delay(16)
	}
}

// Scans outward from a starting point to find the nearest floor tile
find_spawn_point :: proc(m: ^Map, start_x, start_y: int) -> Position {
	for radius in 0 ..< MAP_WIDTH {
		for dx in -radius ..= radius {
			for dy in -radius ..= radius {
				if abs(dx) != radius && abs(dy) != radius do continue
				x := clamp(start_x + dx, 1, MAP_WIDTH - 2)
				y := clamp(start_y + dy, 1, MAP_HEIGHT - 2)
				if m.tiles[x][y].type == .Floor {
					return Position{x, y}
				}
			}
		}
	}
	return Position{1, 1} // fallback, shouldn't happen
}

