#pragma once

#include "entities/Entity.h"
#include "entities/GhostAI.h"
#include <QColor>

class Ghost : public Entity {
public:
    Ghost(uint32_t id = 100, GhostType type = GhostType::Smiler);

    void reset(const QPoint &startTile);
    void update(double dt, const Player &player, const MazeGrid &grid, Difficulty difficulty);

    GhostType type() const { return m_ai.type(); }
    GhostBehaviorState behaviorState() const { return m_ai.state(); }
    bool isChasing() const { return m_ai.isChasing(); }

    QColor bodyColor() const;
    QColor eyeColor() const;
    double eyeGlowIntensity() const { return m_eyeGlow; }

private:
    void stepTowards(const QPoint &targetTile, double dt, double speed, const MazeGrid &grid);

    GhostAI m_ai;
    QPoint m_currentNextTile;
    double m_eyeGlow = 1.0;
    double m_glowTimer = 0.0;
};
