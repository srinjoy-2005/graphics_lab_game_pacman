#include "ui/HorrorRenderer.h"
#include "core/Constants.h"
#include <QPainterPath>
#include <cmath>

HorrorRenderer::HorrorRenderer() {}

void HorrorRenderer::render(QPainter &painter, const GameEngine &engine, const Camera &camera, int width, int height) {
    m_flickerTimer += 0.05;
    m_pulseTimer += 0.08;

    // 1. Dark Void background
    painter.fillRect(0, 0, width, height, Constants::COLOR_DARK_VOID);

    // 2. Render Maze Tiles
    renderTiles(painter, engine, camera);

    // 3. Render Entities (Player & Ghosts)
    renderEntities(painter, engine, camera);

    // 4. Render Square Vision Shroud (Mask outside of square)
    renderSquareVisionShroud(painter, engine.localPlayer(), camera, width, height);

    // 5. Render HUD and Overlays
    renderHUD(painter, engine, width, height);
    renderOverlay(painter, engine.world().gameState(), width, height);
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

    // 2. Square Vision Window Border (Must explicitly use Qt::NoBrush!)
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

    // Survivor body (hazmat suit)
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

void HorrorRenderer::renderHUD(QPainter &painter, const GameEngine &engine, int width, int /*height*/) {
    painter.fillRect(0, 0, width, Constants::HUD_HEIGHT, QColor(14, 14, 18, 230));
    painter.setPen(QColor(40, 40, 50));
    painter.drawLine(0, Constants::HUD_HEIGHT, width, Constants::HUD_HEIGHT);

    const Player *p = engine.localPlayer();
    if (!p) return;

    QFont font = painter.font();
    font.setFamily("Consolas");
    font.setBold(true);
    painter.setFont(font);

    // Level info
    painter.setPen(QColor(180, 180, 180));
    painter.drawText(16, 20, QString("LEVEL %1 : THE LABYRINTH").arg(engine.world().level()));
    painter.setPen(QColor(120, 120, 120));
    painter.drawText(16, 38, "FIND THE UNMARKED EXIT PORTAL");

    // Stamina Bar
    int barX = 320, barY = 16, barW = 120, barH = 14;
    painter.setPen(QColor(60, 60, 60));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(barX, barY, barW, barH);
    painter.fillRect(barX + 2, barY + 2, static_cast<int>((barW - 4) * p->stamina()), barH - 4,
                     p->stamina() > 0.25 ? QColor(220, 190, 40) : QColor(220, 50, 50));
    painter.setPen(Qt::white);
    painter.drawText(barX, barY + barH + 12, "STAMINA [SHIFT]");

    // Heartbeat Monitor
    double ghostDist = engine.closestGhostDistance();
    int bpm = static_cast<int>(70 + qMax(0.0, (12.0 - ghostDist) * 10.0));
    QColor heartColor = (bpm > 110) ? Constants::COLOR_BLOOD_RED : QColor(100, 200, 120);
    painter.setPen(heartColor);
    painter.drawText(500, 28, QString("PULSE: %1 BPM").arg(bpm));

    // Status Alert
    GameState state = engine.world().gameState();
    if (state == GameState::Spotted) {
        painter.setPen(Constants::COLOR_BLOOD_RED);
        painter.drawText(650, 28, "!! ENTITY SPOTTED !!");
    } else {
        painter.setPen(QColor(140, 140, 140));
        painter.drawText(650, 28, "[R] NEW MAZE");
    }
}

void HorrorRenderer::renderOverlay(QPainter &painter, GameState state, int width, int height) {
    if (state == GameState::Caught) {
        painter.fillRect(0, 0, width, height, QColor(140, 10, 10, 200));
        painter.setPen(Qt::white);
        QFont f = painter.font();
        f.setPointSize(28);
        f.setBold(true);
        painter.setFont(f);
        painter.drawText(QRect(0, height / 2 - 60, width, 60), Qt::AlignCenter, "YOU WERE CONSUMED");
        f.setPointSize(14);
        painter.setFont(f);
        painter.drawText(QRect(0, height / 2 + 10, width, 40), Qt::AlignCenter, "Press [SPACE] or [R] to Try Again");
    } else if (state == GameState::Escaped) {
        painter.fillRect(0, 0, width, height, QColor(20, 80, 40, 200));
        painter.setPen(Qt::white);
        QFont f = painter.font();
        f.setPointSize(28);
        f.setBold(true);
        painter.setFont(f);
        painter.drawText(QRect(0, height / 2 - 60, width, 60), Qt::AlignCenter, "YOU ESCAPED THE LEVEL");
        f.setPointSize(14);
        painter.setFont(f);
        painter.drawText(QRect(0, height / 2 + 10, width, 40), Qt::AlignCenter, "Press [SPACE] for Next Level");
    }
}
