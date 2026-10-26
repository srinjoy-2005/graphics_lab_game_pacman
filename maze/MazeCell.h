#pragma once

#include "core/Types.h"

struct MazeCell {
    TileType type = TileType::Wall;
    bool visitedByPlayer = false; // Seen at least once (memory fog)
    bool inSquareVision = false;  // Currently visible in square flashlight window
    float lightIntensity = 0.0f;  // Current calculated illumination (0.0 = dark, 1.0 = lit)
    bool checkpointActivated = false;

    bool isWalkable() const {
        return type == TileType::Floor || type == TileType::ExitDoor || type == TileType::Checkpoint;
    }

    bool isWall() const {
        return type == TileType::Wall;
    }
};
