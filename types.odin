package main

import "core:strings"

Entity_ID :: int

Component_Type :: enum {
	Player,
	Enemy,
	Collider,
	Item,
	Phantom,
	Pending_Destroy,
	Stairs,

	// Data Markers
	Position,
	Transform,
	Velocity,
	Hitbox,
	Health,
	Combat_Stats,
	Render_Color,
	Item_Effect,
	Inventory,
	Entropy_Stats,
	Projectile,
	Particle,
}

Component_Set :: bit_set[Component_Type]

Position :: struct {
	x, y: int,
}

Transform :: struct {
	x, y: f32,
}

Velocity :: struct {
	dx, dy: f32,
	speed:  f32,
}

Hitbox :: struct {
	width, height:      f32,
	offset_x, offset_y: f32,
}

Health :: struct {
	current, max: int,
}

Combat_Stats :: struct {
	attack, defense: int,
}

Render_Color :: struct {
	r, g, b, a: u8,
}

Item_effect :: struct {
	effect_type: string,
	magnitude:   int,
}

Inventory :: struct {
	items:        [dynamic]Entity_ID,
	max_capacity: int,
}

Entropy_State :: struct {
	entropy:          int,
	fov_radius:       int,
	bonus_aoe:        int,
	has_passive_aura: bool,
	health_locked:    bool,
}

Projectile :: struct {
	life_time: f32,
	damage:    int,
	element:   string,
	caster:    Entity_ID,
}

Particle :: struct {
	life_time: f32,
	max_life:  f32,
	vx, vy:    f32,
	r, g, b:   u8,
}

Enemy_Data :: struct {
	attack_cooldown: f32,
	aura_tick_timer: f32,
}

