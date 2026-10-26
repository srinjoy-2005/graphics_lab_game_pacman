#include "ui/HorrorOverlay.h"
#include "core/Constants.h"
#include <cmath>

HorrorOverlay::HorrorOverlay() {}

void HorrorOverlay::renderHUD(QPainter &painter, const GameEngine &engine, int width, int /*height*/) {
    painter.fillRect(0, 0, width, Constants::HUD_HEIGHT, QColor(14, 14, 18, 230));
    painter.setPen(QColor(40, 40, 50));
    painter.drawLine(0, Constants::HUD_HEIGHT, width, Constants::HUD_HEIGHT);

    const Player *p = engine.localPlayer();
    if (!p) return;

    QFont font = painter.font();
    font.setFamily("Consolas");
    font.setBold(true);
    painter.setFont(font);

    // Level info & Difficulty Tag
    painter.setPen(QColor(180, 180, 180));
    painter.drawText(16, 20, QString("LEVEL %1 / 3 : THE LABYRINTH").arg(engine.world().level()));

    Difficulty diff = engine.world().difficulty();
    QString diffStr = (diff == Difficulty::Easy) ? "EASY" : (diff == Difficulty::Normal ? "NORMAL" : "HARD");
    QColor diffCol = (diff == Difficulty::Easy) ? QColor(80, 220, 100) : (diff == Difficulty::Normal ? QColor(240, 200, 40) : QColor(240, 50, 50));
    painter.setPen(diffCol);
    painter.drawText(200, 20, QString("[%1]").arg(diffStr));

    // Lives Indicator
    int lives = engine.world().lives();
    QString livesText = "LIVES: ";
    for (int i = 0; i < 3; ++i) {
        livesText += (i < lives) ? "[●] " : "[ ] ";
    }
    painter.setPen(lives > 1 ? QColor(240, 210, 40) : QColor(230, 40, 40));
    painter.drawText(16, 38, livesText);

    // Stamina Bar
    int barX = 265, barY = 16, barW = 105, barH = 14;
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
    painter.drawText(420, 28, QString("PULSE: %1 BPM").arg(bpm));

    // Status Alert & Controls
    GameState state = engine.world().gameState();
    if (state == GameState::Spotted) {
        painter.setPen(Constants::COLOR_BLOOD_RED);
        painter.drawText(580, 28, "!! ENTITY SPOTTED !!");
    } else {
        painter.setPen(QColor(140, 140, 140));
        painter.drawText(580, 28, "[R] RESTART | [ESC] MENU");
    }
}

