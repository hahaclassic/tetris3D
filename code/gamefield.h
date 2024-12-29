#ifndef GAMEFIELD_H
#define GAMEFIELD_H

#include "polygonmeshobject.h"
#include "cube.h"
#include "gamefieldside.h"
#include <vector>
#include <memory> 
#include <algorithm>

class GameField {
private:
    QColor gridColor;
    QVector3D position;
    int width, height, depth;                                     // Размеры игрового пространства (в ячейках)
    float cellSize;                                               // Размер ячейки (длина ребра куба)
    std::vector<std::vector<std::vector<std::shared_ptr<Cube>>>> field; 
    std::vector<GameFieldSide> sides;


    void removeCube(int x, int y, int z);

public:
    GameField();
    GameField(QVector3D position, int width, int depth, int height, float cellSize, QColor color, float transparency = 0.5f);
    void setUpSides(QColor color, float transparency);

    bool addCube(int x, int y, int z, std::shared_ptr<Cube> cube);

    std::shared_ptr<Cube> getCube(int x, int y, int z) const;
    std::vector<std::shared_ptr<Cube>> getAllCubes() const;
    std::vector<int> convertToPosInField(QVector3D globalPos) const;


    bool isLayerFull(int z) const;
    void clearLayer(int z);
    void moveLayer(int fromZ, int toZ);
    int removeFullLayers();
    void clear();

    bool isPositionValid(int x, int y, int z) const;
    bool isPositionEmpty(int x, int y, int z) const;

    inline int getWidth() const noexcept { return width; }
    inline int getHeight() const noexcept { return height; }
    inline int getDepth() const noexcept { return depth; }
    inline float getCellSize() const noexcept { return cellSize; }
    inline QVector3D getPosition() const noexcept { return position; }

    std::vector<std::pair<QVector3D, QVector3D>>& getCoordinateGrid() const;
    const inline std::vector<GameFieldSide>& getSides() const noexcept { return sides; }
};

#endif // GAMEFIELD_H
