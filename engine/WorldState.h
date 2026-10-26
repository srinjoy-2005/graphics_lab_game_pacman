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

private:
    uint64_t m_tick = 0;
    int m_level = 1;
    GameState m_gameState = GameState::Ready;
    MazeGrid m_grid;
    QMap<uint32_t, Player> m_players;
    QVector<Ghost> m_ghosts;
};
