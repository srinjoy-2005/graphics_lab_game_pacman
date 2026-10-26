#include "entities/Player.h"
#include "core/Constants.h"
#include <algorithm>
#include <cmath>

Player::Player(uint32_t id, const QString &name)
    : Entity(id), m_name(name) {}

void Player::reset(const QPoint &startTile) {
    m_x = startTile.x();
    m_y = startTile.y();
    m_facing = Direction::Down;
    m_stamina = 1.0;
    m_isSprinting = false;
    m_isMoving = false;
    m_flashlightOn = true;
}

double Player::noiseRadius() const {
    if (!m_isMoving) return 0.0;
    return m_isSprinting ? Constants::SPRINT_NOISE_RADIUS : Constants::WALK_NOISE_RADIUS;
}

void Player::update(double dt, const PlayerInput &input, const MazeGrid &grid) {
    if (input.toggleFlashlight) {
        toggleFlashlight();
    }

    // Stamina & Sprint logic
    if (input.sprint && m_stamina > 0.05 && input.moveDir != Direction::None) {
        m_isSprinting = true;
        m_stamina = std::max(0.0, m_stamina - dt * 0.35); // Depletes in ~3 seconds
    } else {
        m_isSprinting = false;
        m_stamina = std::min(1.0, m_stamina + dt * 0.20); // Recovers in 5 seconds
    }

    double speed = m_isSprinting ? Constants::PLAYER_SPRINT_SPEED : Constants::PLAYER_WALK_SPEED;
    double dist = speed * dt;

    m_isMoving = (input.moveDir != Direction::None);
    if (m_isMoving) {
        m_facing = input.moveDir;
        moveInDirection(input.moveDir, dist, grid);
    }
}

void Player::moveInDirection(Direction dir, double dist, const MazeGrid &grid) {
    double dx = 0.0, dy = 0.0;
    switch (dir) {
        case Direction::Up:    dy = -dist; break;
        case Direction::Down:  dy =  dist; break;
        case Direction::Left:  dx = -dist; break;
        case Direction::Right: dx =  dist; break;
        default: break;
    }

    // Slide on walls: try both axes independently
    double nextX = m_x + dx;
    if (canMoveTo(nextX, m_y, grid)) {
        m_x = nextX;
    }

    double nextY = m_y + dy;
    if (canMoveTo(m_x, nextY, grid)) {
        m_y = nextY;
    }
}

bool Player::canMoveTo(double nx, double ny, const MazeGrid &grid) const {
    // Collision check using player bounding radius (0.30 of a tile)
    const double radius = 0.30;
    int minTileX = static_cast<int>(std::floor(nx - radius + 0.5));
    int maxTileX = static_cast<int>(std::floor(nx + radius + 0.5));
    int minTileY = static_cast<int>(std::floor(ny - radius + 0.5));
    int maxTileY = static_cast<int>(std::floor(ny + radius + 0.5));

    for (int ty = minTileY; ty <= maxTileY; ++ty) {
        for (int tx = minTileX; tx <= maxTileX; ++tx) {
            if (!grid.isWalkable(tx, ty)) {
                return false;
            }
        }
    }
    return true;
}
