#pragma once

#include <QPainter>
#include "engine/GameEngine.h"

class HorrorOverlay {
public:
    HorrorOverlay();

    static void renderSplashScreen(QPainter &painter, const GameEngine &engine, double pulseTimer, int width, int height);
    static void renderHUD(QPainter &painter, const GameEngine &engine, int width, int height);
    static void renderStateOverlay(QPainter &painter, const GameEngine &engine, int width, int height);
};
