package main

import lua "vendor:lua/5.4"
import sdl "vendor:sdl3"

Position :: struct {
	x, y: int,
}
Transform :: struct {
	x, y: f32,
}
Velocity :: struct {
	x, y: f32,
}
Hitbox :: struct {
	w, h: f32,
}
Render_Color :: struct {
	r, g, b, a: u8,
}

Entropy_Tier :: enum {
	Calm, // 0 - 25%
	Unstable, // 26 - 50%
	Fractured, // 51 - 75%
	Critical, // 76 - 99%
	Overflow, // 100%
}

Entropy_State :: struct {
	entropy, max_entropy, tick_rate, fov_radius, bonus_aoe: int,
	has_passive_aura, health_locked:                        bool,
	tier:                                                   Entropy_Tier,
}

Action_Type :: enum {
	None,
	Move,
	Melee_Attack,
	Spell,
	Wait,
}

Particle :: struct {
	x, y, dx, dy, life, max_life, size: f32,
	color:                              Render_Color,
}

Particle_System :: struct {
	particles: [dynamic]Particle,
}

Action :: struct {
	type:      Action_Type,
	direction: [2]int,
	cost:      int,
}

Camera :: struct {
	x, y: f32,
}

Flash :: struct {
	timer:   f32,
	r, g, b: u8,
}

Component_Type :: enum {
	Position,
	Transform,
	Velocity,
	Hitbox,
	Render_Color,
	Player,
	Enemy,
	Pending_Destroy,
	Health,
	Combat,
}

Component_Set :: bit_set[Component_Type]

Entity :: struct {
	active:           bool,
	position:         Position,
	transform:        Transform,
	velocity:         Velocity,
	hitbox:           Hitbox,
	render_color:     Render_Color,
	components:       Component_Set,
	speed:            int,
	next_action_tick: int,
	tick_threshold:   int,
	health:           int,
	max_health:       int,
	attack:           int,
	facing:           [2]int,
}

Entity_ID :: distinct int

Game_State :: struct {
	world:          World,
	game_map:       Map,
	floor_depth:    int,
	tick_count:     int,
	entropy:        Entropy_State,
	player_id:      Entity_ID,
	player_action:  Action,
	has_action:     bool,
	particle_sys:   Particle_System,
	camera:         Camera,
	player_texture: ^sdl.Texture,
	game_over:      bool,
	flash:          Flash,
	screen_shake:   f32,
	lua_state:      ^lua.State,
}

