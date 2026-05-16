package main

import "core:fmt"
import sdl "vendor:sdl3"

WINDOW_WIDTH :: 800
WINDOW_HEIGHT :: 600

main :: proc() {
	if !sdl.Init({.VIDEO, .AUDIO}) {
		fmt.eprintfln("SDL_Init Error: %s", sdl.GetError())
		return
	}

	defer sdl.Quit()

	window := sdl.CreateWindow("Entropy Descent", WINDOW_WIDTH, WINDOW_HEIGHT, {})

	defer sdl.DestroyWindow(window)

	renderer := sdl.CreateRenderer(window, nil)
	if renderer == nil {
		fmt.eprintfln("SDL_CreateRender Error %s", sdl.GetError())
		return
	}

	defer sdl.DestroyRenderer(renderer)

	is_running := true
	last_time := sdl.GetTicks()

	for is_running {
		current_time := sdl.GetTicks()
		dt := f32(current_time - last_time) / 1000.0
		last_time = current_time

		if dt > 0.05 {
			dt = 0.05
		}

		event: sdl.Event
		for sdl.PollEvent(&event) {
			#partial switch event.type {
			case .QUIT:
				is_running = false
			case .KEY_DOWN:
				if event.key.key == sdl.K_ESCAPE {
					is_running = false
				}
			}
		}
		sdl.SetRenderDrawColor(renderer, 3, 21, 31, 255)
		sdl.RenderClear(renderer)
		sdl.RenderPresent(renderer)
	}

}

