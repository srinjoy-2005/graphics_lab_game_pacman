#pragma once

#include <QPointF>
#include <QRect>
#include "core/Constants.h"

class Camera {
public:
    Camera();

    void setTarget(double worldX, double worldY);
    void update(double dt);
    void snapToTarget();

    QPointF worldToScreen(double worldX, double worldY) const;
    void setViewportSize(int width, int height);

    int viewportWidth() const { return m_viewportWidth; }
    int viewportHeight() const { return m_viewportHeight; }
    double x() const { return m_x; }
    double y() const { return m_y; }

private:
    double m_x = 0.0;
    double m_y = 0.0;
    double m_targetX = 0.0;
    double m_targetY = 0.0;
    int m_viewportWidth = Constants::VIEW_WIDTH;
    int m_viewportHeight = Constants::VIEW_HEIGHT;
};
