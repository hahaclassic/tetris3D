#include "gamefield.h"
#include <stdexcept>

GameField::GameField() {
    GameField({0, 0, 0}, 4, 4, 8, 50, QColor(255, 255, 255));
}

GameField::GameField(QVector3D position, int width, int depth, int height, float cellSize, QColor color, float transparency)
    : width(width), height(height), depth(depth), cellSize(cellSize), position(position) {
    field.resize(height, std::vector<std::vector<std::shared_ptr<Cube>>>(width, std::vector<std::shared_ptr<Cube>>(depth)));
    setUpSides(color, transparency);
}

void GameField::setUpSides(QColor color, float transparency) {
    float realWidth = width * cellSize,
          realDepth = depth * cellSize,
          realHeight = height * cellSize;

    float diff = -5;

    std::vector<QVector3D> vertices1 = {
        {0, 0, 0},
        {0, realDepth, 0},
        {realWidth, 0, 0},
        {realWidth, realDepth, 0},

        {diff, diff, diff},
        {diff, realDepth, diff},
        {realWidth, diff, diff},
        {realWidth, realDepth, diff},
    };

    std::vector<QVector3D> vertices2 = {
        {0, 0, 0},
        {0, realDepth, 0},
        {0, 0, realHeight},
        {0, realDepth, realHeight},

        {diff, diff, diff},
        {diff, realDepth, diff},
        {diff, diff, realHeight},
        {diff, realDepth, realHeight},
    };

    std::vector<QVector3D> vertices3 = {
        {0, 0, 0},
        {realWidth, 0, 0},
        {0, 0, realHeight},
        {realWidth, 0, realHeight},

        {diff, diff, diff},
        {realWidth, diff, diff},
        {diff, diff, realHeight},
        {realWidth, diff, realHeight}
    };

    sides.push_back(GameFieldSide(QVector3D(realWidth / 2, realDepth / 2, diff/2), vertices1, color, transparency));
    sides.push_back(GameFieldSide(QVector3D(diff/2, realDepth / 2, realHeight/2), vertices2, color, transparency));
    sides.push_back(GameFieldSide(QVector3D(realWidth / 2, diff/2, realHeight/2), vertices3, color, transparency));
}

bool GameField::addCube(int x, int y, int z, std::shared_ptr<Cube> cube) {
    if (!isPositionEmpty(x, y, z)) {
        return false;
    }
    field[z][x][y] = cube;

    return true;
}


std::shared_ptr<Cube> GameField::getCube(int x, int y, int z) const {
    if (!isPositionValid(x, y, z)) {
        return nullptr;
    }
    return field[z][x][y];
}

std::vector<std::shared_ptr<Cube>> GameField::getAllCubes() const {
    std::vector<std::shared_ptr<Cube>> cubes;

    for (int z = 0; z < height; z++) {
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < depth; y++) {
                if (field[z][x][y] != nullptr)
                    cubes.push_back(field[z][x][y]);
            }
        }
    }

    return cubes;
}

void GameField::removeCube(int x, int y, int z) {
    if (isPositionValid(x, y, z) && field[z][x][y] != nullptr) {
        field[z][x][y].reset();
    }
}

void GameField::clear() {
    for (int z = 0; z < height; z++) {
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < depth; y++) {
                removeCube(x, y, z);
            }
        }
    }
}

//int GameField::removeFullLayers() {
//    int countDeletedLayers = 0;
//    int newZ = 0;

//    for (int z = 0; z < height; z++) {
//        if (isLayerFull(z)) {
//            clearLayer(z);
//            countDeletedLayers++;
//        } else {
//            if (newZ != z) {
//                moveLayer(z, newZ);
//            }
//            newZ++;
//        }
//    }

//    return countDeletedLayers;
//}

//bool GameField::isLayerFull(int z) const {
//    for (int x = 0; x < width; x++) {
//        for (int y = 0; y < depth; y++) {
//            if (field[z][x][y] == nullptr) {
//                return false;
//            }
//        }
//    }
//    return true;
//}

//void GameField::clearLayer(int z) {
//    for (int x = 0; x < width; x++) {
//        for (int y = 0; y < depth; y++) {
//            field[z][x][y].reset();
//        }
//    }
//}

//void GameField::moveLayer(int fromZ, int toZ) {
//    field[toZ] = field[fromZ];
//    for (int x = 0; x < width; x++) {
//        for (int y = 0; y < depth; y++) {
//            if (field[fromZ][x][y] != nullptr) {
//                field[toZ][x][y]->move({0, 0, -(fromZ - toZ) * cellSize});
//            }
//        }
//    }
//}


int GameField::removeFullLayers() {
    int countDeletedLayers = 0;
    int newZ = 0;
    for (int z = 0; z < height; z++) {
        bool deleteLayer = true;
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < depth; y++) {
                if (field[z][x][y] == nullptr) {
                    deleteLayer = false;
                    break;
                }

            }
            if (!deleteLayer)
                break;
        }

        if (deleteLayer) {
            for (int x = 0; x < width; x++) {
                for (int y = 0; y < depth; y++) {
                    field[z][x][y].reset();
                }
            }
            countDeletedLayers++;
        } else if (newZ != z) {
            field[newZ] = field[z];
            for (int x = 0; x < width; x++) {
                for (int y = 0; y < depth; y++) {
                    if (field[z][x][y] != nullptr)
                        field[newZ][x][y].get()->move({0, 0, -(z-newZ)*cellSize});
                }
            }
            newZ++;
        } else {
            newZ++;
        }
    }

    return countDeletedLayers;
}

bool GameField::isPositionValid(int x, int y, int z) const {
    return x >= 0 && x < width && y >= 0 && y < depth && z >= 0 && z < height;
}

bool GameField::isPositionEmpty(int x, int y, int z) const {
    return isPositionValid(x, y, z) && field[z][x][y] == nullptr;
}

std::vector<int> GameField::convertToPosInField(QVector3D globalPos) const {
    float halfCellSize = cellSize/2;
    QVector3D shift = QVector3D(halfCellSize, halfCellSize, halfCellSize);
    QVector3D cubePosition = (globalPos - shift - position) / cellSize;
    int x = static_cast<int>(cubePosition.x());
    int y = static_cast<int>(cubePosition.y());
    int z = static_cast<int>(cubePosition.z());

    // qDebug() << globalPos << "(" << x << y << z << ")";

    return std::vector<int>({x, y, z});
}
