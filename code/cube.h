#ifndef CUBE_H
#define CUBE_H

#include <QVector3D>
#include <QMatrix4x4>
#include <QColor>
#include <vector>
#include <QtMath>
#include <iostream>
#include "consts.h"
#include "polygonmeshobject.h"

#define NUM_OF_CORNERS 8

class Cube : public PolygonMeshObject {
private:
    float edgeLength;   // Длина ребра куба
    float rounding;     // Радиус скругления

    struct Corners {
        std::vector<std::vector<QVector3D>> points[NUM_OF_CORNERS];
    };

    std::shared_ptr<Corners> generateRoundedCorners(int subdivisions);

    void addReflectedPoints(std::shared_ptr<Corners> corners, QVector3D point);
    void createVerticesAndPolygons(std::shared_ptr<Corners> corners);
    void createCornerVerticesAndPolygons(std::vector<std::vector<QVector3D>>& corner, int cornerIdx);
    void createVerticesAndPolygonsWithoutRounding();
public:
    Cube(const QVector3D& position = QVector3D(0, 0, 0),
         float edgeLength = 6.0f,
         float roundingRadius = 0.0f,
         const QColor& color = QColor(255, 255, 255),
         float transparency = 1.0f);

    Cube(const Cube& other, const QVector3D& position);

    void generateMesh(int subdivisions = 2) override final;

    void setRounding(float round) { rounding = qBound(0.0f, round, edgeLength / 2.0f); }
    inline float getRounding() const { return rounding; }
};

#endif
