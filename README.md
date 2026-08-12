# 2D Furniture Placement Simulator

A C++ furniture-layout tool built to demonstrate core Object-Oriented Programming principles — inheritance, polymorphism, composition/aggregation, operator overloading, and the Adapter pattern — with both a console (ASCII) mode and a full SFML graphical mode.

Place furniture in a room, drag it around, rotate it, detect overlaps in real time, and save/load your layout as JSON.

## Features

- **Interactive placement** — drag furniture anywhere in the room, snap-to-grid toggle
- **Collision detection** — overlapping furniture is highlighted with a red border
- **Save / Load** — layouts persist to `layout.json` (via `S` / `L` keys)
- **Rotate / Delete** — manipulate placed items with keyboard shortcuts
- **Dual visualization** — a live ASCII map in the terminal alongside the SFML graphical window
- **Operator overloading** — compare furniture by area (`==`) and reposition with `+=`

## Controls

| Key | Action |
|---|---|
| `G` | Toggle snap-to-grid |
| `S` | Save layout to `layout.json` |
| `L` | Load layout from `layout.json` |
| `Del` / `Backspace` | Delete selected furniture |
| Drag (mouse) | Move furniture in the room |

## Design & OOP concepts

| Concept | Where it shows up |
|---|---|
| **Inheritance & polymorphism** | `Furniture` is the base class; `Table`, `Chair`, `Bed`, `Cupboard` derive from it and override `showDetails()` |
| **Composition / Aggregation** | `Room` owns a collection of `Furniture*` and enforces placement rules (`addFurniture`) — bounds checking and overlap detection |
| **Operator overloading** | `operator==` compares furniture by area; `operator+=` shifts a furniture's position by an `(x, y)` offset |
| **Adapter pattern** | `VisualAdapter.h` decouples simulation logic from rendering — it wraps a `Furniture*` model and syncs it to an SFML `sf::Sprite`, so the core engine has zero dependency on SFML |
| **Separation of concerns** | Simulation logic (`FurnitureSim.cpp/.h`) is fully independent of the rendering layer (`main_sfml.cpp`) — you could swap SFML for another renderer without touching the core engine |

## Project structure

```
.
├── FurnitureSim.h / .cpp   # Core simulation: Furniture hierarchy, Room, collision/bounds logic
├── VisualAdapter.h         # Adapter pattern: converts Furniture model state -> SFML sprites
├── main_sfml.cpp           # Graphical entry point: event loop, rendering, input handling
├── json.hpp                # nlohmann/json (single-header JSON library, MIT licensed)
├── layout.json             # Example saved layout
├── assets/                 # Sprites: Bed.png, Chair.png, Floor.png, Sofa.png, Table.png
├── run_simulator.bat       # Windows build + run script
```

## Building & running

**Requirements:** a C++17 compiler and [SFML](https://www.sfml-dev.org/) (graphics, window, system modules).

```bash
g++ -std=c++17 main_sfml.cpp FurnitureSim.cpp -I. -Inlohmann -lsfml-graphics -lsfml-window -lsfml-system -o furnsim_sfml.exe
./furnsim_sfml.exe
```

On Windows, `run_simulator.bat` does this for you.

The program first prompts for room dimensions in the console (defaults to `800 600`), prints an ASCII map, then launches the SFML window for interactive placement.

## Screenshots

*(see `screenshots/` — console init, graphical simulation, collision detection)*

## Conclusion

This project demonstrates how OOP principles translate into a real, interactive application: separating simulation logic (C++) from rendering (SFML) keeps the codebase modular, and polymorphism means new furniture types can be added without touching the core engine.

## Acknowledgements

- JSON handling via [nlohmann/json](https://github.com/nlohmann/json) (MIT License).
