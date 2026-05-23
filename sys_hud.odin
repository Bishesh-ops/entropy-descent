package main

import "core:math"
import sdl "vendor:sdl3"

sys_render_hud :: proc(renderer: ^sdl.Renderer, gs: ^Game_State) {
	W :: f32(CAMERA_VIEW_W)
	H :: f32(CAMERA_VIEW_H)

	sdl.SetRenderDrawBlendMode(renderer, cast(sdl.BlendMode)sdl.BLENDMODE_BLEND)

	panel := sdl.FRect{0, H - 20, W, 20}
	sdl.SetRenderDrawColor(renderer, 8, 8, 12, 230)
	sdl.RenderFillRect(renderer, &panel)

	sep := sdl.FRect{0, H - 20, W, 1}
	sdl.SetRenderDrawColor(renderer, 70, 70, 110, 255)
	sdl.RenderFillRect(renderer, &sep)

	player := gs.world.entities[gs.player_id]
	bar_y := H - 12
	bar_h: f32 = 7

	{
		bx: f32 = 4
		bw: f32 = 90
		sdl.SetRenderDrawColor(renderer, 50, 10, 10, 255)
		bg := sdl.FRect{bx, bar_y, bw, bar_h}
		sdl.RenderFillRect(renderer, &bg)

		pct := f32(player.health) / f32(player.max_health)
		if pct < 0 do pct = 0
		if pct > 1 do pct = 1

		gr := u8(pct * 90)
		sdl.SetRenderDrawColor(renderer, 210, gr, 30, 255)
		fill := sdl.FRect{bx, bar_y, bw * pct, bar_h}
		sdl.RenderFillRect(renderer, &fill)

		sdl.SetRenderDrawColor(renderer, 160, 70, 70, 255)
		sdl.RenderRect(renderer, &bg)

		for i in 1 ..< 4 {
			pip := sdl.FRect{bx + bw * f32(i) / 4.0, bar_y, 1, bar_h}
			sdl.SetRenderDrawColor(renderer, 20, 5, 5, 180)
			sdl.RenderFillRect(renderer, &pip)
		}
	}

	{
		bx: f32 = 100
		bw: f32 = 90
		sdl.SetRenderDrawColor(renderer, 25, 0, 45, 255)
		bg := sdl.FRect{bx, bar_y, bw, bar_h}
		sdl.RenderFillRect(renderer, &bg)

		pct := f32(gs.entropy.entropy) / f32(gs.entropy.max_entropy)
		if pct < 0 do pct = 0
		if pct > 1 do pct = 1

		time_sec := f32(sdl.GetTicks()) / 1000.0
		pulse: f32 = 1.0
		#partial switch gs.entropy.tier {
		case .Calm: 
			pulse = 1.0
		case .Unstable: 
			pulse = 0.9 + 0.1 * math.sin(time_sec * 3.0)
		case .Fractured: 
			pulse = 0.7 + 0.3 * math.sin(time_sec * 8.0)
		case .Critical, .Overflow: 
			pulse = 0.5 + 0.5 * math.sin(time_sec * 25.0)
		}

		er_f := (140.0 * pulse) + (pct * 80.0)
		eb_f := (220.0 * pulse) - (pct * 60.0)
		
		er := u8(clamp(er_f, 0.0, 255.0))
		eb := u8(clamp(eb_f, 0.0, 255.0))

		sdl.SetRenderDrawColor(renderer, er, 0, eb, 255)
		fill := sdl.FRect{bx, bar_y, bw * pct, bar_h}
		sdl.RenderFillRect(renderer, &fill)

		sdl.SetRenderDrawColor(renderer, 130, 50, 190, 255)
		sdl.RenderRect(renderer, &bg)

		for i in 1 ..< 4 {
			pip := sdl.FRect{bx + bw * f32(i) / 4.0, bar_y, 1, bar_h}
			sdl.SetRenderDrawColor(renderer, 12, 0, 22, 180)
			sdl.RenderFillRect(renderer, &pip)
		}
	}

	{
		depth := min(gs.floor_depth, 10)
		for i in 0 ..< depth {
			bx := W - f32(i) * 6 - 8
			block := sdl.FRect{bx, bar_y + 1, 4, bar_h - 2}
			brightness := u8(80 + i * 15)
			sdl.SetRenderDrawColor(renderer, 60, 60, brightness, 255)
			sdl.RenderFillRect(renderer, &block)
		}
	}

	if gs.flash.timer > 0 {
		alpha := u8(clamp(gs.flash.timer * 220.0, 0.0, 255.0))
		sdl.SetRenderDrawColor(renderer, gs.flash.r, gs.flash.g, gs.flash.b, alpha)
		sdl.RenderFillRect(renderer, &sdl.FRect{0, 0, W, H})
	}

	if gs.game_over {
		sdl.SetRenderDrawColor(renderer, 0, 0, 0, 160)
		sdl.RenderFillRect(renderer, &sdl.FRect{0, 0, W, H})

		cx := W / 2; cy := H / 2 - 10
		sdl.SetRenderDrawColor(renderer, 200, 30, 30, 255)
		sdl.RenderFillRect(renderer, &sdl.FRect{cx - 18, cy - 4, 36, 8})
		sdl.RenderFillRect(renderer, &sdl.FRect{cx - 4, cy - 18, 8, 36})

		hint := sdl.FRect{cx - 20, cy + 26, 40, 3}
		sdl.SetRenderDrawColor(renderer, 180, 180, 180, 200)
		sdl.RenderFillRect(renderer, &hint)
	}

	sdl.SetRenderDrawBlendMode(renderer, cast(sdl.BlendMode)sdl.BLENDMODE_NONE)
}