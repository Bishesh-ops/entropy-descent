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
	sdl.SetRenderDrawBlendMode(renderer, sdl.BLENDMODE_BLEND)

}

