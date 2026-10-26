#pragma once

#include <QVector>
#include <QPoint>
#include "maze/MazeCell.h"

class IMazeGenerator {
public:
    virtual ~IMazeGenerator() = default;

    // Generates a grid of size (cols x rows), setting startPos and exitPos
    virtual bool generate(int cols,
                          int rows,
                          QVector<QVector<MazeCell>> &grid,
                          QPoint &outStartPos,
                          QPoint &outExitPos,
                          unsigned int seed = 0) = 0;
};
