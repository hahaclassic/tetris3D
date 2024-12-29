#include "renderer.h"

// Конструктор
Renderer::Renderer(Scene* scene): scene(scene), lightPos(700.0f, 200.0f, 850.0f),
    background(QColor(255, 255, 255)), cameraPos(600.0f, 900.0f, 900.0f), withShadows(false) {
    buffer.resize(scene->width(), std::vector<double>(scene->height(), std::numeric_limits<double>::infinity()));
    shadowMap.resize(scene->width(), std::vector<double>(scene->height(), std::numeric_limits<double>::infinity()));
    image = QImage(scene->width(), scene->height(), QImage::Format_RGB32);
    image.fill(background);
    setTransformMatrices();
}

void Renderer::enableShadows(const bool &enable) {
    withShadows = enable;
}

void Renderer::setCameraPos(const QVector3D &pos) {
    cameraPos = pos;
    setTransformMatrices();
}

void Renderer::setLightPos(const QVector3D &pos) {
    lightPos = pos;
    setTransformMatrices();
}

void Renderer::rotateModelMatrix(const QPointF& delta) {
    const float sensitivity = 0.1f;
    float rotationAngle = delta.x() * sensitivity;
    std::fmod(rotationAngle, 360.0);

    QVector3D center(100, 100, 0);
    modelMatrix.translate(center);
    modelMatrix.rotate(rotationAngle, {0, 0, 1});
    modelMatrix.translate(-center);
    rotationMatrix.rotate(rotationAngle, {0, 0, 1});
}

//void Renderer::scaleModelMatrix(const int& delta) {
//    const float sensitivity = 0.001f;  // Чувствительность масштабирования
//    float scaleFactor = 1.0f + delta * sensitivity;  // Пропорциональное изменение по оси Y

//    // Ограничиваем масштабирование, чтобы не было слишком маленьким или слишком большим
//    scaleFactor = std::max(scaleFactor, 0.1f);  // Минимальный масштаб
//    scaleFactor = std::min(scaleFactor, 10.0f);  // Максимальный масштаб

//    QVector3D center(100, 100, 0);  // Центр масштабирования

//    modelMatrix.translate(center);
//    modelMatrix.scale(scaleFactor);  // Применяем масштабирование
//    modelMatrix.translate(-center);

//    rotationMatrix.scale(scaleFactor);
//}

void Renderer::setTransformMatrices() {
    const double aspectRatio = scene->width() / static_cast<double>(scene->height());
    const struct {
           double ratio, fov, nearPlane, farPlane;
       } projCameraConfig = {aspectRatio, 50.0f, 1.0f, 1000.0f};

    QMatrix4x4 cameraProjectionMatrix, cameraViewMatrix;
    cameraProjectionMatrix.perspective(projCameraConfig.fov, projCameraConfig.ratio, projCameraConfig.nearPlane, projCameraConfig.farPlane);

    QVector3D cameraTarget(200.0f, 200.0f, 125.0f); // Камера смотрит на центр сцены
    cameraViewMatrix.lookAt(cameraPos, cameraTarget, QVector3D(0.0f, 0.0f, 1.0f));

    cameraProjectionViewMatrix = cameraProjectionMatrix * cameraViewMatrix;

    const struct {
           double ratio, fov, nearPlane, farPlane;
       } projLightConfig = {aspectRatio, 90.0f, 1.0f, 1000.0f};

    QMatrix4x4 lightProjectionMatrix, lightViewMatrix;
    lightProjectionMatrix.perspective(projLightConfig.fov, projLightConfig.ratio, projLightConfig.nearPlane, projLightConfig.farPlane);
    lightViewMatrix.lookAt(lightPos, cameraTarget, QVector3D(0.0f, 0.0f, 1.0f));

    lightProjectionViewMatrix = lightProjectionMatrix * lightViewMatrix;
}

QColor alphaBlend(const QColor& foreground, const QColor& background) {
    float alpha = foreground.alphaF();

    int red = background.red() + (foreground.red() - background.red()) * alpha;
    int green = background.green() + (foreground.green() - background.green()) * alpha;
    int blue = background.blue() + (foreground.blue() - background.blue()) * alpha;
    int resultAlpha = background.alpha() + (foreground.alpha() - background.alpha()) * alpha;

    return QColor(red, green, blue, resultAlpha);
}

QVector4D Renderer::transformToScreenCoords(const QVector3D &worldCoord, const QMatrix4x4& projViewMatrix) {
    QVector4D screenCoord = projViewMatrix * QVector4D(worldCoord, 1.0f);
    screenCoord /= screenCoord.w();
    screenCoord.setX((screenCoord.x() + 1.0f) * 0.5f * scene->width());
    screenCoord.setY((1.0f - screenCoord.y()) * 0.5f * scene->height());
    return screenCoord;
}

