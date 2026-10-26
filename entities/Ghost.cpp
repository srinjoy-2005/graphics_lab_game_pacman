#include "entities/Ghost.h"
#include <cmath>

Ghost::Ghost(uint32_t id, GhostType type)
    : Entity(id), m_ai(type) {}

void Ghost::reset(const QPoint &startTile) {
    m_x = startTile.x();
    m_y = startTile.y();
    m_currentNextTile = startTile;
    m_ai.reset();
    m_glowTimer = 0.0;
}

QColor Ghost::bodyColor() const {
    switch (m_ai.type()) {
        case GhostType::Smiler:  return QColor(15, 15, 20);      // Pitch black shadow figure
        case GhostType::Stalker: return QColor(35, 20, 25);      // Crimson stalker
        case GhostType::Hound:   return QColor(45, 35, 20);      // Muddy beast
    }
    return QColor(20, 20, 20);
}

QColor Ghost::eyeColor() const {
    if (m_ai.isChasing()) {
        return QColor(255, 30, 30); // Blood red piercing eyes in chase!
    }
    switch (m_ai.type()) {
        case GhostType::Smiler:  return QColor(240, 240, 255);   // Creepy grinning white eyes/teeth
        case GhostType::Stalker: return QColor(255, 140, 0);     // Amber stalking glow
        case GhostType::Hound:   return QColor(180, 220, 40);     // Sickly yellow-green eyes
    }
    return QColor(255, 255, 255);
}

void Ghost::update(double dt, const Player &player, const MazeGrid &grid) {
    m_glowTimer += dt * (m_ai.isChasing() ? 8.0 : 3.0);
    m_eyeGlow = 0.7 + 0.3 * std::sin(m_glowTimer);

    QPoint targetTile;
    double speed = 2.5;
    m_ai.update(dt, tilePos(), player, grid, targetTile, speed);

    stepTowards(targetTile, dt, speed, grid);
}

void Ghost::stepTowards(const QPoint &targetTile, double dt, double speed, const MazeGrid &grid) {
    // If reached next tile or invalid, get next waypoint along path
    double distToNext = std::hypot(m_currentNextTile.x() - m_x, m_currentNextTile.y() - m_y);
    if (distToNext < 0.08 || !grid.isWalkable(m_currentNextTile.x(), m_currentNextTile.y())) {
        m_x = m_currentNextTile.x();
        m_y = m_currentNextTile.y();
        m_currentNextTile = grid.getNextStepTowards(tilePos(), targetTile);
    }

    double dx = m_currentNextTile.x() - m_x;
    double dy = m_currentNextTile.y() - m_y;
    double len = std::hypot(dx, dy);

    if (len > 0.001) {
        double moveStep = std::min(len, speed * dt);
        m_x += (dx / len) * moveStep;
        m_y += (dy / len) * moveStep;

        if (std::abs(dx) > std::abs(dy)) {
            m_facing = (dx > 0) ? Direction::Right : Direction::Left;
        } else {
            m_facing = (dy > 0) ? Direction::Down : Direction::Up;
        }
    }
}
