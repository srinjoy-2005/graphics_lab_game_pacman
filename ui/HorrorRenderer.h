#pragma once

#include <QPainter>
#include <QImage>
#include "engine/GameEngine.h"
#include "ui/Camera.h"

class HorrorRenderer {
public:
    HorrorRenderer();

    void render(QPainter &painter, const GameEngine &engine, const Camera &camera, int width, int height);

private:
    void renderTiles(QPainter &painter, const GameEngine &engine, const Camera &camera);
    void renderSquareVisionShroud(QPainter &painter, const Player *player, const Camera &camera, int width, int height);
    void renderEntities(QPainter &painter, const GameEngine &engine, const Camera &camera);
    void renderPlayer(QPainter &painter, const Player &player, const Camera &camera);
    void renderGhost(QPainter &painter, const Ghost &ghost, const Camera &camera, bool inVision);

    double m_flickerTimer = 0.0;
    double m_pulseTimer = 0.0;
};
