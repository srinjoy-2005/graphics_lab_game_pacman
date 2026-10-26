#pragma once

#include "maze/IMazeGenerator.h"

class StaticMazeGenerator : public IMazeGenerator {
public:
    StaticMazeGenerator();

    bool generate(int cols,
                  int rows,
                  QVector<QVector<MazeCell>> &grid,
                  QPoint &outStartPos,
                  QPoint &outExitPos,
                  unsigned int seed = 0) override;
};
