#pragma once

#include <QVector>
#include <QPoint>
#include "maze/MazeCell.h"
#include "maze/IMazeGenerator.h"

class MazeGrid {
public:
    MazeGrid();

    bool load(IMazeGenerator &generator, int cols, int rows, unsigned int seed = 0);

    bool isValid(int x, int y) const;
    bool isWalkable(int x, int y) const;
    bool isWall(int x, int y) const;
    const MazeCell &cell(int x, int y) const;
    MazeCell &cell(int x, int y);

    bool hasLineOfSight(const QPoint &from, const QPoint &to, double maxRange = 12.0) const;
    QVector<QPoint> findPath(const QPoint &from, const QPoint &to) const;
    QPoint getNextStepTowards(const QPoint &from, const QPoint &to) const;

    void updateVision(const QPoint &centerPos, int squareHalfSize);
    void resetFog();

    int cols() const { return m_cols; }
    int rows() const { return m_rows; }
    const QPoint &startPos() const { return m_startPos; }
    const QPoint &exitPos() const { return m_exitPos; }

private:
    int m_cols = 0;
    int m_rows = 0;
    QPoint m_startPos;
    QPoint m_exitPos;
    QVector<QVector<MazeCell>> m_cells;
};
