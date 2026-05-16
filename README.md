# Entropy Descent

## About
Entropy Descent is a tactical, grid-based roguelike. The game focuses on strategic movement, combat, and narrative progression within a dynamically simulated environment. 

## Architecture: The Odin Pivot
This project was recently rewritten from the ground up, porting from C++23 to the Odin programming language. The goal of this transformation was to eliminate build friction, drop object-oriented overhead, and fully embrace Data-Oriented Design (DOD).

### Key Technical Shifts
* **Language:** Odin replaced C++.
* **Custom ECS:** The third-party EnTT library was removed. The engine now uses a custom, from-scratch Entity Component System utilizing Odin's native `#soa` (Structure of Arrays) for cache-friendly memory access.
* **No Build Systems:** CMake and associated build scripts were removed. The project relies exclusively on the native Odin compiler for instant iteration times.
* **Flat Architecture:** Logic is strictly separated from data. State is held in plain data structs, manipulated by isolated, procedural systems.
* **Rendering & Input:** Handled via direct, zero-abstraction calls to Odin's `vendor:sdl3` library.

## Building and Running
Ensure you have the Odin compiler and SDL3 installed on your system.

To compile and run the game, navigate to the project root and execute:

```bash
odin run .
