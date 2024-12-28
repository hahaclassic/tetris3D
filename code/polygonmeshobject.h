#ifndef POLYGONMESHOBJECT_H
#define POLYGONMESHOBJECT_H

#include <QVector3D>
#include <QMatrix4x4>
#include <QColor>
#include <vector>
#include <QtMath>
#include <iostream>
#include <QDebug>
#include "consts.h"

class PolygonMeshObject
{
protected:
    QColor color;
    float transparency;

    QVector3D position;
    std::vector<QVector3D> vertices;
    std::vector<std::vector<unsigned long>> polygons;
    std::vector<QVector3D> vertexNormals;

    virtual void computeNormals();
public:
    inline const std::vector<QVector3D>& getVertices() const noexcept  { return vertices; }
    inline const std::vector<QVector3D>& getVertexNormals() const noexcept { return vertexNormals; }
    inline const std::vector<std::vector<unsigned long>>& getPolygons() const noexcept { return polygons; }

    void setPosition(const QVector3D& pos) { position = pos; }
    inline QVector3D getPosition() const noexcept { return position; }

    void move(const QVector3D& offset) { position += offset; }

    void setColor(const QColor& col) { color = col; }
    inline QColor getColor() const noexcept { return color; }

    void setTransparency(float alpha) {
        transparency = qBound(0.0f, alpha, 1.0f);
        color.setAlphaF(alpha);
    }

    inline float getTransparency() const noexcept { return transparency; }

    virtual void generateMesh(int subdivisions) = 0;

    PolygonMeshObject();
    PolygonMeshObject(const PolygonMeshObject& other);
};

#endif // POLYGONMESHOBJECT_H
