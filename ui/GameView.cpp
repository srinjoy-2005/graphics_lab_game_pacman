#include "ui/GameView.h"
#include <QKeyEvent>
#include <QPainter>
#include "core/Constants.h"

GameView::GameView(QWidget *parent) : QWidget(parent) {
    setFixedSize(Constants::VIEW_WIDTH, Constants::VIEW_HEIGHT);
    setFocusPolicy(Qt::StrongFocus);
    setFocus();
    setStyleSheet("background-color: #050508;");

    m_camera.setViewportSize(width(), height());
    const Player *p = m_engine.localPlayer();
    if (p) {
        m_camera.setTarget(p->x(), p->y());
        m_camera.snapToTarget();
    }

    // Physics simulation timer (40 Hz = 25 ms)
    m_physicsTimer = new QTimer(this);
    connect(m_physicsTimer, &QTimer::timeout, this, &GameView::onPhysicsTick);
    m_physicsTimer->start(Constants::TICK_RATE_MS);

    // Render timer (~60 FPS = 16 ms)
    m_renderTimer = new QTimer(this);
    connect(m_renderTimer, &QTimer::timeout, this, [this]() {
        update();
    });
    m_renderTimer->start(Constants::RENDER_RATE_MS);
}

void GameView::resizeEvent(QResizeEvent * /*e*/) {
    m_camera.setViewportSize(width(), height());
}

PlayerInput GameView::buildInput() {
    PlayerInput input;

    if (m_heldKeys.contains(Qt::Key_W) || m_heldKeys.contains(Qt::Key_Up)) {
        input.moveDir = Direction::Up;
    } else if (m_heldKeys.contains(Qt::Key_S) || m_heldKeys.contains(Qt::Key_Down)) {
        input.moveDir = Direction::Down;
    } else if (m_heldKeys.contains(Qt::Key_A) || m_heldKeys.contains(Qt::Key_Left)) {
        input.moveDir = Direction::Left;
    } else if (m_heldKeys.contains(Qt::Key_D) || m_heldKeys.contains(Qt::Key_Right)) {
        input.moveDir = Direction::Right;
    }

    input.sprint = m_heldKeys.contains(Qt::Key_Shift);
    return input;
}

void GameView::onPhysicsTick() {
    PlayerInput input = buildInput();
    double dt = Constants::TICK_RATE_MS / 1000.0;
    m_engine.update(dt, input);

    const Player *p = m_engine.localPlayer();
    if (p) {
        m_camera.setTarget(p->x(), p->y());
    }
    m_camera.update(dt);
}

void GameView::keyPressEvent(QKeyEvent *e) {
    if (e->isAutoRepeat()) return;

    int key = e->key();
    m_heldKeys.insert(key);

    if (key == Qt::Key_R) {
        m_engine.startNewGame(true);
        const Player *p = m_engine.localPlayer();
        if (p) {
            m_camera.setTarget(p->x(), p->y());
            m_camera.snapToTarget();
        }
    } else if (key == Qt::Key_Space) {
        GameState state = m_engine.world().gameState();
        if (state == GameState::Caught) {
            m_engine.restartLevel();
        } else if (state == GameState::Escaped) {
            m_engine.nextLevel();
        }
    } else if (key == Qt::Key_F) {
        Player *p = m_engine.world().player(m_engine.localPlayerId());
        if (p) p->toggleFlashlight();
    }

    QWidget::keyPressEvent(e);
}

void GameView::keyReleaseEvent(QKeyEvent *e) {
    if (e->isAutoRepeat()) return;
    m_heldKeys.remove(e->key());
    QWidget::keyReleaseEvent(e);
}

void GameView::mousePressEvent(QMouseEvent *e) {
    setFocus();
    QWidget::mousePressEvent(e);
}

void GameView::paintEvent(QPaintEvent * /*e*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    m_renderer.render(painter, m_engine, m_camera, width(), height());
}
