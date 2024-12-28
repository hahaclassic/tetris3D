#include "cube.h"

//Cube::Cube(const QVector3D& position, float edgeLength, float rounding, const QColor& color, float transparency)
//    : position(position), edgeLength(edgeLength), rounding(qBound(0.0f, rounding, edgeLength / 2.0f)),
//      color(color), transparency(qBound(0.0f, transparency, 1.0f)) {}

Cube::Cube(const QVector3D& position, float edgeLength, float roundingRadius, const QColor& color, float transparency)
    : edgeLength(edgeLength) {
    rounding = qBound(0.0f, roundingRadius, edgeLength / 2.0f);
    setPosition(position); // Устанавливаем позицию из базового класса
    setColor(color);       // Устанавливаем цвет из базового класса
    setTransparency(transparency); // Устанавливаем прозрачность
}

Cube::Cube(const Cube& other, const QVector3D& position)
    : PolygonMeshObject(other), // Вызов конструктора копирования базового класса
      edgeLength(other.edgeLength),
      rounding(other.rounding) {
    setPosition(position);
}


void coordAdjustment(QVector3D& vec)
{
    float x = vec.x(), y = vec.y(), z = vec.z();
    if (abs(x) < EPS) { vec.setX(0); }
    if (abs(y) < EPS) { vec.setY(0); }
    if (abs(z) < EPS) { vec.setZ(0); }
}

void Cube::generateMesh(int subdivisions) {
    if (vertices.size() != 0) {
        vertices.clear();
        vertexNormals.clear();
        polygons.clear();
    }
    if (rounding < EPS) {
        createVerticesAndPolygonsWithoutRounding();
    } else {
        auto corners = generateRoundedCorners(subdivisions);
        createVerticesAndPolygons(corners);
    }
    computeNormals();
}

void Cube::createVerticesAndPolygonsWithoutRounding()
{
    float halfEdge = edgeLength / 2.0f;

    vertices = {
        {-halfEdge, -halfEdge, halfEdge},
        {halfEdge, -halfEdge, halfEdge},
        {halfEdge, halfEdge, halfEdge},
        {-halfEdge, halfEdge, halfEdge},
        {-halfEdge, -halfEdge, -halfEdge},
        {halfEdge, -halfEdge, -halfEdge},
        {halfEdge, halfEdge, -halfEdge},
        {-halfEdge, halfEdge, -halfEdge}
    };

    polygons = {
        {0, 1, 2},
        {0, 2, 3},

        {4, 5, 6},
        {4, 6, 7},

        {0, 3, 7},
        {0, 7, 4},

        {1, 2, 6},
        {1, 6, 5},

        {0, 1, 5},
        {0, 5, 4},

        {3, 2, 6},
        {3, 6, 7}
    };
}

void Cube::createVerticesAndPolygons(std::shared_ptr<Cube::Corners> corners)
{
    for (int i = 0; i < NUM_OF_CORNERS; i++)
        createCornerVerticesAndPolygons(corners->points[i], i);
    if (abs(edgeLength / 2 - rounding) < EPS)
        return;

    int halfCorners = NUM_OF_CORNERS / 2, next, lowerCurr, lowerNext;
    auto azimuteSize = corners->points[0].size(),
            polarSize = corners->points[0][0].size();
    auto cornerSize = azimuteSize * polarSize;

    for (int i = 0; i < halfCorners; i++) {
        next = (i + 1) % halfCorners;
        lowerCurr = (halfCorners + i);
        lowerNext = halfCorners + (i + 1) % halfCorners;

        for (int j = 1; j < polarSize; j++) {
            unsigned long trueFirstVertexIdxCurr = i*cornerSize + (azimuteSize-1)*polarSize + j-1,
                          trueFirstVertexIdxNext = next*cornerSize + j-1;
            if (j == 1) {
                trueFirstVertexIdxCurr = i * cornerSize;
                trueFirstVertexIdxNext = next * cornerSize;
            }
            polygons.push_back({trueFirstVertexIdxCurr,
                                trueFirstVertexIdxNext,
                                i*cornerSize + (azimuteSize-1)*polarSize + j});
            polygons.push_back({i*cornerSize + (azimuteSize-1)*polarSize + j,
                                trueFirstVertexIdxNext,
                                next*cornerSize + j});

        }
        for (int j = 1; j < polarSize; j++) {
            unsigned long trueFirstVertexIdxCurr = lowerCurr*cornerSize + (azimuteSize-1)*polarSize + j-1,
                          trueFirstVertexIdxNext = lowerNext*cornerSize + j-1;
            if (j == 1) {
                trueFirstVertexIdxCurr = lowerCurr * cornerSize;
                trueFirstVertexIdxNext = lowerNext * cornerSize;
            }
            polygons.push_back({trueFirstVertexIdxCurr,
                                trueFirstVertexIdxNext,
                                lowerCurr*cornerSize + (azimuteSize-1)*polarSize + j});
            polygons.push_back({lowerCurr*cornerSize + (azimuteSize-1)*polarSize + j,
                                trueFirstVertexIdxNext,
                                lowerNext*cornerSize + j});
        }
        for (int j = 1; j < azimuteSize; j++) {
            polygons.push_back({i*cornerSize + j*polarSize - 1,
                                lowerCurr*cornerSize + j*polarSize - 1,
                                i*cornerSize + (j+1)*polarSize - 1});
            polygons.push_back({i*cornerSize + (j+1)*polarSize - 1,
                                lowerCurr*cornerSize + j*polarSize - 1,
                                lowerCurr*cornerSize + (j+1)*polarSize - 1});
        }

        // Main side polygons
        polygons.push_back({(i+1)*cornerSize - 1, next*cornerSize + polarSize - 1,
                            lowerNext*cornerSize + polarSize - 1});
        polygons.push_back({(i+1)*cornerSize - 1, (lowerCurr + 1)*cornerSize - 1,
                            lowerNext*cornerSize + polarSize - 1});
    }

    // Top and under main polygons
    polygons.push_back({0, cornerSize, cornerSize*2});
    polygons.push_back({0, cornerSize*2, cornerSize*3});
    polygons.push_back({cornerSize*4, cornerSize*5, cornerSize*6});
    polygons.push_back({cornerSize*4, cornerSize*6, cornerSize*7});
}

