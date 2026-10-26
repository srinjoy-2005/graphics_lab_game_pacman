#pragma once

#include <QColor>

namespace Constants {

// Grid and Rendering geometry
constexpr int TILE_SIZE = 28;         // Screen pixels per maze tile
constexpr int MAZE_COLS = 29;         // Must be odd for maze carve algorithms
constexpr int MAZE_ROWS = 29;         // Must be odd

// Viewport and Vision
constexpr int VISION_SQUARE_TILES = 11; // Player sees 11x11 square section of maze
constexpr int VIEW_WIDTH = 784;        // 28 * 28 tiles wide
constexpr int VIEW_HEIGHT = 784;       // 28 * 28 tiles high
constexpr int HUD_HEIGHT = 48;         // Top horror HUD height

// Entity Speeds (tiles per second)
constexpr double PLAYER_WALK_SPEED = 4.0;
constexpr double PLAYER_SPRINT_SPEED = 6.2;
constexpr double GHOST_PATROL_SPEED = 2.8;
constexpr double GHOST_CHASE_SPEED = 4.8;
constexpr double GHOST_STALKER_SPEED = 5.2;

// Detection and Horror Parameters
constexpr double SPRINT_NOISE_RADIUS = 7.5;   // Hearing distance when running
constexpr double WALK_NOISE_RADIUS = 3.5;     // Hearing distance when walking
constexpr double GHOST_HEARING_BOOST = 1.2;
constexpr double MAX_LINE_OF_SIGHT_TILES = 12.0;

// Horror Colors (Backrooms / Liminal space theme)
inline const QColor COLOR_BG = QColor(10, 10, 14);
inline const QColor COLOR_WALL = QColor(155, 138, 88);         // Musty mono-yellow wallpaper
inline const QColor COLOR_WALL_ACCENT = QColor(95, 82, 48);    // Wall shadow/trim
inline const QColor COLOR_FLOOR = QColor(58, 52, 40);          // Damp carpet
inline const QColor COLOR_FLOOR_GRID = QColor(44, 40, 30);     // Floor tile seams
inline const QColor COLOR_DARK_VOID = QColor(6, 6, 9);
inline const QColor COLOR_FLASHLIGHT = QColor(255, 240, 190, 30);
inline const QColor COLOR_EXIT = QColor(40, 230, 120);         // Phosphor green exit door
inline const QColor COLOR_BLOOD_RED = QColor(220, 20, 40);

// Physics & Engine
constexpr int TICK_RATE_MS = 25;       // 40 Hz physics simulation
constexpr int RENDER_RATE_MS = 16;     // ~60 FPS rendering

} // namespace Constants
