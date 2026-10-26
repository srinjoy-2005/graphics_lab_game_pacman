#include "maze/BacktrackerGenerator.h"
#include <QQueue>
#include <QRandomGenerator>
#include <algorithm>
#include <vector>

BacktrackerGenerator::BacktrackerGenerator(double loopProbability)
    : m_loopProbability(loopProbability) {}

bool BacktrackerGenerator::generate(int cols,
                                    int rows,
                                    QVector<QVector<MazeCell>> &grid,
                                    QPoint &outStartPos,
                                    QPoint &outExitPos,
                                    unsigned int seed) {
    if (cols < 5 || rows < 5 || cols % 2 == 0 || rows % 2 == 0) {
        return false;
    }

    if (seed != 0) {
        m_rng.seed(seed);
    } else {
        m_rng = *QRandomGenerator::global();
    }

    // Initialize entire grid as solid walls
    grid.resize(rows);
    for (int y = 0; y < rows; ++y) {
        grid[y].resize(cols);
        for (int x = 0; x < cols; ++x) {
            grid[y][x].type = TileType::Wall;
            grid[y][x].visitedByPlayer = false;
            grid[y][x].inSquareVision = false;
        }
    }

    // Carve from (1, 1)
    carvePassages(1, 1, grid, cols, rows);

    // Carve loops so corridors interconnect (Backrooms horror style)
    addBraidLoops(grid, cols, rows);

    // Place player start at (1, 1) or center
    outStartPos = QPoint(1, 1);

    // Find the farthest reachable point for the Exit Door
    outExitPos = findFarthestPoint(grid, cols, rows, outStartPos);
    grid[outExitPos.y()][outExitPos.x()].type = TileType::ExitDoor;

    return true;
}

void BacktrackerGenerator::carvePassages(int cx, int cy, QVector<QVector<MazeCell>> &grid, int cols, int rows) {
    grid[cy][cx].type = TileType::Floor;

    // Directions: (dx, dy) stepping by 2
    std::vector<std::pair<int, int>> dirs = {
        {0, -2}, {0, 2}, {-2, 0}, {2, 0}
    };

    // Shuffle directions
    for (int i = 3; i > 0; --i) {
        int j = m_rng.bounded(i + 1);
        std::swap(dirs[i], dirs[j]);
    }

    for (const auto &d : dirs) {
        int nx = cx + d.first;
        int ny = cy + d.second;

        if (nx > 0 && nx < cols - 1 && ny > 0 && ny < rows - 1) {
            if (grid[ny][nx].type == TileType::Wall) {
                // Carve through the wall in between
                grid[cy + d.second / 2][cx + d.first / 2].type = TileType::Floor;
                carvePassages(nx, ny, grid, cols, rows);
            }
        }
    }
}

void BacktrackerGenerator::addBraidLoops(QVector<QVector<MazeCell>> &grid, int cols, int rows) {
    for (int y = 1; y < rows - 1; ++y) {
        for (int x = 1; x < cols - 1; ++x) {
            if (grid[y][x].type == TileType::Wall) {
                // If this wall separates two floor tiles horizontally or vertically
                bool horiz = (grid[y][x - 1].type == TileType::Floor && grid[y][x + 1].type == TileType::Floor);
                bool vert  = (grid[y - 1][x].type == TileType::Floor && grid[y + 1][x].type == TileType::Floor);

                if ((horiz || vert) && !(horiz && vert)) {
                    if (m_rng.generateDouble() < m_loopProbability) {
                        grid[y][x].type = TileType::Floor;
                    }
                }
            }
        }
    }
}

QPoint BacktrackerGenerator::findFarthestPoint(const QVector<QVector<MazeCell>> &grid,
                                              int cols,
                                              int rows,
                                              const QPoint &start) {
    QVector<QVector<int>> dist(rows, QVector<int>(cols, -1));
    QQueue<QPoint> queue;

    dist[start.y()][start.x()] = 0;
    queue.enqueue(start);

    QPoint farthest = start;
    int maxDist = 0;

    const int dx[] = {0, 0, -1, 1};
    const int dy[] = {-1, 1, 0, 0};

    while (!queue.isEmpty()) {
        QPoint curr = queue.dequeue();
        int d = dist[curr.y()][curr.x()];

        if (d > maxDist) {
            maxDist = d;
            farthest = curr;
        }

        for (int i = 0; i < 4; ++i) {
            int nx = curr.x() + dx[i];
            int ny = curr.y() + dy[i];

            if (nx >= 0 && nx < cols && ny >= 0 && ny < rows) {
                if (grid[ny][nx].type == TileType::Floor && dist[ny][nx] == -1) {
                    dist[ny][nx] = d + 1;
                    queue.enqueue(QPoint(nx, ny));
                }
            }
        }
    }

    return farthest;
}
