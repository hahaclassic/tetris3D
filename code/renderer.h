#ifndef RENDERER_H
#define RENDERER_H

#include <omp.h>
#include "polygonmeshobject.h"
#include "gamefield.h"
#include "block.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <iostream>
#include "scene.h"
#include <QColor>
#include <QPen>
#include <QBrush>
#include <limits>
#include <QMatrix4x4>
#include <QPainter>
#include <QImage>
#include <QGraphicsPixmapItem>
#include <scene.h>
#include <QSize>
#include <QPointF>
#include <chrono>

class Renderer {
private:
    QMatrix4x4 lightProjectionViewMatrix;
    QMatrix4x4 cameraProjectionViewMatrix;
    QMatrix4x4 modelMatrix;
    QMatrix4x4 rotationMatrix;

    Scene *scene;
    std::vector<std::vector<double>> buffer;
    std::vector<std::vector<double>> shadowMap;
    std::vector<std::vector<std::pair<QColor, float>>> transparencyBuffer;

    QColor background;
    QVector3D lightPos;
    QVector3D cameraPos;
    bool withShadows;
    QImage image;

    void renderPolygonMeshObject(const PolygonMeshObject& obj, bool withBackFaceCulling);

    void renderCoordinateGrid(const GameField& field);

    void rasterizeTriangle(const QVector4D vertices[3], const double intensities[3], QColor color);

    void rasterizeLine(const QVector4D start, const QVector4D end);

    void setTransformMatrices();
    QVector4D transformToScreenCoords(const QVector3D &worldCoord, const QMatrix4x4& projViewMatrix);
    QVector4D convertCameraToLightCoords(const QVector3D &screenCoord);

    float computeIntensity(
        const QVector3D& normal,
        const QVector3D& vertex,
        double ambientIntensity,
        double diffuseIntensity,
        double ambientCoefficient,
        double diffuseCoefficient
    );

    QColor applyIntensity(const QColor& color, float intensity);

    void renderShadows(const GameField& field, const Block& activeBlock);
    void renderShadowMesh(const PolygonMeshObject& obj, bool withBackFaceCulling);
    void rasterizeShadowTriangle(const QVector4D vertices[3]);
    QVector4D transformToLightPos(const QVector2D &screenCoord, double depth);

private slots:
    void rotate(const QPointF& delta);

public:
    explicit Renderer(Scene* scene);

    void renderScene(const GameField& field, const Block& activeBlock, const Block& projBlock);

    void rotateModelMatrix(const QPointF& delta);
    void scaleModelMatrix(const int& delta);

    void enableShadows(const bool& withShadows);
    void setCameraPos(const QVector3D& pos);
    void setLightPos(const QVector3D& pos);

    void renderCubes(std::vector<Cube> cubes);
};

#endif // RENDERER_H