void HorrorOverlay::renderSplashScreen(QPainter &painter, const GameEngine &engine, double pulseTimer, int width, int height) {
    painter.fillRect(0, 0, width, height, QColor(8, 8, 12));

    QFont font = painter.font();
    font.setFamily("Consolas");

    // Title
    font.setPointSize(22);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(QColor(245, 215, 60));
    painter.drawText(QRect(0, 35, width, 35), Qt::AlignCenter, "PACMAN MAZE: SURVIVAL IN THE DARK");

    font.setPointSize(11);
    font.setBold(false);
    painter.setFont(font);
    painter.setPen(QColor(150, 150, 160));
    painter.drawText(QRect(0, 72, width, 22), Qt::AlignCenter, "-- 3-LEVEL PROCEDURAL ESCAPE PROTOCOL --");

    // Rules Box
    int boxW = 710, boxH = 460;
    int boxX = (width - boxW) / 2;
    int boxY = 100;
    QRect box(boxX, boxY, boxW, boxH);

    painter.setBrush(QColor(14, 15, 20, 225));
    painter.setPen(QPen(QColor(70, 65, 45), 2));
    painter.drawRect(box);

    int textX = boxX + 24;
    int curY = boxY + 24;

    auto drawRule = [&](const QString &tag, const QString &desc, const QColor &tagCol) {
        font.setPointSize(10);
        font.setBold(true);
        painter.setFont(font);
        painter.setPen(tagCol);
        painter.drawText(textX, curY, tag);

        font.setPointSize(9);
        font.setBold(false);
        painter.setFont(font);
        painter.setPen(QColor(215, 215, 215));
        painter.drawText(textX + 130, curY, desc);
        curY += 25;
    };

    drawRule("1. OBJECTIVE", "Find the unmarked green EXIT portal hidden in each maze.", QColor(50, 220, 120));
    drawRule("2. 3 LEVELS", "3 unique dynamic mazes generated for this session. Escape all 3 to win.", QColor(220, 190, 40));
    drawRule("3. 4-TILE FOG", "Entities are completely BLIND past ~4 tiles! Use distance & darkness to hide.", QColor(100, 210, 255));
    drawRule("4. 3 LIVES", "You have 3 lives for the entire session. Die thrice = Session Over.", QColor(240, 70, 70));
    drawRule("5. CHECKPOINTS", "Step onto glowing blue beacons to save a safe respawn point in the level.", QColor(60, 140, 240));
    drawRule("6. STEALTH", "Walking is silent. Sprinting [SHIFT] is fast but emits sound within 3.2 tiles.", QColor(240, 160, 40));

    // Entity Threat Intel Section
    curY += 4;
    painter.setPen(QColor(50, 50, 60));
    painter.drawLine(textX, curY, boxX + boxW - 24, curY);
    curY += 18;

    font.setPointSize(10);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(QColor(255, 180, 60));
    painter.drawText(textX, curY, "ENTITY EYE & BEHAVIOR INTEL:");
    curY += 20;

    auto drawIntel = [&](const QString &eye, const QString &name, const QString &desc, const QColor &eyeCol) {
        font.setPointSize(9);
        font.setBold(true);
        painter.setFont(font);
        painter.setPen(eyeCol);
        painter.drawText(textX + 12, curY, eye);

        painter.setPen(QColor(230, 230, 230));
        painter.drawText(textX + 110, curY, name);

        font.setBold(false);
        painter.setFont(font);
        painter.setPen(QColor(170, 170, 180));
        painter.drawText(textX + 220, curY, desc);
        curY += 21;
    };

    drawIntel("WHITE + SMILE", "The Smiler", "Creepy grin & glowing eyes. Lurks in the dark.", QColor(245, 245, 255));
    drawIntel("AMBER GLOW", "The Stalker", "Cautious hunter. Navigates corridors smoothly.", QColor(255, 140, 0));
    drawIntel("YELLOW-GREEN", "The Hound", "Canine predator. Very sensitive to sprint footsteps.", QColor(180, 220, 40));
    drawIntel("BLOOD RED", "IN PURSUIT", "Entity locked sight on you! Turn corners to break LOS!", QColor(255, 30, 30));

    // Difficulty Selector Section
    curY += 6;
    painter.setPen(QColor(50, 50, 60));
    painter.drawLine(textX, curY, boxX + boxW - 24, curY);
    curY += 22;

    Difficulty currDiff = engine.world().difficulty();
    font.setPointSize(10);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(QColor(220, 220, 220));
    painter.drawText(textX, curY, "SELECT DIFFICULTY [Press TAB or 1/2/3]:");

    auto drawDiffButton = [&](int x, const QString &label, Difficulty d, const QColor &c) {
        bool active = (currDiff == d);
        QRect btnRect(x, curY - 14, 85, 22);
        if (active) {
            painter.setBrush(QColor(c.red(), c.green(), c.blue(), 60));
            painter.setPen(QPen(c, 2));
            painter.drawRect(btnRect);
            painter.setPen(c);
        } else {
            painter.setBrush(Qt::NoBrush);
            painter.setPen(QColor(80, 80, 90));
            painter.drawRect(btnRect);
            painter.setPen(QColor(130, 130, 140));
        }
        painter.drawText(btnRect, Qt::AlignCenter, label);
    };

    drawDiffButton(textX + 310, "[1] EASY", Difficulty::Easy, QColor(80, 220, 100));
    drawDiffButton(textX + 405, "[2] NORMAL", Difficulty::Normal, QColor(240, 200, 40));
    drawDiffButton(textX + 500, "[3] HARD", Difficulty::Hard, QColor(240, 50, 50));

    // Pulsing Start Prompt
    double promptPulse = 0.6 + 0.4 * std::sin(pulseTimer * 4.0);
    font.setPointSize(13);
    font.setBold(true);
    painter.setFont(font);
    QColor promptCol = QColor(255, 220, 60);
    promptCol.setAlphaF(static_cast<float>(promptPulse));
    painter.setPen(promptCol);
    painter.drawText(QRect(0, height - 90, width, 30), Qt::AlignCenter, "PRESS [SPACE] OR [ENTER] TO BEGIN SESSION");

    font.setPointSize(10);
    font.setBold(false);
    painter.setFont(font);
    painter.setPen(QColor(130, 130, 140));
    painter.drawText(QRect(0, height - 52, width, 22), Qt::AlignCenter, "[WASD] Move  |  [SHIFT] Sprint  |  [F] Flashlight  |  [TAB] Difficulty  |  [R] Re-roll Session");
}

