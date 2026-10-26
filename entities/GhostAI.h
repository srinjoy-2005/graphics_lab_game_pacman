#pragma once

#include <QPoint>
#include "core/Types.h"
#include "maze/MazeGrid.h"

class Player;

class GhostAI {
public:
    GhostAI(GhostType type = GhostType::Smiler);

    void reset();
    void update(double dt,
                const QPoint &currentTile,
                const Player &player,
                const MazeGrid &grid,
                Difficulty difficulty,
                QPoint &outTargetTile,
                double &outSpeed);

    GhostBehaviorState state() const { return m_state; }
    GhostType type() const { return m_type; }
    bool isChasing() const { return m_state == GhostBehaviorState::Chase; }
    const QPoint &lastKnownPos() const { return m_lastKnownPlayerPos; }

private:
    void updateSensory(const QPoint &currentTile, const Player &player, const MazeGrid &grid, Difficulty difficulty);
    QPoint pickPatrolTarget(const QPoint &currentTile, const MazeGrid &grid);

    GhostType m_type;
    GhostBehaviorState m_state = GhostBehaviorState::Patrol;
    QPoint m_patrolTarget;
    QPoint m_lastKnownPlayerPos;
    double m_stateTimer = 0.0;
    Direction m_patrolDir = Direction::None;
};
