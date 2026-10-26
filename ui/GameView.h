#pragma once

#include <QWidget>
#include <QTimer>
#include <QSet>
#include "engine/GameEngine.h"
#include "ui/Camera.h"
#include "ui/HorrorRenderer.h"

class GameView : public QWidget {
    Q_OBJECT
public:
    explicit GameView(QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void paintEvent(QPaintEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

private slots:
    void onPhysicsTick();

private:
    PlayerInput buildInput();

    GameEngine m_engine;
    Camera m_camera;
    HorrorRenderer m_renderer;
    QTimer *m_physicsTimer = nullptr;
    QTimer *m_renderTimer = nullptr;
    QSet<int> m_heldKeys;
};
