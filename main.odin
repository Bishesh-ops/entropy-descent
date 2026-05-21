package main

import "core:fmt"
import "core:math/rand"
import sdl "vendor:sdl3"
import sdl_image "vendor:sdl3/image"

GAME_W :: 1280
GAME_H :: 720
CAMERA_VIEW_W :: 320
CAMERA_VIEW_H :: 240

Enemy_Def :: struct {
	hp, attack, speed, tick: int,
	color:                   Render_Color,
	hint_x, hint_y:          int,
}

ENEMY_ROSTER := [8]Enemy_Def {
	{
		hp = 15,
		attack = 3,
		speed = 100,
		tick = 2,
		color = {80, 160, 50, 255},
		hint_x = MAP_WIDTH / 4,
		hint_y = MAP_HEIGHT / 4,
	},
	{
		hp = 12,
		attack = 5,
		speed = 80,
		tick = 3,
		color = {200, 200, 175, 255},
		hint_x = 3 * MAP_WIDTH / 4,
		hint_y = MAP_HEIGHT / 4,
	},
	{
		hp = 35,
		attack = 9,
		speed = 60,
		tick = 4,
		color = {50, 110, 40, 255},
		hint_x = MAP_WIDTH / 4,
		hint_y = 3 * MAP_HEIGHT / 4,
	},
	{
		hp = 10,
		attack = 7,
		speed = 150,
		tick = 1,
		color = {100, 180, 220, 255},
		hint_x = 3 * MAP_WIDTH / 4,
		hint_y = 3 * MAP_HEIGHT / 4,
	},
	{
		hp = 15,
		attack = 3,
		speed = 100,
		tick = 2,
		color = {80, 160, 50, 255},
		hint_x = MAP_WIDTH / 2,
		hint_y = MAP_HEIGHT / 4,
	},
	{
		hp = 12,
		attack = 5,
		speed = 80,
		tick = 3,
		color = {200, 200, 175, 255},
		hint_x = MAP_WIDTH / 3,
		hint_y = MAP_HEIGHT / 2,
	},
	{
		hp = 20,
		attack = 6,
		speed = 90,
		tick = 2,
		color = {160, 80, 40, 255},
		hint_x = 2 * MAP_WIDTH / 3,
		hint_y = MAP_HEIGHT / 2,
	},
	{
		hp = 10,
		attack = 7,
		speed = 150,
		tick = 1,
		color = {100, 180, 220, 255},
		hint_x = MAP_WIDTH / 2,
		hint_y = 2 * MAP_HEIGHT / 3,
	},
}

