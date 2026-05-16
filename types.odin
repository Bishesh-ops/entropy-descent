// types.odin
package main

import "core:math"

// ----- Basic Components -----
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

// ----- Entropy State -----
Entropy_State :: struct {
	entropy:          int,
	max_entropy:      int,
	tick_rate:        int, // passive entropy +1 every N ticks
	fov_radius:       int,
	bonus_aoe:        int,
	has_passive_aura: bool,
	health_locked:    bool,
}

// ----- Action & Tick System -----
Action_Type :: enum {
	None,
	Move,
	Melee_Attack,
	Spell,
	Wait,
}

Action :: struct {
	type:      Action_Type,
	direction: [2]int, // tile offset (dx, dy)
	cost:      int, // tick cost
}

// ----- Component Tags (bit_set enum) -----
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

// ----- Entity (Fat Struct) -----
// Note: #soa is applied to the slice in World, not here.
Entity :: struct {
	position:         Position,
	transform:        Transform,
	velocity:         Velocity,
	hitbox:           Hitbox,
	render_color:     Render_Color,
	components:       Component_Set,

	// Tick scheduling (used by enemies, but lives here for SOA)
	speed:            int,
	next_action_tick: int,
	tick_threshold:   int,
}

Entity_ID :: distinct int

