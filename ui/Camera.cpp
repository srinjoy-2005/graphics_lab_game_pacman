#include "ui/Camera.h"
#include <algorithm>
#include <cmath>

Camera::Camera() {}

void Camera::setTarget(double worldX, double worldY) {
    m_targetX = worldX;
    m_targetY = worldY;
}

void Camera::update(double dt) {
    // Smooth camera lag (Lerp)
    double speed = 10.0;
    m_x += (m_targetX - m_x) * std::min(1.0, speed * dt);
    m_y += (m_targetY - m_y) * std::min(1.0, speed * dt);
}

void Camera::snapToTarget() {
    m_x = m_targetX;
    m_y = m_targetY;
}

void Camera::setViewportSize(int width, int height) {
    m_viewportWidth = width;
    m_viewportHeight = height;
}

QPointF Camera::worldToScreen(double worldX, double worldY) const {
    double screenCenterX = m_viewportWidth / 2.0;
    double screenCenterY = Constants::HUD_HEIGHT + (m_viewportHeight - Constants::HUD_HEIGHT) / 2.0;

    double screenX = screenCenterX + (worldX - m_x) * Constants::TILE_SIZE;
    double screenY = screenCenterY + (worldY - m_y) * Constants::TILE_SIZE;

    return QPointF(screenX, screenY);
}
