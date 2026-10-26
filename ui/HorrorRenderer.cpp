#include "ui/HorrorRenderer.h"
#include "ui/HorrorOverlay.h"
#include "core/Constants.h"
#include <QPainterPath>
#include <cmath>

HorrorRenderer::HorrorRenderer() {}

void HorrorRenderer::render(QPainter &painter, const GameEngine &engine, const Camera &camera, int width, int height) {
    m_flickerTimer += 0.05;
    m_pulseTimer += 0.08;

    // 1. Dark Void background
    painter.fillRect(0, 0, width, height, Constants::COLOR_DARK_VOID);

    // If on splash screen, render rules and title
    if (engine.world().gameState() == GameState::Splash) {
        HorrorOverlay::renderSplashScreen(painter, engine, m_pulseTimer, width, height);
        return;
    }

    // 2. Render Maze Tiles
    renderTiles(painter, engine, camera);

    // 3. Render Entities (Player & Ghosts)
    renderEntities(painter, engine, camera);

    // 4. Render Square Vision Shroud (Mask outside of square)
    renderSquareVisionShroud(painter, engine.localPlayer(), camera, width, height);

    // 5. Render HUD and State Overlays
    HorrorOverlay::renderHUD(painter, engine, width, height);
    HorrorOverlay::renderStateOverlay(painter, engine, width, height);
}

void HorrorRenderer::renderTiles(QPainter &painter, const GameEngine &engine, const Camera &camera) {
    const MazeGrid &grid = engine.world().grid();
    int ts = Constants::TILE_SIZE;

    for (int y = 0; y < grid.rows(); ++y) {
        for (int x = 0; x < grid.cols(); ++x) {
            const MazeCell &c = grid.cell(x, y);
            if (!c.visitedByPlayer) continue;

            QPointF p = camera.worldToScreen(x, y);
            QRectF rect(p.x() - ts / 2.0, p.y() - ts / 2.0, ts, ts);

            float light = c.inSquareVision ? c.lightIntensity : 0.08f;

            if (c.type == TileType::Wall) {
                QColor wallColor = Constants::COLOR_WALL;
                wallColor.setAlphaF(light);
                painter.fillRect(rect, wallColor);

                // Wall 3D bevel / border
                QColor trimColor = Constants::COLOR_WALL_ACCENT;
                trimColor.setAlphaF(light);
                painter.setPen(QPen(trimColor, 1));
                painter.setBrush(Qt::NoBrush);
                painter.drawRect(rect);
                painter.drawLine(rect.topLeft(), rect.topRight());
            } else if (c.type == TileType::Floor) {
                QColor floorColor = Constants::COLOR_FLOOR;
                floorColor.setAlphaF(light);
                painter.fillRect(rect, floorColor);

                // Subtle floor seams
                QColor seamColor = Constants::COLOR_FLOOR_GRID;
                seamColor.setAlphaF(light * 0.6f);
                painter.setPen(seamColor);
                painter.setBrush(Qt::NoBrush);
                painter.drawRect(rect);
            } else if (c.type == TileType::Checkpoint) {
                QColor floorColor = Constants::COLOR_FLOOR;
                floorColor.setAlphaF(light);
                painter.fillRect(rect, floorColor);

                if (c.inSquareVision) {
                    bool act = c.checkpointActivated;
                    double pulse = 0.6 + 0.4 * std::sin(m_pulseTimer * 3.5);
                    QColor cpColor = act ? Constants::COLOR_CHECKPOINT_ACTIVE : Constants::COLOR_CHECKPOINT_INACTIVE;
                    cpColor.setAlphaF(static_cast<float>(pulse));

                    painter.setBrush(cpColor);
                    painter.setPen(QPen(Qt::white, 1));
                    painter.drawEllipse(rect.center(), ts * 0.38, ts * 0.38);

                    painter.setPen(act ? Qt::black : Qt::white);
                    painter.setBrush(Qt::NoBrush);
                    QFont font = painter.font();
                    font.setPointSize(7);
                    font.setBold(true);
                    painter.setFont(font);
                    painter.drawText(rect, Qt::AlignCenter, act ? "SAFE" : "SAVE");
                }
            } else if (c.type == TileType::ExitDoor) {
                if (c.inSquareVision) {
                    double pulse = 0.7 + 0.3 * std::sin(m_pulseTimer * 4.0);
                    QColor exitColor = Constants::COLOR_EXIT;
                    exitColor.setAlphaF(static_cast<float>(pulse));
                    painter.fillRect(rect, exitColor);

                    painter.setPen(Qt::white);
                    painter.setBrush(Qt::NoBrush);
                    QFont font = painter.font();
                    font.setPointSize(8);
                    font.setBold(true);
                    painter.setFont(font);
                    painter.drawText(rect, Qt::AlignCenter, "EXIT");
                }
            }
        }
    }
}

