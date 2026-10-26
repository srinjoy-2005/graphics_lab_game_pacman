#include <QApplication>
#include "ui/GameView.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Pacman Maze - Entity In The Dark");

    GameView window;
    window.setWindowTitle("Pacman Maze: Survival In The Dark");
    window.show();

    return app.exec();
}