QVector4D Renderer::convertCameraToLightCoords(const QVector3D &screenCoord) {
    QMatrix4x4 inverseProjectionView = cameraProjectionViewMatrix.inverted();
    QVector4D ndcCoord = QVector4D(
        (2.0f * screenCoord.x() / scene->width()) - 1.0f,
        1.0f - (2.0f * screenCoord.y() / scene->height()),
        screenCoord.z(),
        1.0f
    );

    QVector4D worldCoord = inverseProjectionView * ndcCoord;
    worldCoord /= worldCoord.w();
    return transformToScreenCoords(worldCoord.toVector3D(), lightProjectionViewMatrix);
}

void Renderer::rasterizeShadowTriangle(const QVector4D vertices[3]) {
    auto screenWidth = scene->width(), screenHeight = scene->height();

    double minX = std::max(0.0f, std::min({vertices[0].x(), vertices[1].x(), vertices[2].x()}));
    double maxX = std::min(static_cast<float>(screenWidth - 1), std::max({vertices[0].x(), vertices[1].x(), vertices[2].x()}));
    double minY = std::max(0.0f, std::min({vertices[0].y(), vertices[1].y(), vertices[2].y()}));
    double maxY = std::min(static_cast<float>(screenHeight - 1), std::max({vertices[0].y(), vertices[1].y(), vertices[2].y()}));

    QVector2D v0 = QVector2D(vertices[1].toVector2D()) - QVector2D(vertices[0].toVector2D());
    QVector2D v1 = QVector2D(vertices[2].toVector2D()) - QVector2D(vertices[0].toVector2D());
    double invDet = 1.0f / (v0.x() * v1.y() - v0.y() * v1.x());

#pragma omp parallel for
    for (int y = static_cast<int>(minY); y <= static_cast<int>(maxY); ++y) {
        for (int x = static_cast<int>(minX); x <= static_cast<int>(maxX); ++x) {
            QVector2D p = QVector2D(x, y) - QVector2D(vertices[0].toVector2D());
            double bary1 = (p.x() * v1.y() - p.y() * v1.x()) * invDet;
            double bary2 = (v0.x() * p.y() - v0.y() * p.x()) * invDet;
            double bary0 = 1.0f - bary1 - bary2;

//            double perspectiveBary0 = bary0 / vertices[0].w();
//            double perspectiveBary1 = bary1 / vertices[1].w();
//            double perspectiveBary2 = bary2 / vertices[2].w();
//            double perspectiveSum = perspectiveBary0 + perspectiveBary1 + perspectiveBary2;
//            perspectiveBary0 /= perspectiveSum;
//            perspectiveBary1 /= perspectiveSum;
//            perspectiveBary2 /= perspectiveSum;

//            double depth = perspectiveBary0 * vertices[0].z() +
//                                    perspectiveBary1 * vertices[1].z() +
//                                    perspectiveBary2 * vertices[2].z();

            if (bary0 < 0 || bary1 < 0 || bary2 < 0 || bary0 > 1 || bary1 > 1 || bary2 > 1)
                continue;

            double depth = bary0 * vertices[0].z() + bary1 * vertices[1].z() + bary2 * vertices[2].z();
            if (depth < shadowMap[x][y]) {
                #pragma omp atomic write
                shadowMap[x][y] = depth;
            }
        }
    }
}

void Renderer::renderShadowMesh(const PolygonMeshObject& obj, bool withBackFaceCulling) {
    const auto& vertices = obj.getVertices();
    const auto& polygons = obj.getPolygons();

    QVector3D objPosition = obj.getPosition(), rotatedObjPos = modelMatrix.map(objPosition);
    QVector<QVector3D> rotatedVertices(vertices.size());

    #pragma omp parallel for
    for (int i = 0; i < vertices.size(); i++) {
        rotatedVertices[i] = modelMatrix.map(vertices[i] + objPosition);
    }

    #pragma omp parallel for
    for (const auto &polygon : polygons)
    { 
        if (withBackFaceCulling) {
           QVector3D normal = QVector3D::crossProduct(
               rotatedVertices[polygon[1]] - rotatedVertices[polygon[0]],
               rotatedVertices[polygon[2]] - rotatedVertices[polygon[0]]).normalized();

           if (QVector3D::dotProduct(normal, (rotatedVertices[polygon[0]] - rotatedObjPos).normalized()) < 0) {
               normal = -normal;
           }

           if (QVector3D::dotProduct(normal, (lightPos - rotatedVertices[polygon[0]]).normalized()) < 0) {
               continue;
           }
        }

        QVector4D polygonVertices[3];
        for (int i = 0; i < polygon.size(); i++)
        {
            polygonVertices[i] = transformToScreenCoords(rotatedVertices[polygon[i]], lightProjectionViewMatrix);
        }
        rasterizeShadowTriangle(polygonVertices);
    }
}

