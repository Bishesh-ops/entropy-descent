package main
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

Entropy_State :: struct {
	entropy:          int,
	max_entropy:      int,
	tick_rate:        int,
	fov_radius:       int,
	bonus_aoe:        int,
	has_passive_aura: bool,
	health_locked:    bool,
}

Action_Type :: enum {
	None,
	Move,
	Melee_Attack,
	Spell,
	Wait,
}

Particle :: struct {
	x, y:     f32,
	dx, dy:   f32,
	life:     f32,
	max_life: f32,
	color:    Render_Color,
	size:     f32,
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

Component_Type :: enum {
	Position,
	Transform,
	Velocity,
	Hitbox,
	Render_Color,
	Player,
	Enemy,
	Pending_Destroy,
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
}

