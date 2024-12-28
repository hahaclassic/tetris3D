#ifndef GAMEFIELDSIDE_H
#define GAMEFIELDSIDE_H

#include "polygonmeshobject.h"

class GameFieldSide : public PolygonMeshObject
{
public:
    GameFieldSide();
    GameFieldSide(QVector3D position, std::vector<QVector3D> vertices, QColor color, float transparency);

    void generateMesh(int subdivisions) override final;
    void generateMesh(std::vector<QVector3D> vertices);
};

#endif // GAMEFIELDSIDE_H
