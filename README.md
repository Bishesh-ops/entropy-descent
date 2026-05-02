# Entropy Descent

A systemic, data-driven 2D narrative roguelike built natively in modern C++23. Bypassing commercial game engines, Entropy Descent relies on hardware-level memory manipulation and a highly modular Entity Component System (ECS) to simulate chaotic physics, environmental synergies, and emergent progression.

The core philosophy of this project is **Emergent Physics & Progression**. We replace standard hack-and-slash combat with deep systemic interactions, Lua-driven environmental spellcasting, and the overarching "Entropy" mutation system.

## 🛠 Tech Stack
* **Language:** Modern C++ (Targeting C++23 standards for maximum memory safety and performance).
* **Build System:** CMake
* **Scripting:** Lua 5.4 bound via `sol2` for data-driven combat and AI logic.
* **Architecture:** Entity Component System (ECS) powered by `EnTT` (Industry-standard, blazingly fast ECS framework).
* **Rendering & Core:** `SDL3` (Hardware-accelerated windowing, input, and 2D rendering).
* **Live Tooling:** Dear ImGui for real-time engine debugging and editing.

## Core Features & Systems
The engine is deeply modular and heavily data-driven, allowing for massive content scaling without recompiling the C++ core:

* **Entity Component System (ECS):** Complete separation of data (`Components`) and logic (`Systems`), ensuring cache-friendly performance. Uses `EnTT`'s modern storage iteration for strict memory safety.
* **Pushdown Automata State Machine:** A robust, stack-based state manager seamlessly transitioning between `MenuState`, `PlayState`, and the pivotal `VowState`.
* **Hybrid Spatial Architecture:** Blends grid-based logic for FOV and pathfinding with pixel-precise continuous movement and AABB collision resolution for entities and particles.
* **Lua-Driven Combat & Spells:** The C++ engine acts purely as a dispatcher. Spells and AI are handled via Lua scripts (`scripts/spells.lua`, `scripts/ai.lua`), allowing real-time injection of logic and environmental synergies (e.g., casting fire on a scorched tile).
* **The Entropy & Vow System:** A unique resource mechanic. High Entropy triggers chaotic cascades in the world state. Reaching 100 Entropy forces the player into `VowState` to accept permanent, run-altering curses/buffs (e.g., sacrificing FOV for massive spell AoE).
* **Algorithmic World Generation:** Organic cavern networks generated via **Cellular Automata**, smoothed and guaranteed fully interconnected using **Breadth-First Search (BFS)** graph theory.
* **Data Pipelines:** Python scripts and JSON payloads (`data/enemies.json`, `data/items.json`) pre-validate and build massive amounts of content declaratively.

##  Execution Roadmap
The engine foundation is laid. Current and future development focuses on building a full execution slice of the game:

- [x] **Tooling & Data Pipeline:** Procedural JSON generation and data validation.
- [ ] **Live Debug & Editor:** ImGui-powered Entity Inspector, Lua Console, and performance profiling.
- [ ] **Visual Juice & Polish:** Screen shake, floating combat text, and `SDL_mixer` audio integration.
- [ ] **MVP Gameplay Loop:** A fully scoped single-floor slice featuring combat, entropy interactions, and basic win/loss conditions.
- [ ] **Web Build & Deployment:** Emscripten compilation to host the engine natively on the web via WebAssembly.

## 🔧 Build Instructions (Linux)
Ensure you have **GCC 14+** and **CMake 3.31+** installed. `SDL3` must be available or built from source on your system. 

```bash
# Clone the repository
git clone https://github.com/bishesh-ops/entropy-descent.git
cd entropy-descent

# Generate build files and compile
mkdir build && cd build
cmake ..
make

# Run the engine
./Roguelike
```
