#pragma once

#include <QPoint>
#include "core/Types.h"

class Entity {
public:
    Entity(uint32_t id = 0) : m_id(id) {}
    virtual ~Entity() = default;

    uint32_t id() const { return m_id; }
    void setId(uint32_t id) { m_id = id; }

    double x() const { return m_x; }
    double y() const { return m_y; }
    void setPosition(double x, double y) {
        m_x = x;
        m_y = y;
    }

    QPoint tilePos() const {
        return QPoint(static_cast<int>(std::round(m_x)),
                      static_cast<int>(std::round(m_y)));
    }

    Direction facing() const { return m_facing; }
    void setFacing(Direction dir) { m_facing = dir; }

protected:
    uint32_t m_id = 0;
    double m_x = 0.0;
    double m_y = 0.0;
    Direction m_facing = Direction::Down;
};
