package main

import "core:fmt"
import sdl "vendor:sdl3"

WINDOW_WIDTH :: 800
WINDOW_HEIGHT :: 600

main :: proc() {
	if !sdl.Init({.VIDEO, .AUDIO}) {
		fmt.eprintfln("SDL_Init Error: %s", sdl.GetError())
		return
	}
	defer sdl.Quit()

	window := sdl.CreateWindow("Entropy Descent", WINDOW_WIDTH, WINDOW_HEIGHT, {})
	if window == nil {
		fmt.eprintfln("SDL_CreateWindow Error: %s", sdl.GetError())
		return
	}
	defer sdl.DestroyWindow(window)

	renderer := sdl.CreateRenderer(window, nil)
	if renderer == nil {
		fmt.eprintfln("SDL_CreateRenderer Error: %s", sdl.GetError())
		return
	}

	defer sdl.DestroyRenderer(renderer)
	sdl.SetRenderDrawBlendMode(renderer, sdl.BLENDMODE_BLEND)

	world := init_world()
	defer destroy_world(&world)
	game_map := init_map()

	player_id := spawn_entity(&world)
	world.entities[player_id].components += {
		.Player,
		.Position,
		.Velocity,
		.Transform,
		.Render_Color,
		.Hitbox,
	}
	world.entities[player_id].transform = {
		x = 100.0,
		y = 100.0,
	}
	world.entities[player_id].vel = {
		dx    = 0,
		dy    = 0,
		speed = 300.0,
	}
	world.entities[player_id].pos = {
		x = 100,
		y = 100,
	}
	world.entities[player_id].color = {
		r = 0,
		g = 255,
		b = 100,
		a = 255,
	}
	world.entities[player_id].hitbox = {
		width    = 32.0,
		height   = 32.0,
		offset_x = 0,
		offset_y = 0,
	}

	is_running := true
	last_time := sdl.GetTicks()

	for is_running {
		current_time := sdl.GetTicks()
		dt := f32(current_time - last_time) / 1000.0
		last_time = current_time
		if dt > 0.05 do dt = 0.05

		event: sdl.Event
		for sdl.PollEvent(&event) {
			#partial switch event.type {
			case .QUIT:
				is_running = false
			case .KEY_DOWN:
				if event.key.key == sdl.K_ESCAPE {
					is_running = false
				}
			}
		}


		// --- SYSTEMS PIPELINE ---

		sys_input(&world)
		sys_movement(&world, &game_map, dt)

		// 1. WIPE THE SLATE CLEAN (This fixes the trailing!)
		sdl.SetRenderDrawColor(renderer, 20, 20, 25, 255)
		sdl.RenderClear(renderer)

		// 2. Render Map layer
		sys_render_map(&game_map, renderer)

		// 3. Render Entity layer
		sys_render(&world, renderer)

		process_destroys(&world)}
}

