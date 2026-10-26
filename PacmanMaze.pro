QT += core gui widgets
CONFIG += c++17
TARGET = PacmanMaze
TEMPLATE = app

win32: LIBS += -lwinmm

INCLUDEPATH += $$PWD

HEADERS += \
    core/Constants.h \
    core/Types.h \
    core/SoundManager.h \
    maze/MazeCell.h \
    maze/IMazeGenerator.h \
    maze/BacktrackerGenerator.h \
    maze/StaticMazeGenerator.h \
    maze/MazeGrid.h \
    entities/Entity.h \
    entities/Player.h \
    entities/GhostAI.h \
    entities/Ghost.h \
    engine/WorldState.h \
    engine/GameEngine.h \
    ui/Camera.h \
    ui/HorrorRenderer.h \
    ui/GameView.h

SOURCES += \
    main.cpp \
    core/SoundManager.cpp \
    maze/BacktrackerGenerator.cpp \
    maze/StaticMazeGenerator.cpp \
    maze/MazeGrid.cpp \
    entities/Player.cpp \
    entities/GhostAI.cpp \
    entities/Ghost.cpp \
    engine/WorldState.cpp \
    engine/GameEngine.cpp \
    ui/Camera.cpp \
    ui/HorrorRenderer.cpp \
    ui/GameView.cpp
