# Pacman Maze: Survival in the Dark

A 2D top-down horror maze survival game inspired by Pacman and Backrooms/liminal survival horror, built with C++17 and Qt 6.

---

## 🎮 Gameplay & Mechanics

1. **Title, Rules & Difficulty Selection**:
   - The game begins with an atmospheric Dark Horror Splash Screen detailing the lore, rules, and controls.
   - Choose your challenge level with <kbd>TAB</kbd> or keys <kbd>1</kbd> / <kbd>2</kbd> / <kbd>3</kbd>:
     - **EASY**: Relaxed entity speeds (~2.5 chase vs 4.2 walk), 3.5-tile detection.
     - **NORMAL**: Balanced experience (~3.2 chase vs 6.4 sprint), 4.0-tile detection.
     - **HARD**: Fast relentless predators (~4.0 chase), 4.5-tile detection.
   - Press <kbd>Space</kbd> or <kbd>Enter</kbd> to begin the 3-level session.

2. **Fog Blindness (~4 Tiles Detection Limit)**:
   - Entities are now **completely BLIND beyond ~4 tiles** due to the dense dark fog!
   - They cannot spot you from across long corridors anymore.
   - Footstep hearing is tuned down: walking is silent, sprinting can only be heard within 3.2 tiles.
   - If an entity pursues you, rounding a corner or breaking line of sight for ~2.8 seconds causes them to lose track and return to patrolling!

3. **Entity Types & Eye Visual Intel**:
   - **The Smiler** (White Glowing Eyes + Creepy Smile): Lurks quietly in the dark.
   - **The Stalker** (Amber/Yellow Eyes): Navigates corridors methodically.
   - **The Hound** (Sickly Yellow-Green Eyes): Highly sensitive to sprinting footstep noise.
   - **BLOOD RED EYES**: Entity has locked visual line-of-sight on you and is in active pursuit! Turn corners immediately to break sight!

6. **Controls**:
   - <kbd>W</kbd>, <kbd>A</kbd>, <kbd>S</kbd>, <kbd>D</kbd> or <kbd>Arrow Keys</kbd>: Move survivor
   - <kbd>Shift</kbd>: Sprint (drains stamina and alerts entities through noise vibrations!)
   - <kbd>F</kbd>: Toggle flashlight
   - <kbd>Space</kbd> / <kbd>Enter</kbd>: Begin Session / Respawn at Checkpoint / Enter Next Level
   - <kbd>R</kbd>: Retry level (on death) or re-roll a brand new 3-level session
   - <kbd>Esc</kbd>: Return to Rules Splash Screen

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

### 1. Desktop (Native Windows):
- Launch by double-clicking [`run.bat`](file:///c:/Users/soura/OneDrive/Desktop/pacman_maze/run.bat) in the project root.
- Alternatively, open [`PacmanMaze.pro`](file:///c:/Users/soura/OneDrive/Desktop/pacman_maze/PacmanMaze.pro) in **Qt Creator** and click **Run (<kbd>Ctrl</kbd> + <kbd>R</kbd>)**.

### 2. Instant Web Deployment (Browser / Zero Setup):
A production-ready, zero-dependency HTML5 / Canvas / WebAudio deployment is included in [`web/`](file:///c:/Users/soura/OneDrive/Desktop/pacman_maze/web/):
- **Local Run**: Double-click [`run_web.bat`](file:///c:/Users/soura/OneDrive/Desktop/pacman_maze/run_web.bat) to launch the local web server at `http://localhost:8080`.
- **Cloud Deploy**: Drag-and-drop the `web/` folder directly to [Netlify](https://app.netlify.com/drop), Vercel, or push to GitHub Pages!

### 3. Direct Qt for WebAssembly (WASM / JS):
The C++ codebase is fully equipped with `#ifdef __EMSCRIPTEN__` and Qt WASM flags:
1. In `C:\Qt\MaintenanceTool.exe`, ensure the **Qt 6.11.1 -> WebAssembly** kit is checked.
2. Install Emscripten SDK matching Qt 6.11:
   ```cmd
   git clone https://github.com/emscripten-core/emsdk.git
   cd emsdk
   emsdk.bat install 3.1.56
   emsdk.bat activate 3.1.56
   ```
3. Run qmake with the WASM kit:
   ```cmd
   C:\Qt\6.11.1\wasm_singlethread\bin\qmake.exe PacmanMaze.pro
   mingw32-make -j4
   ```
   This generates `PacmanMaze.html`, `PacmanMaze.js`, and `PacmanMaze.wasm`.