void Cube::createCornerVerticesAndPolygons(std::vector<std::vector<QVector3D>>& corner, int cornerIdx)
{
    auto azimuteSize = corner.size(),
            polarSize = corner[0].size();
    auto cornerSize = azimuteSize * polarSize * cornerIdx;

    for (int j = 0; j < azimuteSize; j++) {
        for (int k = 0; k < polarSize; k++) {
            vertices.push_back(corner[j][k]);
            if (j < 1 || k < 1) {
                continue;
            }

            unsigned long trueVertexIdx = cornerSize + (j-1)*polarSize + (k-1);
            if (k == 1) {
                // Removing a vertex collision at the 1st vertex of each vertex set
                // (vertices with constant azimuth x (j)). The 1st vertex is always repeated.
                trueVertexIdx = cornerSize;
            }

            polygons.push_back({cornerSize + j*polarSize + k,
                                trueVertexIdx,
                                cornerSize + (j-1)*polarSize + k});
            if (k < polarSize - 1) {
                polygons.push_back({cornerSize + j*polarSize + k,
                                    cornerSize + (j-1)*polarSize + k,
                                    cornerSize + j*polarSize + k+1});
            }
        }
    }
}

std::shared_ptr<Cube::Cube::Corners> Cube::generateRoundedCorners(int subdivisions) {
    float halfEdge = edgeLength / 2.0f;
    QVector3D center = QVector3D(halfEdge-rounding, halfEdge-rounding, halfEdge-rounding);

    auto corners = std::make_shared<Cube::Corners>();

    float step = M_PI_2 / subdivisions;

    for (int u = 0; u <= subdivisions; ++u) {
        for (int cornerIdx = 0; cornerIdx < 8; cornerIdx++)
            corners->points[cornerIdx].push_back({});

        float azimutal = u * step;

        for (int v = 0; v <= subdivisions; ++v) {
            float polar = v * step;

            QVector3D point = center + rounding * QVector3D(qSin(polar) * qCos(azimutal),
                                                            qSin(polar) * qSin(azimutal),
                                                            qCos(polar));
            addReflectedPoints(corners, point);
        }
    }

    return corners;
}

void Cube::addReflectedPoints(std::shared_ptr<Cube::Corners> corners, QVector3D point) {
    coordAdjustment(point);
    int last = corners->points[0].size() - 1;

    corners->points[0][last].push_back(point);
    corners->points[1][last].push_back(QVector3D(-point.y(), point.x(), point.z()));
    corners->points[2][last].push_back(QVector3D(-point.x(), -point.y(), point.z()));
    corners->points[3][last].push_back(QVector3D(point.y(), -point.x(), point.z()));

    corners->points[4][last].push_back(QVector3D(point.x(), point.y(), -point.z()));
    corners->points[5][last].push_back(QVector3D(-point.y(), point.x(), -point.z()));
    corners->points[6][last].push_back(QVector3D(-point.x(), -point.y(), -point.z()));
    corners->points[7][last].push_back(QVector3D(point.y(), -point.x(), -point.z()));
}
