#pragma once

#include "maze/IMazeGenerator.h"
#include <QRandomGenerator>

class BacktrackerGenerator : public IMazeGenerator {
public:
    BacktrackerGenerator(double loopProbability = 0.12);

    bool generate(int cols,
                  int rows,
                  QVector<QVector<MazeCell>> &grid,
                  QPoint &outStartPos,
                  QPoint &outExitPos,
                  unsigned int seed = 0) override;

private:
    void carvePassages(int cx, int cy, QVector<QVector<MazeCell>> &grid, int cols, int rows);
    void addBraidLoops(QVector<QVector<MazeCell>> &grid, int cols, int rows);
    QPoint findFarthestPoint(const QVector<QVector<MazeCell>> &grid, int cols, int rows, const QPoint &start);

    double m_loopProbability;
    QRandomGenerator m_rng;
};
