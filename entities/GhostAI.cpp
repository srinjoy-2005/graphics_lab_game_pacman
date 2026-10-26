#include "entities/GhostAI.h"
#include "entities/Player.h"
#include "core/Constants.h"
#include "core/SoundManager.h"
#include <QRandomGenerator>
#include <cmath>

GhostAI::GhostAI(GhostType type) : m_type(type) {
    reset();
}

void GhostAI::reset() {
    m_state = GhostBehaviorState::Patrol;
    m_stateTimer = 0.0;
    m_patrolDir = Direction::None;
    m_patrolTarget = QPoint(-1, -1);
    m_lastKnownPlayerPos = QPoint(-1, -1);
}

void GhostAI::update(double dt,
                     const QPoint &currentTile,
                     const Player &player,
                     const MazeGrid &grid,
                     QPoint &outTargetTile,
                     double &outSpeed) {
    m_stateTimer += dt;
    updateSensory(currentTile, player, grid);

    switch (m_state) {
        case GhostBehaviorState::Chase: {
            outSpeed = (m_type == GhostType::Stalker) ? Constants::GHOST_STALKER_SPEED : Constants::GHOST_CHASE_SPEED;
            outTargetTile = player.tilePos();
            break;
        }

        case GhostBehaviorState::Investigate: {
            outSpeed = Constants::GHOST_PATROL_SPEED * 1.25;
            outTargetTile = m_lastKnownPlayerPos;

            // If reached the investigation point, search around then resume patrol
            if (currentTile == m_lastKnownPlayerPos || m_stateTimer > 6.0) {
                m_state = GhostBehaviorState::Cooldown;
                m_stateTimer = 0.0;
            }
            break;
        }

        case GhostBehaviorState::Cooldown: {
            outSpeed = Constants::GHOST_PATROL_SPEED * 0.5;
            outTargetTile = pickPatrolTarget(currentTile, grid);
            if (m_stateTimer > 2.5) {
                m_state = GhostBehaviorState::Patrol;
                m_stateTimer = 0.0;
            }
            break;
        }

        case GhostBehaviorState::Patrol:
        default: {
            outSpeed = Constants::GHOST_PATROL_SPEED;
            outTargetTile = pickPatrolTarget(currentTile, grid);
            break;
        }
    }
}

void GhostAI::updateSensory(const QPoint &currentTile, const Player &player, const MazeGrid &grid) {
    QPoint playerTile = player.tilePos();
    double dx = player.x() - currentTile.x();
    double dy = player.y() - currentTile.y();
    double dist = std::sqrt(dx * dx + dy * dy);

    // 1. Line-of-sight visual check
    double maxLos = (m_type == GhostType::Smiler) ? Constants::MAX_LINE_OF_SIGHT_TILES * 1.3 : Constants::MAX_LINE_OF_SIGHT_TILES;
    bool hasVisual = grid.hasLineOfSight(currentTile, playerTile, maxLos);

    if (hasVisual) {
        if (m_state != GhostBehaviorState::Chase) {
            SoundManager::instance().playScreech();
        }
        m_state = GhostBehaviorState::Chase;
        m_lastKnownPlayerPos = playerTile;
        m_stateTimer = 0.0;
        return;
    }

    // If was chasing and lost visual, transition to investigate last known point
    if (m_state == GhostBehaviorState::Chase) {
        m_state = GhostBehaviorState::Investigate;
        m_stateTimer = 0.0;
        return;
    }

    // 2. Sound hearing check (Hound is especially sensitive)
    double hearingMult = (m_type == GhostType::Hound) ? 1.6 : 1.0;
    double effectiveNoise = player.noiseRadius() * hearingMult;

    if (effectiveNoise > 0.1 && dist <= effectiveNoise) {
        if (m_state == GhostBehaviorState::Patrol) {
            m_state = GhostBehaviorState::Investigate;
            m_lastKnownPlayerPos = playerTile;
            m_stateTimer = 0.0;
        }
    }
}

QPoint GhostAI::pickPatrolTarget(const QPoint &currentTile, const MazeGrid &grid) {
    if (m_patrolTarget != QPoint(-1, -1) && currentTile != m_patrolTarget) {
        return m_patrolTarget;
    }

    // Pick valid neighbor tile without immediate 180 backtrack if possible
    struct Step { int dx; int dy; Direction dir; };
    const Step steps[] = {
        {0, -1, Direction::Up},
        {0,  1, Direction::Down},
        {-1, 0, Direction::Left},
        {1,  0, Direction::Right}
    };

    QVector<QPoint> options;
    Direction opposite = Direction::None;
    if (m_patrolDir == Direction::Up) opposite = Direction::Down;
    else if (m_patrolDir == Direction::Down) opposite = Direction::Up;
    else if (m_patrolDir == Direction::Left) opposite = Direction::Right;
    else if (m_patrolDir == Direction::Right) opposite = Direction::Left;

    for (int i = 0; i < 4; ++i) {
        int nx = currentTile.x() + steps[i].dx;
        int ny = currentTile.y() + steps[i].dy;
        if (grid.isWalkable(nx, ny) && steps[i].dir != opposite) {
            options.append(QPoint(nx, ny));
        }
    }

    if (options.isEmpty()) {
        // Dead end: allow turning back
        for (int i = 0; i < 4; ++i) {
            int nx = currentTile.x() + steps[i].dx;
            int ny = currentTile.y() + steps[i].dy;
            if (grid.isWalkable(nx, ny)) {
                options.append(QPoint(nx, ny));
            }
        }
    }

    if (!options.isEmpty()) {
        int idx = QRandomGenerator::global()->bounded(options.size());
        m_patrolTarget = options[idx];
        return m_patrolTarget;
    }

    return currentTile;
}
