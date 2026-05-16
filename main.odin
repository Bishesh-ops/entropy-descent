// main.odin
package main

import "core:fmt"
import sdl "vendor:sdl3"

main :: proc() {
	if !sdl.Init(sdl.INIT_VIDEO) {
		fmt.eprintln("SDL init failed:", sdl.GetError())
		return
	}
	defer sdl.Quit()

	// SDL3 window flags need an explicit value, e.g. sdl.WINDOW_SHOWN
	window := sdl.CreateWindow("Entropy Descent", 640, 480, sdl.WINDOW_SHOWN)
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

	// ----- Game State Init -----
	gs: Game_State
	gs.world.entities = make([dynamic]Entity) // allocate SOA array
	defer delete(gs.world.entities) // now works because type is known
	gs.game_map = init_map()
	gs.floor_depth = 1
	gs.tick_count = 0
	gs.entropy = Entropy_State {
		entropy          = 0,
		max_entropy      = 100,
		tick_rate        = 10,
		fov_radius       = 8,
		bonus_aoe        = 0,
		has_passive_aura = false,
		health_locked    = false,
	}
	gs.has_action = false

	// Spawn player
	player_id := spawn_entity(&gs.world)
	player := &gs.world.entities[player_id]
	player.position = {2, 2}
	player.transform = {f32(2 * TILE_SIZE), f32(2 * TILE_SIZE)}
	player.render_color = {0, 255, 0, 255}
	player.components += {.Position, .Transform, .Render_Color, .Player}
	gs.player_id = player_id

	// Spawn test enemy (red)
	enemy_id := spawn_entity(&gs.world)
	enemy := &gs.world.entities[enemy_id]
	enemy.position = {8, 8}
	enemy.transform = {f32(8 * TILE_SIZE), f32(8 * TILE_SIZE)}
	enemy.render_color = {255, 0, 0, 255}
	enemy.components += {.Position, .Transform, .Render_Color, .Enemy}
	enemy.speed = 100
	enemy.tick_threshold = int(225.0 / 100.0)
	enemy.next_action_tick = 0

	// ----- Main Loop -----
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

