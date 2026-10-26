#pragma once

#include <memory>
#include "engine/WorldState.h"
#include "maze/IMazeGenerator.h"

class GameEngine {
public:
    GameEngine();

    void startNewSession(bool procedural = true);
    void beginGame();
    void nextLevel();
    void restartLevel();
    void respawnAtCheckpoint();

    void update(double dt, const PlayerInput &localInput);

    WorldState &world() { return m_world; }
    const WorldState &world() const { return m_world; }

    uint32_t localPlayerId() const { return m_localPlayerId; }
    const Player *localPlayer() const { return m_world.player(m_localPlayerId); }

    double closestGhostDistance() const;

private:
    void generateSessionLevels();
    void loadLevel(int levelNum);
    void spawnGhosts(int count);
    void checkCollisions();

    WorldState m_world;
    uint32_t m_localPlayerId = 1;
    std::unique_ptr<IMazeGenerator> m_generator;
    bool m_procedural = true;
    double m_deathTimer = 0.0;
    double m_winTimer = 0.0;
};