QColor Renderer::applyIntensity(const QColor& color, float intensity) {
    intensity = std::clamp(intensity, 0.0f, 1.0f);

    int r = static_cast<int>(color.red() * intensity);
    int g = static_cast<int>(color.green() * intensity);
    int b = static_cast<int>(color.blue() * intensity);

    return QColor(r, g, b, color.alpha());
}

float Renderer::computeIntensity(
    const QVector3D& normal,
    const QVector3D& vertex,
    double ambientIntensity,
    double diffuseIntensity,
    double ambientCoefficient,
    double diffuseCoefficient
) {
    QVector3D normalizedNormal = normal.normalized();
    QVector3D normalizedLightDir = (lightPos - vertex).normalized();

    float cosTheta = QVector3D::dotProduct(normalizedNormal, normalizedLightDir);

    cosTheta = std::max(0.0f, cosTheta);

    float intensity = ambientIntensity * ambientCoefficient +
                      diffuseIntensity * diffuseCoefficient * cosTheta;

    return std::clamp(intensity, 0.0f, 1.0f);
}


void Renderer::rasterizeTriangle(const QVector4D vertices[3], const double intensities[3], QColor color)
{
    auto screenWidth = scene->width(), screenHeight = scene->height();

    double minX = std::max(0.0f, std::min({ vertices[0].x(), vertices[1].x(), vertices[2].x() }));
    double maxX = std::min(static_cast<float>(screenWidth - 1), std::max({ vertices[0].x(), vertices[1].x(), vertices[2].x() }));
    double minY = std::max(0.0f, std::min({ vertices[0].y(), vertices[1].y(), vertices[2].y() }));
    double maxY = std::min(static_cast<float>(screenHeight - 1), std::max({ vertices[0].y(), vertices[1].y(), vertices[2].y() }));

    QVector2D v0 = QVector2D(vertices[1].toVector2D()) - QVector2D(vertices[0].toVector2D());
    QVector2D v1 = QVector2D(vertices[2].toVector2D()) - QVector2D(vertices[0].toVector2D());
    double vecMul = v0.x() * v1.y() - v0.y() * v1.x();
    if (fabs(vecMul) < EPS)
        return;
    double invDet = 1.0f / vecMul;

    //double intensity = (intensities[0] + intensities[1] + intensities[2]) / 3; // simple filling

#pragma omp parallel for
    for (int y = static_cast<int>(minY); y <= static_cast<int>(maxY); ++y) {
        for (int x = static_cast<int>(minX); x <= static_cast<int>(maxX); ++x) {
            QVector2D p = QVector2D(x, y) - QVector2D(vertices[0].toVector2D());
            double bary1 = (p.x() * v1.y() - p.y() * v1.x()) * invDet;
            double bary2 = (v0.x() * p.y() - v0.y() * p.x()) * invDet;
            double bary0 = 1.0f - bary1 - bary2;

            if (bary0 < 0 || bary1 < 0 || bary2 < 0 || bary0 > 1 || bary1 > 1 || bary2 > 1)
                continue;

//            double perspectiveBary0 = bary0 / vertices[0].w();
//            double perspectiveBary1 = bary1 / vertices[1].w();
//            double perspectiveBary2 = bary2 / vertices[2].w();
//            double perspectiveSum = perspectiveBary0 + perspectiveBary1 + perspectiveBary2;
//            perspectiveBary0 /= perspectiveSum;
//            perspectiveBary1 /= perspectiveSum;
//            perspectiveBary2 /= perspectiveSum;

//            double depth = perspectiveBary0 * vertices[0].z() +
//                                    perspectiveBary1 * vertices[1].z() +
//                                    perspectiveBary2 * vertices[2].z();

            double depth = bary0 * vertices[0].z() + bary1 * vertices[1].z() + bary2 * vertices[2].z();

            if (depth >= buffer[x][y])
                continue;

            double intensity = bary0 * intensities[0] + bary1 * intensities[1] + bary2 * intensities[2];

            if (withShadows) {
                QVector4D shadowPos = convertCameraToLightCoords(QVector3D(x, y, depth));
                long shadowX = static_cast<long>(shadowPos.x()), shadowY = static_cast<long>(shadowPos.y());

                if (shadowMap[shadowX][shadowY] < shadowPos.z() - 0.00005f) {
                    intensity *= 0.4;
                }
            }

            QColor interpolatedColor = applyIntensity(color, intensity);
            if (interpolatedColor.alpha() != 255)
                interpolatedColor = alphaBlend(interpolatedColor, image.pixelColor(x, y));


            #pragma omp critical
            {
            image.setPixelColor(x, y, interpolatedColor);
            buffer[x][y] = depth;
            }
        }
    }
}