void HorrorOverlay::renderStateOverlay(QPainter &painter, const GameEngine &engine, int width, int height) {
    GameState state = engine.world().gameState();
    QFont font = painter.font();
    font.setFamily("Consolas");

    if (state == GameState::Caught) {
        painter.fillRect(0, 0, width, height, QColor(140, 10, 10, 210));

        font.setPointSize(28);
        font.setBold(true);
        painter.setFont(font);
        painter.setPen(Qt::white);
        painter.drawText(QRect(0, height / 2 - 80, width, 55), Qt::AlignCenter, "YOU WERE CONSUMED");

        font.setPointSize(14);
        font.setBold(false);
        painter.setFont(font);
        painter.setPen(QColor(255, 220, 120));
        painter.drawText(QRect(0, height / 2 - 15, width, 30), Qt::AlignCenter,
                         QString("Lives Remaining: %1 / 3").arg(engine.world().lives()));

        font.setPointSize(13);
        painter.setFont(font);
        painter.setPen(Qt::white);
        painter.drawText(QRect(0, height / 2 + 25, width, 30), Qt::AlignCenter,
                         "Press [SPACE] to Respawn at Last Checkpoint");
        painter.setPen(QColor(200, 200, 200));
        painter.drawText(QRect(0, height / 2 + 60, width, 30), Qt::AlignCenter,
                         "Press [R] to Restart Level  |  [ESC] Exit to Menu");
    } else if (state == GameState::GameOver) {
        painter.fillRect(0, 0, width, height, QColor(30, 5, 5, 235));

        font.setPointSize(28);
        font.setBold(true);
        painter.setFont(font);
        painter.setPen(QColor(240, 40, 40));
        painter.drawText(QRect(0, height / 2 - 80, width, 55), Qt::AlignCenter, "SESSION OVER");

        font.setPointSize(14);
        font.setBold(false);
        painter.setFont(font);
        painter.setPen(QColor(220, 220, 220));
        painter.drawText(QRect(0, height / 2 - 15, width, 30), Qt::AlignCenter,
                         "All 3 lives were lost in the labyrinth.");

        font.setPointSize(13);
        painter.setFont(font);
        painter.setPen(QColor(255, 220, 60));
        painter.drawText(QRect(0, height / 2 + 25, width, 30), Qt::AlignCenter,
                         "Press [SPACE] or [R] to Start a New 3-Level Session");
        painter.setPen(QColor(180, 180, 180));
        painter.drawText(QRect(0, height / 2 + 60, width, 30), Qt::AlignCenter,
                         "Press [ESC] for Title Screen");
    } else if (state == GameState::Escaped) {
        painter.fillRect(0, 0, width, height, QColor(15, 80, 35, 210));

        font.setPointSize(28);
        font.setBold(true);
        painter.setFont(font);
        painter.setPen(Qt::white);
        painter.drawText(QRect(0, height / 2 - 70, width, 55), Qt::AlignCenter,
                         QString("LEVEL %1 CLEARED!").arg(engine.world().level()));

        font.setPointSize(14);
        font.setBold(false);
        painter.setFont(font);
        painter.setPen(QColor(255, 255, 200));
        painter.drawText(QRect(0, height / 2 - 10, width, 30), Qt::AlignCenter,
                         QString("Preparing Next Level: %1 / 3").arg(engine.world().level() + 1));

        font.setPointSize(13);
        painter.setFont(font);
        painter.setPen(Qt::white);
        painter.drawText(QRect(0, height / 2 + 35, width, 30), Qt::AlignCenter,
                         "Press [SPACE] to Enter Next Level");
    } else if (state == GameState::Victory) {
        painter.fillRect(0, 0, width, height, QColor(10, 45, 75, 230));

        font.setPointSize(28);
        font.setBold(true);
        painter.setFont(font);
        painter.setPen(QColor(255, 225, 60));
        painter.drawText(QRect(0, height / 2 - 80, width, 55), Qt::AlignCenter, "YOU CONQUERED THE LABYRINTH!");

        font.setPointSize(14);
        font.setBold(false);
        painter.setFont(font);
        painter.setPen(Qt::white);
        painter.drawText(QRect(0, height / 2 - 15, width, 30), Qt::AlignCenter,
                         "All 3 dynamic levels successfully escaped.");

        font.setPointSize(13);
        painter.setFont(font);
        painter.setPen(QColor(100, 220, 255));
        painter.drawText(QRect(0, height / 2 + 25, width, 30), Qt::AlignCenter,
                         "Press [SPACE] or [R] to Start a New Session");
        painter.setPen(QColor(180, 180, 180));
        painter.drawText(QRect(0, height / 2 + 60, width, 30), Qt::AlignCenter,
                         "Press [ESC] for Title Screen");
    }
}
