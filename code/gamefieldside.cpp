#include "gamefieldside.h"

GameFieldSide::GameFieldSide()
{

}
GameFieldSide::GameFieldSide(QVector3D position, std::vector<QVector3D> points, QColor color, float transparency) {
    setColor(color);
    setPosition(position);
    setTransparency(transparency);
    generateMesh(points);
}


void GameFieldSide::generateMesh(std::vector<QVector3D> points) {
    for (auto& point : points)
        point -= position;
    vertices = points;

    polygons = {
        {0, 1, 3},
        {0, 2, 3},
        {4, 5, 7},
        {4, 6, 7},
        {0, 1, 5},
        {0, 4, 5},
        {0, 2, 6},
        {0, 4, 6},
        {2, 3, 7},
        {2, 6, 7},
        {1, 3, 7},
        {1, 5, 7}
    };

    computeNormals();
}

void GameFieldSide::generateMesh(int subdivisions) {
    /*
        param subdivisions has no effect
    */
}

