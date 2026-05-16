package main

import "core:fmt"
import sdl "vendor:sdl3"

main :: proc() {
	if !sdl.Init({.VIDEO}) {
		fmt.eprintln("SDL init failed:", sdl.GetError())
		return
	}
	defer sdl.Quit()

	window := sdl.CreateWindow("Entropy Descent", 640, 480, {})
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

	gs: Game_State
	gs.world.entities = make([dynamic]Entity)
	defer delete(gs.world.entities)

	gs.game_map = init_map()
	gs.floor_depth = 1
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

	player_id := spawn_entity(&gs.world)
	enemy_id := spawn_entity(&gs.world)

	gs.world.entities[player_id].position = {2, 2}
	gs.world.entities[player_id].transform = {f32(2 * TILE_SIZE), f32(2 * TILE_SIZE)}
	gs.world.entities[player_id].render_color = {0, 255, 0, 255}
	gs.world.entities[player_id].components += {.Position, .Transform, .Render_Color, .Player}
	gs.player_id = player_id

	gs.world.entities[enemy_id].position = {8, 8}
	gs.world.entities[enemy_id].transform = {f32(8 * TILE_SIZE), f32(8 * TILE_SIZE)}
	gs.world.entities[enemy_id].render_color = {255, 0, 0, 255}
	gs.world.entities[enemy_id].components += {.Position, .Transform, .Render_Color, .Enemy}
	gs.world.entities[enemy_id].speed = 100
	gs.world.entities[enemy_id].tick_threshold = int(225 / 100)
	gs.world.entities[enemy_id].next_action_tick = 0

	running := true
	for running {
		event_loop(&gs, &running)

		if gs.has_action {
			sys_tick(&gs)
			gs.has_action = false
		}

		sys_render_map(renderer, &gs.game_map)
		sys_render_entities(renderer, &gs.world)
		sdl.RenderPresent(renderer)

		sdl.Delay(16)
	}
}

