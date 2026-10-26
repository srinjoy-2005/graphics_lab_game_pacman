#include "engine/GameEngine.h"
#include "maze/BacktrackerGenerator.h"
#include "maze/StaticMazeGenerator.h"
#include "core/Constants.h"
#include "core/SoundManager.h"
#include <QRandomGenerator>
#include <cmath>

GameEngine::GameEngine() {
    startNewSession(true);
}

void GameEngine::startNewSession(bool procedural) {
    m_procedural = procedural;
    m_world.reset();
    m_world.setLives(3);
    m_world.setLevel(1);

    if (m_procedural) {
        m_generator = std::make_unique<BacktrackerGenerator>(0.12);
    } else {
        m_generator = std::make_unique<StaticMazeGenerator>();
    }

    generateSessionLevels();
    m_world.setGameState(GameState::Splash);
}

void GameEngine::generateSessionLevels() {
    m_world.sessionLevels().clear();

    const int levelSizes[3][2] = {
        {25, 25},
        {29, 29},
        {31, 31}
    };

    for (int i = 0; i < 3; ++i) {
        MazeGrid grid;
        unsigned int seed = static_cast<unsigned int>(QRandomGenerator::global()->generate());
        grid.load(*m_generator, levelSizes[i][0], levelSizes[i][1], seed);
        m_world.sessionLevels().append(grid);
    }
}

void GameEngine::beginGame() {
    m_world.setLevel(1);
    loadLevel(1);
}

void GameEngine::loadLevel(int levelNum) {
    m_world.setLevel(levelNum);
    int idx = qBound(0, levelNum - 1, m_world.sessionLevels().size() - 1);

    // Load pre-generated maze for this session
    m_world.grid() = m_world.sessionLevels()[idx];
    m_world.grid().resetFog();

    // Checkpoint starts at maze entrance
    m_world.setCurrentCheckpoint(m_world.grid().startPos());

    // Initialize local player
    Player local(m_localPlayerId, "Survivor");
    local.reset(m_world.grid().startPos());
    m_world.players().clear();
    m_world.addPlayer(local);

    // Initial vision update
    m_world.grid().updateVision(local.tilePos(), Constants::VISION_SQUARE_TILES / 2);

    // Spawn entities (scaling count: 2, 3, 4)
    int ghostCount = qMin(5, 1 + levelNum);
    spawnGhosts(ghostCount);

    m_world.setGameState(GameState::Exploring);
    m_deathTimer = 0.0;
    m_winTimer = 0.0;
}

void GameEngine::respawnAtCheckpoint() {
    Player *p = m_world.player(m_localPlayerId);
    if (!p) return;

    p->reset(m_world.currentCheckpoint());
    m_world.grid().updateVision(p->tilePos(), Constants::VISION_SQUARE_TILES / 2);

    int ghostCount = qMin(5, 1 + m_world.level());
    spawnGhosts(ghostCount);

    m_world.setGameState(GameState::Exploring);
}

void GameEngine::restartLevel() {
    loadLevel(m_world.level());
}

void GameEngine::nextLevel() {
    if (m_world.level() < 3) {
        loadLevel(m_world.level() + 1);
    } else {
        m_world.setGameState(GameState::Victory);
        SoundManager::instance().playEscaped();
    }
}

void GameEngine::spawnGhosts(int count) {
    m_world.ghosts().clear();
    GhostType types[] = { GhostType::Smiler, GhostType::Stalker, GhostType::Hound };

    const QPoint &safePos = m_world.currentCheckpoint();
    int spawned = 0;
    int attempts = 0;

    while (spawned < count && attempts < 600) {
        attempts++;
        int gx = QRandomGenerator::global()->bounded(1, m_world.grid().cols() - 1);
        int gy = QRandomGenerator::global()->bounded(1, m_world.grid().rows() - 1);

        if (m_world.grid().isWalkable(gx, gy)) {
            double dist = std::hypot(gx - safePos.x(), gy - safePos.y());
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

    if (st != GameState::Exploring && st != GameState::Spotted) {
        return;
    }

    Player *p = m_world.player(m_localPlayerId);
    if (!p) return;

    // Update Player (Movement audio is turned off)
    p->update(dt, localInput, m_world.grid());

    // Update limited square vision window
    m_world.grid().updateVision(p->tilePos(), Constants::VISION_SQUARE_TILES / 2);

    // Update Ghosts
    bool anyChasing = false;
    for (Ghost &g : m_world.ghosts()) {
        g.update(dt, *p, m_world.grid(), m_world.difficulty());
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
            m_world.decrementLife();
            SoundManager::instance().playCaught();

            if (m_world.lives() > 0) {
                m_world.setGameState(GameState::Caught);
            } else {
                m_world.setGameState(GameState::GameOver);
            }
            return;
        }
    }

    // 2. Checkpoint activation
    QPoint pt = p->tilePos();
    if (m_world.grid().isValid(pt.x(), pt.y())) {
        MazeCell &cell = m_world.grid().cell(pt.x(), pt.y());
        if (cell.type == TileType::Checkpoint && !cell.checkpointActivated) {
            cell.checkpointActivated = true;
            m_world.setCurrentCheckpoint(pt);
            SoundManager::instance().playCheckpoint();
        }
    }

    // 3. Escape point reached
    const QPoint &exitPos = m_world.grid().exitPos();
    double distToExit = std::hypot(p->x() - exitPos.x(), p->y() - exitPos.y());
    if (distToExit < 0.65) {
        m_world.setGameState(GameState::Escaped);
        SoundManager::instance().playEscaped();
    }
}
