#pragma once

#include "entities/Entity.h"
#include "maze/MazeGrid.h"

class Player : public Entity {
public:
    Player(uint32_t id = 1, const QString &name = "Survivor");

    void reset(const QPoint &startTile);
    void update(double dt, const PlayerInput &input, const MazeGrid &grid);

    const QString &name() const { return m_name; }
    double stamina() const { return m_stamina; }
    bool isSprinting() const { return m_isSprinting; }
    bool isMoving() const { return m_isMoving; }
    bool flashlightOn() const { return m_flashlightOn; }
    void toggleFlashlight() { m_flashlightOn = !m_flashlightOn; }
    double noiseRadius() const;

private:
    void moveInDirection(Direction dir, double distance, const MazeGrid &grid);
    bool canMoveTo(double nx, double ny, const MazeGrid &grid) const;

    QString m_name;
    double m_stamina = 1.0; // 0.0 to 1.0
    bool m_isSprinting = false;
    bool m_isMoving = false;
    bool m_flashlightOn = true;
};
