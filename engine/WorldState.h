#pragma once

#include <QMap>
#include <QVector>
#include "core/Types.h"
#include "maze/MazeGrid.h"
#include "entities/Player.h"
#include "entities/Ghost.h"

class WorldState {
public:
    WorldState();

    void reset();
    void addPlayer(const Player &player);
    Player *player(uint32_t id);
    const Player *player(uint32_t id) const;

    QMap<uint32_t, Player> &players() { return m_players; }
    const QMap<uint32_t, Player> &players() const { return m_players; }

    QVector<Ghost> &ghosts() { return m_ghosts; }
    const QVector<Ghost> &ghosts() const { return m_ghosts; }

    MazeGrid &grid() { return m_grid; }
    const MazeGrid &grid() const { return m_grid; }

    GameState gameState() const { return m_gameState; }
    void setGameState(GameState state) { m_gameState = state; }

    uint64_t tick() const { return m_tick; }
    void incrementTick() { ++m_tick; }

    int level() const { return m_level; }
    void setLevel(int lvl) { m_level = lvl; }

    Difficulty difficulty() const { return m_difficulty; }
    void setDifficulty(Difficulty diff) { m_difficulty = diff; }
    void cycleDifficulty() {
        if (m_difficulty == Difficulty::Easy) m_difficulty = Difficulty::Normal;
        else if (m_difficulty == Difficulty::Normal) m_difficulty = Difficulty::Hard;
        else m_difficulty = Difficulty::Easy;
    }

    int lives() const { return m_lives; }
    void setLives(int lives) { m_lives = lives; }
    void decrementLife() { if (m_lives > 0) --m_lives; }

    const QPoint &currentCheckpoint() const { return m_currentCheckpoint; }
    void setCurrentCheckpoint(const QPoint &pt) { m_currentCheckpoint = pt; }

    QVector<MazeGrid> &sessionLevels() { return m_sessionLevels; }
    const QVector<MazeGrid> &sessionLevels() const { return m_sessionLevels; }

private:
    uint64_t m_tick = 0;
    int m_level = 1;
    int m_lives = 3;
    Difficulty m_difficulty = Difficulty::Normal;
    QPoint m_currentCheckpoint;
    GameState m_gameState = GameState::Splash;
    MazeGrid m_grid;
    QVector<MazeGrid> m_sessionLevels;
    QMap<uint32_t, Player> m_players;
    QVector<Ghost> m_ghosts;
};