void Renderer::renderPolygonMeshObject(const PolygonMeshObject& obj, bool withBackFaceCulling) {
    const auto& vertices = obj.getVertices();
    const auto& polygons = obj.getPolygons();
    const auto& normals = obj.getVertexNormals();

    QVector3D objPosition = obj.getPosition(), rotatedObjPos = modelMatrix.map(objPosition);
    QVector<QVector3D> rotatedVertices(vertices.size());
    QVector<QVector3D> rotatedNormals(normals.size());

    #pragma omp parallel for
    for (int i = 0; i < vertices.size(); i++) {
        rotatedVertices[i] = modelMatrix.map(vertices[i] + objPosition);
        rotatedNormals[i] = rotationMatrix.map(normals[i]).normalized();
    }

    #pragma omp parallel for
    for (const auto &polygon : polygons)
    {
        if (withBackFaceCulling) {
           QVector3D normal = QVector3D::crossProduct(
               rotatedVertices[polygon[1]] - rotatedVertices[polygon[0]],
               rotatedVertices[polygon[2]] - rotatedVertices[polygon[0]]).normalized();

           if (QVector3D::dotProduct(normal, (rotatedVertices[polygon[0]] - rotatedObjPos).normalized()) < 0) {
               normal = -normal;
           }

           if (QVector3D::dotProduct(normal, (cameraPos - rotatedVertices[polygon[0]]).normalized()) < 0) {
               continue;
           }
        }

        QVector4D polygonVertices[3];
        double intensities[3];
        for (int i = 0; i < polygon.size(); i++)
        {
            polygonVertices[i] = transformToScreenCoords(rotatedVertices[polygon[i]], cameraProjectionViewMatrix);
            intensities[i] = computeIntensity(rotatedNormals[polygon[i]], rotatedVertices[polygon[i]], 0.3f, 1.0f, 0.5f, 0.6f);
        }
        rasterizeTriangle(polygonVertices, intensities, obj.getColor());
    }
}

void Renderer::renderShadows(const GameField& field, const Block& activeBlock) {
    for (const auto& cube : activeBlock) {
        renderShadowMesh(*cube.get(), true);
    }

    for (const auto& cube : field.getAllCubes()) {
        renderShadowMesh(*cube.get(), true);
    }

    for (const auto& side : field.getSides()) {
        renderShadowMesh(side, true);
    }
}

void Renderer::renderScene(const GameField& field, const Block& activeBlock, const Block& projBlock)
{
    scene->clear();
    image.fill(background);

    bool withBackFaceCulling = true;

    for (int i = 0; i < buffer.size(); i++) {
        for (int j = 0; j < buffer[i].size(); j++) {
            buffer[i][j] = std::numeric_limits<double>::infinity();
        }
    }

    if (withShadows) {
        for (int i = 0; i < shadowMap.size(); i++) {
            for (int j = 0; j < shadowMap[i].size(); j++) {
                shadowMap[i][j] = std::numeric_limits<double>::infinity();
            }
        }
        renderShadows(field, activeBlock);
    }

    for (const auto& cube : activeBlock) {
        renderPolygonMeshObject(*cube.get(), withBackFaceCulling);
    }

    for (const auto& cube : field.getAllCubes()) {
        renderPolygonMeshObject(*cube.get(), withBackFaceCulling);
    }

    for (const auto& side : field.getSides())
        renderPolygonMeshObject(side, withBackFaceCulling);

    for (const auto& cube : projBlock) {
        renderPolygonMeshObject(*cube.get(), true);
    }

    QGraphicsPixmapItem* pixmapItem = new QGraphicsPixmapItem(QPixmap::fromImage(image));
    scene->addItem(pixmapItem);
    scene->update();
}


// Функция для замера времени рендеринга сцены
void Renderer::renderCubes(std::vector<Cube> cubes) {
    scene->clear();
    image.fill(background);

    bool withBackFaceCulling = true;

    for (int i = 0; i < buffer.size(); i++) {
        for (int j = 0; j < buffer[i].size(); j++) {
            buffer[i][j] = std::numeric_limits<double>::infinity();
        }
    }

    if (withShadows) {
        for (int i = 0; i < shadowMap.size(); i++) {
            for (int j = 0; j < shadowMap[i].size(); j++) {
                shadowMap[i][j] = std::numeric_limits<double>::infinity();
            }
        }
        for (const auto& cube : cubes) {
            renderShadowMesh(cube, withBackFaceCulling);
        }
    }

    for (const auto& cube : cubes) {
        renderPolygonMeshObject(cube, withBackFaceCulling);
    }

    QGraphicsPixmapItem* pixmapItem = new QGraphicsPixmapItem(QPixmap::fromImage(image));
    scene->addItem(pixmapItem);
    scene->update();
}
