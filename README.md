# Entropy Descent: C++ Narrative Roguelike

A ground-up, engine-less 2D roguelike built natively in modern C++. This project bypasses heavy commercial game engines to directly manipulate hardware, memory, and systemic logic. 

The core philosophy of this project is **Emergent Physics & Progression**—replacing standard hack-and-slash combat with systemic, grid-based physics, environmental manipulation, and deep narrative lore.

## The Tech Stack
* **Language:** Modern C++ (Targeting C++23 standards for maximum memory safety and performance).
* **Build System:** CMake
* **Architecture:** Entity Component System (ECS)
* **Core Libraries:** * `SDL3` (Hardware-accelerated windowing, input, and 2D rendering)
  * `EnTT` (Industry-standard, blazingly fast ECS framework)

## Current Features
The foundation of the engine is fully operational, featuring:

* **Algorithmic World Generation:** Organic cavern networks generated via **Cellular Automata**, smoothed and guaranteed fully interconnected using **Breadth-First Search (BFS)** graph theory to prevent isolated spawn pockets.
* **Pushdown Automata State Machine:** A robust, stack-based state manager using `std::unique_ptr` to seamlessly transition between menus and gameplay without monolithic `switch` statements or memory leaks.
* **Entity Component System (ECS):** Complete separation of data (`Components`) and logic (`Systems`), ensuring cache-friendly performance even with massive entity counts.
* **Dynamic Field of View (FOV):** 360-degree mathematical raycasting that dynamically calculates line-of-sight, casting deep shadows against organic wall structures and retaining explored memory.
* **Smart Entity Spawning:** Randomized enemy generation constrained by logic checks (preventing wall-spawning or immediate player swarming) and tied directly into the FOV renderer for stealth.
* **Massive World Tracking:** A decoupled 2D View Matrix (Camera) that effortlessly centers and tracks the player across a sprawling 200x150 grid.

## Roadmap & Future Systems
The engine foundation is laid. The next phases focus on systemic interactions and data-driven design to support the narrative hook.

- [ ] **Grid Collision & Resolution:** Prevent entities from overlapping and handle wall "bumping" elegantly.
- [ ] **Pathfinding AI:** Implement A* (A-Star) or Dijkstra algorithms so enemies can intelligently navigate complex cave systems to hunt the player.
- [ ] **The Event Bus (Observer Pattern):** A fully decoupled messaging system where the combat system, audio engine, and loot manager can communicate without hardcoded dependencies.
- [ ] **Data-Driven Architecture:** Integration of a JSON parser to load enemies, items, and narrative lore fragments from external files, allowing for massive content scaling without recompiling C++.
- [ ] **Physics-Based Combat Loop:** Implementing grid-based translations of n-body physics, such as gravity wells, knockbacks, and environmental state changes (e.g., turning water to steam to block line of sight).

## 🛠️ Build Instructions (Linux)
Ensure you have GCC 14+ and CMake 3.31+ installed. SDL3 must be installed or built from source on your system.

```bash
# Clone the repository
git clone [https://github.com/yourusername/entropy-descent.git](https://github.com/yourusername/entropy-descent.git)
cd entropy-descent

# Generate build files and compile
mkdir build && cd build
cmake ..
make

# Run the engine
./Roguelike