main :: proc() {
	if !sdl.Init({.VIDEO}) {
		fmt.eprintln("SDL init failed:", sdl.GetError())
		return
	}
	defer sdl.Quit()

	window := sdl.CreateWindow("Entropy Descent", 0, 0, {.FULLSCREEN})
	if window == nil {fmt.eprintln("Window failed:", sdl.GetError()); return}
	defer sdl.DestroyWindow(window)

	renderer := sdl.CreateRenderer(window, nil)
	if renderer == nil {fmt.eprintln("Renderer failed:", sdl.GetError()); return}
	defer sdl.DestroyRenderer(renderer)

	sdl.SetRenderLogicalPresentation(renderer, CAMERA_VIEW_W, CAMERA_VIEW_H, .LETTERBOX)

	gs: Game_State
	gs.world.entities = make(#soa[dynamic]Entity)
	gs.particle_sys.particles = make([dynamic]Particle)
	defer delete(gs.world.entities)
	defer delete(gs.particle_sys.particles)

	gs.player_texture = sdl_image.LoadTexture(renderer, "assets/player/Player.png")
	if gs.player_texture == nil {
		fmt.eprintln("Texture load failed:", sdl.GetError())
	}
	defer sdl.DestroyTexture(gs.player_texture)

	init_game_state(&gs)
	sys_update_fov(&gs)

	last_tick := sdl.GetTicks()

	running := true
	for running {
		current_tick := sdl.GetTicks()
		dt := f32(current_tick - last_tick) / 1000.0
		last_tick = current_tick

		if dt > 0.05 do dt = 0.05

		event_loop(&gs, &running)

		if gs.has_action && !gs.game_over {
			sys_tick(&gs)
			process_destroys(&gs.world)
			sys_update_fov(&gs)
			gs.has_action = false
		}

		if gs.flash.timer > 0 {
			gs.flash.timer -= dt
			if gs.flash.timer < 0 do gs.flash.timer = 0
		}

		px := gs.world.entities[gs.player_id].transform.x
		py := gs.world.entities[gs.player_id].transform.y

		tx := clamp(px - f32(CAMERA_VIEW_W) / 2, 0, f32(MAP_WIDTH * TILE_SIZE - CAMERA_VIEW_W))
		ty := clamp(py - f32(CAMERA_VIEW_H) / 2, 0, f32(MAP_HEIGHT * TILE_SIZE - CAMERA_VIEW_H))

		gs.camera.x += (tx - gs.camera.x) * 9.0 * dt
		gs.camera.y += (ty - gs.camera.y) * 9.0 * dt

		sys_update_particles(&gs, dt)

		sdl.SetRenderDrawColor(renderer, 0, 0, 0, 255)
		sdl.RenderClear(renderer)


		shake_offset_x: f32 = 0
		shake_offset_y: f32 = 0
		if gs.screen_shake > 0 {
			gs.screen_shake -= 0.016
			if gs.screen_shake < 0 do gs.screen_shake = 0

			intensity := gs.screen_shake * 30.0
			shake_offset_x = (rand.float32() - 0.5) * intensity
			shake_offset_y = (rand.float32() - 0.5) * intensity
		}

		render_cam := gs.camera
		render_cam.x += shake_offset_x
		render_cam.y += shake_offset_y

		sdl.SetRenderDrawColor(renderer, 0, 0, 0, 255)
		sdl.RenderClear(renderer)

		sys_render_map(renderer, &gs.game_map, render_cam)
		sys_render_entities(renderer, &gs.world, render_cam, &gs)

		sys_render_particles(renderer, &gs)
		sys_render_hud(renderer, &gs)

		sdl.RenderPresent(renderer)
		sdl.Delay(16)
	}
}
init_game_state :: proc(gs: ^Game_State) {
	gs.floor_depth = 1
	gs.game_map = init_map(u64(gs.floor_depth))
	gs.tick_count = 0
	gs.has_action = false
	gs.game_over = false
	gs.flash = {}
	gs.entropy = Entropy_State {
		entropy          = 0,
		max_entropy      = 100,
		tick_rate        = 10,
		fov_radius       = 8,
		bonus_aoe        = 0,
		has_passive_aura = false,
		health_locked    = false,
	}
	init_lua(gs)
	spawn_all_entities(gs)
}

spawn_all_entities :: proc(gs: ^Game_State) {
	player_spawn := find_spawn_point(&gs.game_map, 5, 5)
	pid := spawn_entity(&gs.world)
	gs.player_id = pid

	p := &gs.world.entities[pid]
	p.position = player_spawn
	p.transform = {f32(player_spawn.x * TILE_SIZE), f32(player_spawn.y * TILE_SIZE)}
	p.render_color = {0, 255, 0, 255}
	p.hitbox = {14, 14}
	p.health = 80
	p.max_health = 80
	p.attack = 12
	p.facing = {0, 1}
	p.components += {.Position, .Transform, .Render_Color, .Player, .Health, .Combat}

	gs.camera = Camera {
		x = f32(player_spawn.x * TILE_SIZE) - f32(CAMERA_VIEW_W) / 2,
		y = f32(player_spawn.y * TILE_SIZE) - f32(CAMERA_VIEW_H) / 2,
	}

	for def in ENEMY_ROSTER {
		spawn := find_spawn_point(&gs.game_map, def.hint_x, def.hint_y)
		eid := spawn_entity(&gs.world)
		e := &gs.world.entities[eid]
		e.position = spawn
		e.transform = {f32(spawn.x * TILE_SIZE), f32(spawn.y * TILE_SIZE)}
		e.render_color = def.color
		e.hitbox = {14, 14}
		e.health = def.hp
		e.max_health = def.hp
		e.attack = def.attack
		e.speed = def.speed
		e.tick_threshold = def.tick
		e.next_action_tick = 0
		e.components += {.Position, .Transform, .Render_Color, .Enemy, .Health, .Combat}
	}
}

restart_game :: proc(gs: ^Game_State) {
	texture := gs.player_texture

	delete(gs.world.entities)
	delete(gs.particle_sys.particles)
	gs.world.entities = make(#soa[dynamic]Entity)
	gs.particle_sys.particles = make([dynamic]Particle)
	gs.player_texture = texture

	init_game_state(gs)
	sys_update_fov(gs)
}

find_spawn_point :: proc(m: ^Map, start_x, start_y: int) -> Position {
	for radius in 0 ..< MAP_WIDTH {
		for dx in -radius ..= radius {
			for dy in -radius ..= radius {
				if abs(dx) != radius && abs(dy) != radius do continue
				x := clamp(start_x + dx, 1, MAP_WIDTH - 2)
				y := clamp(start_y + dy, 1, MAP_HEIGHT - 2)
				if m.tiles[x][y].type == .Floor do return Position{x, y}
			}
		}
	}
	return Position{1, 1}
}