void HorrorRenderer::renderSquareVisionShroud(QPainter &painter, const Player *player, const Camera &camera, int width, int height) {
    if (!player) return;

    QPointF playerScreen = camera.worldToScreen(player->x(), player->y());
    int visionPx = Constants::VISION_SQUARE_TILES * Constants::TILE_SIZE;
    QRectF visionRect(playerScreen.x() - visionPx / 2.0, playerScreen.y() - visionPx / 2.0, visionPx, visionPx);

    // 1. Shroud outside square vision window
    QPainterPath fullScreen;
    fullScreen.addRect(0, 0, width, height);

    QPainterPath visionBox;
    visionBox.addRect(visionRect);

    QPainterPath shroud = fullScreen.subtracted(visionBox);
    painter.fillPath(shroud, Constants::COLOR_DARK_VOID);

    // 2. Square Vision Window Border
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(QColor(90, 80, 55, 200), 2));
    painter.drawRect(visionRect);

    // 3. Subtle ambient flashlight glow inside square
    if (player->flashlightOn()) {
        float flicker = 0.85f + 0.15f * static_cast<float>(std::sin(m_flickerTimer * 12.0) * std::cos(m_flickerTimer * 7.0));
        QColor flashColor = Constants::COLOR_FLASHLIGHT;
        flashColor.setAlphaF(0.12f * flicker);
        painter.fillRect(visionRect, flashColor);
    }
}

void HorrorRenderer::renderEntities(QPainter &painter, const GameEngine &engine, const Camera &camera) {
    const Player *player = engine.localPlayer();
    int visionHalf = Constants::VISION_SQUARE_TILES / 2;

    // Render Ghosts
    for (const Ghost &g : engine.world().ghosts()) {
        bool inVision = false;
        if (player) {
            double dx = std::abs(g.x() - player->x());
            double dy = std::abs(g.y() - player->y());
            inVision = (dx <= visionHalf && dy <= visionHalf);
        }
        renderGhost(painter, g, camera, inVision);
    }

    // Render Player
    if (player) {
        renderPlayer(painter, *player, camera);
    }
}

void HorrorRenderer::renderPlayer(QPainter &painter, const Player &player, const Camera &camera) {
    QPointF p = camera.worldToScreen(player.x(), player.y());
    double r = Constants::TILE_SIZE * 0.38;

    // Flashlight cone
    if (player.flashlightOn()) {
        double beamLen = Constants::TILE_SIZE * 1.8;
        double spread = Constants::TILE_SIZE * 0.7;
        QPainterPath cone;
        cone.moveTo(p);
        switch (player.facing()) {
            case Direction::Up:
                cone.lineTo(p.x() - spread, p.y() - beamLen);
                cone.lineTo(p.x() + spread, p.y() - beamLen);
                break;
            case Direction::Down:
                cone.lineTo(p.x() - spread, p.y() + beamLen);
                cone.lineTo(p.x() + spread, p.y() + beamLen);
                break;
            case Direction::Left:
                cone.lineTo(p.x() - beamLen, p.y() - spread);
                cone.lineTo(p.x() - beamLen, p.y() + spread);
                break;
            case Direction::Right:
                cone.lineTo(p.x() + beamLen, p.y() - spread);
                cone.lineTo(p.x() + beamLen, p.y() + spread);
                break;
            default: break;
        }
        cone.closeSubpath();
        painter.fillPath(cone, QColor(255, 245, 200, 35));
    }

    // Survivor body
    painter.setBrush(QColor(245, 215, 30));
    painter.setPen(QPen(QColor(30, 30, 20), 2));
    painter.drawEllipse(p, r, r);

    // Visor
    painter.setBrush(QColor(30, 35, 45));
    painter.setPen(Qt::NoPen);
    double vx = p.x(), vy = p.y();
    switch (player.facing()) {
        case Direction::Up:    vy -= r * 0.4; break;
        case Direction::Down:  vy += r * 0.4; break;
        case Direction::Left:  vx -= r * 0.4; break;
        case Direction::Right: vx += r * 0.4; break;
        default: break;
    }
    painter.drawEllipse(QPointF(vx, vy), r * 0.45, r * 0.35);
}

void HorrorRenderer::renderGhost(QPainter &painter, const Ghost &ghost, const Camera &camera, bool inVision) {
    QPointF p = camera.worldToScreen(ghost.x(), ghost.y());
    double r = Constants::TILE_SIZE * 0.42;

    if (inVision) {
        painter.setBrush(ghost.bodyColor());
        painter.setPen(QPen(ghost.isChasing() ? QColor(220, 20, 20) : QColor(60, 60, 60), 1.5));
        painter.drawEllipse(p, r, r);
    }

    // Piercing glowing eyes
    if (inVision || ghost.isChasing()) {
        QColor eye = ghost.eyeColor();
        eye.setAlphaF(static_cast<float>(ghost.eyeGlowIntensity()));
        painter.setBrush(eye);
        painter.setPen(Qt::NoPen);

        double eyeOffset = r * 0.38;
        painter.drawEllipse(QPointF(p.x() - eyeOffset, p.y() - eyeOffset * 0.5), 2.5, 2.5);
        painter.drawEllipse(QPointF(p.x() + eyeOffset, p.y() - eyeOffset * 0.5), 2.5, 2.5);

        if (ghost.type() == GhostType::Smiler && inVision) {
            painter.setPen(QPen(eye, 1.5));
            painter.setBrush(Qt::NoBrush);
            painter.drawArc(QRectF(p.x() - r * 0.5, p.y() - r * 0.2, r, r * 0.6), 200 * 16, 140 * 16);
        }
    }
}
