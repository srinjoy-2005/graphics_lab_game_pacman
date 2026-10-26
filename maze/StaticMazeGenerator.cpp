#include "maze/StaticMazeGenerator.h"

StaticMazeGenerator::StaticMazeGenerator() {}

bool StaticMazeGenerator::generate(int cols,
                                   int rows,
                                   QVector<QVector<MazeCell>> &grid,
                                   QPoint &outStartPos,
                                   QPoint &outExitPos,
                                   unsigned int /*seed*/) {
    // A crafted 29x29 horror maze template with central hubs and claustrophobic corridors
    grid.resize(rows);
    for (int y = 0; y < rows; ++y) {
        grid[y].resize(cols);
        for (int x = 0; x < cols; ++x) {
            bool isBorder = (x == 0 || y == 0 || x == cols - 1 || y == rows - 1);
            bool isPillar = (x % 2 == 0 && y % 2 == 0);
            bool isRoomWall = ((x == 7 || x == 21) && (y >= 5 && y <= 11 && y != 8));
            bool isCenterBlock = ((x >= 12 && x <= 16) && (y >= 12 && y <= 16) && !(x == 14 && y == 14));

            if (isBorder || isPillar || isRoomWall || isCenterBlock) {
                grid[y][x].type = TileType::Wall;
            } else {
                grid[y][x].type = TileType::Floor;
            }
            grid[y][x].visitedByPlayer = false;
            grid[y][x].inSquareVision = false;
        }
    }

    outStartPos = QPoint(1, 1);
    outExitPos  = QPoint(cols - 2, rows - 2);

    grid[outExitPos.y()][outExitPos.x()].type = TileType::ExitDoor;
    grid[outStartPos.y()][outStartPos.x()].type = TileType::Floor;

    return true;
}
