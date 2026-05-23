// sys_particles.odin
package main

import "core:math"
import "core:math/rand"
import sdl "vendor:sdl3"

spawn_particle_burst :: proc(
	gs: ^Game_State,
	grid_x, grid_y: int,
	color: Render_Color,
	count: int = 15,
) {
	// Center the burst on the tile
	sx := f32(grid_x * TILE_SIZE) + f32(TILE_SIZE) / 2.0
	sy := f32(grid_y * TILE_SIZE) + f32(TILE_SIZE) / 2.0

	for _ in 0 ..< count {
		angle := rand.float32_range(0, 2 * math.PI)
		speed := rand.float32_range(40.0, 150.0)
		life := rand.float32_range(0.15, 0.4)
		size := rand.float32_range(2.0, 5.0)

		append(
			&gs.particle_sys.particles,
			Particle {
				x = sx,
				y = sy,
				dx = math.cos(angle) * speed,
				dy = math.sin(angle) * speed,
				life = life,
				max_life = life,
				color = color,
				size = size,
			},
		)
	}
}

sys_update_particles :: proc(gs: ^Game_State, dt: f32) {
	spawn_entropy_aura(gs)
	for i := 0; i < len(gs.particle_sys.particles); {
		gs.particle_sys.particles[i].life -= dt

		if gs.particle_sys.particles[i].life <= 0 {
			unordered_remove(&gs.particle_sys.particles, i)
		} else {
			gs.particle_sys.particles[i].dx *= 0.95
			gs.particle_sys.particles[i].dy *= 0.95

			gs.particle_sys.particles[i].x += gs.particle_sys.particles[i].dx * dt
			gs.particle_sys.particles[i].y += gs.particle_sys.particles[i].dy * dt
			i += 1
		}
	}
}

sys_render_particles :: proc(renderer: ^sdl.Renderer, gs: ^Game_State) {
	// Enable alpha blending for fading particles
	sdl.SetRenderDrawBlendMode(renderer, cast(sdl.BlendMode)sdl.BLENDMODE_BLEND)

	for i in 0 ..< len(gs.particle_sys.particles) {
		p := &gs.particle_sys.particles[i]

		// Calculate fade out
		alpha_pct := p.life / p.max_life
		alpha := u8(alpha_pct * 255.0)

		sdl.SetRenderDrawColor(renderer, p.color.r, p.color.g, p.color.b, alpha)

		rect := sdl.FRect {
			x = p.x - gs.camera.x - (p.size / 2.0),
			y = p.y - gs.camera.y - (p.size / 2.0),
			w = p.size,
			h = p.size,
		}
		sdl.RenderFillRect(renderer, &rect)
	}

	// Reset blend mode to default
	sdl.SetRenderDrawBlendMode(renderer, cast(sdl.BlendMode)sdl.BLENDMODE_NONE)

}

spawn_entropy_aura::proc(gs: ^Game_State){
	if gs.entropy.tier == .Calm do return

	spawn_chance: f32 = 0.0
	#partial switch gs.entropy.tier {
		case .Calm: spawn_chance = 0.0
		case .Unstable: spawn_chance = 0.1
		case .Fractured: spawn_chance = 0.40
		case .Critical: spawn_chance = 0.75
		case .Overflow: spawn_chance = 0.90
	}

	if rand.float32() < spawn_chance{
		px := gs.world.entities[gs.player_id].transform.x + f32(TILE_SIZE) / 2.0
		py := gs.world.entities[gs.player_id].transform.y + f32(TILE_SIZE) / 2.0

		offset_x := (rand.float32() - 0.5) * 20.0
		offset_y := (rand.float32() - 0.5) * 20.0

		append(
			&gs.particle_sys.particles,
			Particle {
				x = px + offset_x,
				y = py + offset_y,
				dx = (rand.float32() - 0.5) * 10.0,
				dy = -15.0 - (rand.float32() * 10.0),
				life = 0.5 + rand.float32() * 0.5,
				max_life = 1.0,
				color = {150, 50, 200, 255},
				size = 2.0 + rand.float32() * 2.0,
			},
		)
	}
}