# Pacman Maze: Survival in the Dark

A 2D top-down horror maze survival game inspired by Pacman and Backrooms/liminal survival horror, built with C++17 and Qt 6.

---

## 🎮 Gameplay & Mechanics

1. **The Objective**:
   - You are placed at the beginning of an unknown, sprawling labyrinth.
   - An exit portal exists deep in the maze, but you **do not know where it is** at the start.
   - You must explore the dark corridors, avoid entities, and find the illuminated green exit portal to escape to the next level.

2. **Fog of War & Vision**:
   - The player only sees an **$11 \times 11$ square section** of the maze around them at any given time.
   - Outside the active square vision window lies pitch-black darkness.
   - Cells you have visited remain faintly etched into your memory, but entities and exit beacons remain hidden until illuminated by your active vision window.
   - Flashlight flickers dynamically, adding to the claustrophobic horror atmosphere.

3. **Backrooms Horror Entity AI**:
   - Entities do not follow predictable classical Pacman routines:
     - **Patrol**: Wanders corridors, turning down hallways and avoiding instant backtracking.
     - **Line-of-Sight Detection**: Uses raycasting corridor checks. If an entity gets an unobstructed line of sight to you, it shrieks and dashes into high-speed pursuit!
     - **Sound & Footstep Hearing**: Walking and sprinting emit noise waves. The Hound entity has heightened hearing and will investigate suspicious sounds.
     - **Stalking & Memory**: When you break line of sight around corners, entities search your last known location before returning to patrol.
     - **Instant Kill**: Contact with any entity is fatal!
   - Dynamic heartbeat audio synthesizer: Pulse accelerates and grows louder as entities draw near.

4. **Controls**:
   - `W`, `A`, `S`, `D` or `Arrow Keys`: Move survivor
   - `Shift`: Sprint (consumes stamina and generates more footstep noise!)
   - `F`: Toggle flashlight
   - `R`: Generate a new procedural maze
   - `Space`: Retry after death / Advance to next level after escape

---

## 🏗️ Modular Architecture (~200 Lines Per File)

The codebase is strictly modularized with separation of concerns:

```
pacman_maze/
├── PacmanMaze.pro           # Qt qmake configuration
├── main.cpp                 # Application entry point
├── run.bat                  # One-click launcher with Qt in PATH
├── core/
│   ├── Constants.h          # Grid sizes, horror color palette, speeds, noise radii
│   ├── Types.h              # GameState, Direction, GhostType, PlayerInput
│   ├── SoundManager.h       # Procedural 8-bit retro horror audio synthesizer
│   └── SoundManager.cpp
├── maze/
│   ├── MazeCell.h           # Cell definition (wall, floor, exit, vision flags)
│   ├── IMazeGenerator.h     # Interface enabling dynamic maze generation
│   ├── BacktrackerGenerator.h # Procedural DFS maze generator with loop carving
│   ├── BacktrackerGenerator.cpp
│   ├── StaticMazeGenerator.h  # Deterministic test maze generator
│   ├── StaticMazeGenerator.cpp
│   ├── MazeGrid.h           # Grid queries, line-of-sight raycasting, BFS pathfinding
│   └── MazeGrid.cpp
├── entities/
│   ├── Entity.h             # Base entity (ID, float positions, facing)
│   ├── Player.h             # Player state, stamina, sprint, noise radius
│   ├── Player.cpp
│   ├── GhostAI.h            # Backrooms state machine (Patrol, Investigate, Chase, Cooldown)
│   ├── GhostAI.cpp
│   ├── Ghost.h              # Ghost wrapper, archetype appearance, eye glow
│   └── Ghost.cpp
├── engine/
│   ├── WorldState.h         # Authoritative state container (players map, ghosts, grid, tick)
│   ├── WorldState.cpp
│   ├── GameEngine.h         # Simulation loop, collision handling, heartbeat trigger
│   └── GameEngine.cpp
└── ui/
    ├── Camera.h             # Smooth lerp camera tracking the player
    ├── Camera.cpp
    ├── HorrorRenderer.h     # Top-down horror renderer, square vision shroud, HUD
    ├── HorrorRenderer.cpp
    ├── GameView.h           # QWidget window, 40Hz physics & 60Hz rendering timers
    └── GameView.cpp
```

---

## 🔮 Future-Proofing: Dynamic Mazes & Multiplayer

### 1. Dynamic Procedural Mazes
- Designed around the abstract interface `IMazeGenerator`.
- Any maze generation algorithm (e.g. Wilson's, Kruskal's, Cellular Automata, BSP rooms) can be plugged in seamlessly without modifying game logic or rendering code.
- Pressing `R` instantly generates a fresh randomized labyrinth using `BacktrackerGenerator`.

### 2. Multiplayer Foundation
- `Player` instances are identified by discrete `uint32_t playerId`.
- `WorldState` stores players inside `QMap<uint32_t, Player>` rather than a single hardcoded player variable.
- Inputs are encapsulated as discrete `PlayerInput` structs with sequence numbers, ready for client-server prediction, serialization, and network replication.

---

## 🔨 How to Build and Run

### Running Directly (Standalone):
All required Qt runtime libraries and MinGW DLLs are deployed directly into the `release\` directory. You can launch the game in either of two ways:
1. Double-click [`run.bat`](file:///c:/Users/soura/OneDrive/Desktop/pacman_maze/run.bat) in the project root.
2. Directly double-click [`release\PacmanMaze.exe`](file:///c:/Users/soura/OneDrive/Desktop/pacman_maze/release/PacmanMaze.exe).

### Building with Qt / MinGW:
```powershell
$env:PATH = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.11.1\mingw_64\bin;" + $env:PATH
qmake PacmanMaze.pro
mingw32-make -j4
windeployqt release\PacmanMaze.exe
```
You can also open [`PacmanMaze.pro`](file:///c:/Users/soura/OneDrive/Desktop/pacman_maze/PacmanMaze.pro) directly in **Qt Creator** and click **Run (Ctrl + R)**.
