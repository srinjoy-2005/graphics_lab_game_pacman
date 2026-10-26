#pragma once

#include <QString>
#include <QPoint>

enum class Direction {
    None,
    Up,
    Down,
    Left,
    Right
};

enum class Difficulty {
    Easy,
    Normal,
    Hard
};

enum class GameState {
    Splash,      // Rules and introduction screen
    Ready,
    Exploring,
    Spotted,
    Caught,      // Lost a life; respawn at checkpoint if lives remain
    GameOver,    // All 3 lives lost; session terminated
    Escaped,     // Level cleared
    Victory,     // All 3 levels completed!
    Paused
};

enum class GhostType {
    Smiler,      // High line-of-sight range, fast chase
    Stalker,     // Sneaks closer, tries to ambush
    Hound        // Sensitive hearing, tracks footsteps
};

enum class GhostBehaviorState {
    Patrol,      // Wandering the maze corridors
    Investigate, // Heading towards heard sound / lost player
    Chase,       // Visual contact confirmed, chasing
    Cooldown     // Lost target, pausing before next patrol
};

enum class TileType {
    Wall,
    Floor,
    ExitDoor,
    Checkpoint
};

struct PlayerInput {
    Direction moveDir = Direction::None;
    bool sprint = false;
    bool toggleFlashlight = false;
    uint32_t sequenceNumber = 0; // For future multiplayer prediction & replay
};

struct Vector2D {
    double x = 0.0;
    double y = 0.0;

    Vector2D() = default;
    Vector2D(double px, double py) : x(px), y(py) {}

    double lengthSquared() const { return x * x + y * y; }
};
