#include "engine/GameEngine.h"
#include "maze/BacktrackerGenerator.h"
#include "maze/StaticMazeGenerator.h"
#include "core/Constants.h"
#include "core/SoundManager.h"
#include <QRandomGenerator>
#include <cmath>

GameEngine::GameEngine() {
    startNewGame(true);
}

void GameEngine::startNewGame(bool procedural) {
    m_procedural = procedural;
    m_world.reset();
    m_world.setLevel(1);

    if (m_procedural) {
        m_generator = std::make_unique<BacktrackerGenerator>(0.12);
    } else {
        m_generator = std::make_unique<StaticMazeGenerator>();
    }

    restartLevel();
}

void GameEngine::nextLevel() {
    m_world.setLevel(m_world.level() + 1);
    restartLevel();
}

void GameEngine::restartLevel() {
    // Generate maze
    unsigned int seed = static_cast<unsigned int>(QRandomGenerator::global()->generate());
    m_world.grid().load(*m_generator, Constants::MAZE_COLS, Constants::MAZE_ROWS, seed);

    // Initialize local player
    Player local(m_localPlayerId, "Survivor");
    local.reset(m_world.grid().startPos());
    m_world.players().clear();
    m_world.addPlayer(local);

    // Initial vision update
    m_world.grid().updateVision(local.tilePos(), Constants::VISION_SQUARE_TILES / 2);

    // Spawn entities (increased count with levels)
    int ghostCount = qMin(5, 2 + m_world.level());
    spawnGhosts(ghostCount);

    m_world.setGameState(GameState::Exploring);
    m_deathTimer = 0.0;
    m_winTimer = 0.0;
}

void GameEngine::spawnGhosts(int count) {
    m_world.ghosts().clear();
    GhostType types[] = { GhostType::Smiler, GhostType::Stalker, GhostType::Hound };

    const QPoint &start = m_world.grid().startPos();
    int spawned = 0;
    int attempts = 0;

    while (spawned < count && attempts < 500) {
        attempts++;
        int gx = QRandomGenerator::global()->bounded(1, Constants::MAZE_COLS - 1);
        int gy = QRandomGenerator::global()->bounded(1, Constants::MAZE_ROWS - 1);

        if (m_world.grid().isWalkable(gx, gy)) {
            // Keep at least 8 tiles away from start position
            double dist = std::hypot(gx - start.x(), gy - start.y());
            if (dist > 8.0) {
                Ghost g(100 + spawned, types[spawned % 3]);
                g.reset(QPoint(gx, gy));
                m_world.ghosts().append(g);
                spawned++;
            }
        }
    }
}

double GameEngine::closestGhostDistance() const {
    const Player *p = localPlayer();
    if (!p) return 999.0;

    double minDist = 999.0;
    for (const Ghost &g : m_world.ghosts()) {
        double dist = std::hypot(g.x() - p->x(), g.y() - p->y());
        if (dist < minDist) {
            minDist = dist;
        }
    }
    return minDist;
}

void GameEngine::update(double dt, const PlayerInput &localInput) {
    m_world.incrementTick();
    GameState st = m_world.gameState();

    if (st == GameState::Caught) {
        m_deathTimer += dt;
        return;
    }
    if (st == GameState::Escaped) {
        m_winTimer += dt;
        return;
    }

    Player *p = m_world.player(m_localPlayerId);
    if (!p) return;

    // Update Player
    p->update(dt, localInput, m_world.grid());

    // Play footstep sounds
    if (p->isMoving()) {
        SoundManager::instance().playFootstep(p->isSprinting());
    }

    // Update limited square vision window
    m_world.grid().updateVision(p->tilePos(), Constants::VISION_SQUARE_TILES / 2);

    // Update Ghosts
    bool anyChasing = false;
    for (Ghost &g : m_world.ghosts()) {
        g.update(dt, *p, m_world.grid());
        if (g.isChasing()) {
            anyChasing = true;
        }
    }

    // Horror state and heartbeat audio
    if (anyChasing) {
        m_world.setGameState(GameState::Spotted);
    } else {
        m_world.setGameState(GameState::Exploring);
    }

    double ghostDist = closestGhostDistance();
    if (ghostDist < 12.0) {
        double factor = qBound(0.0, ghostDist / 12.0, 1.0);
        SoundManager::instance().playHeartbeat(factor);
    }

    checkCollisions();
}

void GameEngine::checkCollisions() {
    Player *p = m_world.player(m_localPlayerId);
    if (!p) return;

    // 1. Ghost instant kill on contact
    for (const Ghost &g : m_world.ghosts()) {
        double dist = std::hypot(g.x() - p->x(), g.y() - p->y());
        if (dist < 0.62) {
            m_world.setGameState(GameState::Caught);
            SoundManager::instance().playCaught();
            return;
        }
    }

    // 2. Escape point reached
    const QPoint &exitPos = m_world.grid().exitPos();
    double distToExit = std::hypot(p->x() - exitPos.x(), p->y() - exitPos.y());
    if (distToExit < 0.65) {
        m_world.setGameState(GameState::Escaped);
        SoundManager::instance().playEscaped();
    }
}
