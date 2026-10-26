#include "maze/MazeGrid.h"
#include <QQueue>
#include <cmath>

MazeGrid::MazeGrid() {}

bool MazeGrid::load(IMazeGenerator &generator, int cols, int rows, unsigned int seed) {
    m_cols = cols;
    m_rows = rows;
    return generator.generate(cols, rows, m_cells, m_startPos, m_exitPos, seed);
}

bool MazeGrid::isValid(int x, int y) const {
    return x >= 0 && x < m_cols && y >= 0 && y < m_rows;
}

bool MazeGrid::isWalkable(int x, int y) const {
    if (!isValid(x, y)) return false;
    return m_cells[y][x].isWalkable();
}

bool MazeGrid::isWall(int x, int y) const {
    if (!isValid(x, y)) return true;
    return m_cells[y][x].isWall();
}

const MazeCell &MazeGrid::cell(int x, int y) const {
    return m_cells[y][x];
}

MazeCell &MazeGrid::cell(int x, int y) {
    return m_cells[y][x];
}

bool MazeGrid::hasLineOfSight(const QPoint &from, const QPoint &to, double maxRange) const {
    int dx = to.x() - from.x();
    int dy = to.y() - from.y();
    double dist = std::sqrt(dx * dx + dy * dy);
    if (dist > maxRange) return false;

    // Bresenham's line algorithm
    int x0 = from.x();
    int y0 = from.y();
    int x1 = to.x();
    int y1 = to.y();

    int stepX = (x0 < x1) ? 1 : -1;
    int stepY = (y0 < y1) ? 1 : -1;

    int absDx = std::abs(x1 - x0);
    int absDy = std::abs(y1 - y0);
    int err = absDx - absDy;

    while (x0 != x1 || y0 != y1) {
        if ((x0 != from.x() || y0 != from.y()) && isWall(x0, y0)) {
            return false;
        }

        int e2 = 2 * err;
        if (e2 > -absDy) {
            err -= absDy;
            x0 += stepX;
        }
        if (e2 < absDx) {
            err += absDx;
            y0 += stepY;
        }
    }

    return true;
}

QVector<QPoint> MazeGrid::findPath(const QPoint &from, const QPoint &to) const {
    if (from == to || !isWalkable(to.x(), to.y())) return {};

    QVector<QVector<QPoint>> parent(m_rows, QVector<QPoint>(m_cols, QPoint(-1, -1)));
    QVector<QVector<bool>> visited(m_rows, QVector<bool>(m_cols, false));
    QQueue<QPoint> queue;

    queue.enqueue(from);
    visited[from.y()][from.x()] = true;

    const int dx[] = {0, 0, -1, 1};
    const int dy[] = {-1, 1, 0, 0};
    bool found = false;

    while (!queue.isEmpty()) {
        QPoint curr = queue.dequeue();
        if (curr == to) {
            found = true;
            break;
        }

        for (int i = 0; i < 4; ++i) {
            int nx = curr.x() + dx[i];
            int ny = curr.y() + dy[i];

            if (isValid(nx, ny) && isWalkable(nx, ny) && !visited[ny][nx]) {
                visited[ny][nx] = true;
                parent[ny][nx] = curr;
                queue.enqueue(QPoint(nx, ny));
            }
        }
    }

    if (!found) return {};

    QVector<QPoint> path;
    QPoint curr = to;
    while (curr != from) {
        path.prepend(curr);
        curr = parent[curr.y()][curr.x()];
    }
    return path;
}

QPoint MazeGrid::getNextStepTowards(const QPoint &from, const QPoint &to) const {
    QVector<QPoint> path = findPath(from, to);
    if (!path.isEmpty()) {
        return path.first();
    }
    return from;
}

void MazeGrid::updateVision(const QPoint &centerPos, int squareHalfSize) {
    // Reset currently visible flag for all cells
    for (int y = 0; y < m_rows; ++y) {
        for (int x = 0; x < m_cols; ++x) {
            m_cells[y][x].inSquareVision = false;
        }
    }

    // Set square window
    int minX = qMax(0, centerPos.x() - squareHalfSize);
    int maxX = qMin(m_cols - 1, centerPos.x() + squareHalfSize);
    int minY = qMax(0, centerPos.y() - squareHalfSize);
    int maxY = qMin(m_rows - 1, centerPos.y() + squareHalfSize);

    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            m_cells[y][x].inSquareVision = true;
            m_cells[y][x].visitedByPlayer = true;

            // Distance attenuation
            double dist = std::sqrt(std::pow(x - centerPos.x(), 2) + std::pow(y - centerPos.y(), 2));
            float light = static_cast<float>(1.0 - (dist / (squareHalfSize * 1.414)));
            m_cells[y][x].lightIntensity = qBound(0.15f, light, 1.0f);
        }
    }
}

void MazeGrid::resetFog() {
    for (int y = 0; y < m_rows; ++y) {
        for (int x = 0; x < m_cols; ++x) {
            m_cells[y][x].inSquareVision = false;
            m_cells[y][x].visitedByPlayer = false;
        }
    }
}
