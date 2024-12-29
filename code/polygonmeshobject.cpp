#include "polygonmeshobject.h"

PolygonMeshObject::PolygonMeshObject()
{

}

void PolygonMeshObject::computeNormals() {
    vertexNormals.resize(vertices.size(), QVector3D(0, 0, 0));
    for (const auto& polygon : polygons) {
        QVector3D normal = QVector3D::crossProduct(
            vertices[polygon[1]] - vertices[polygon[0]],
            vertices[polygon[2]] - vertices[polygon[0]]).normalized();

        if (QVector3D::dotProduct(normal, vertices[polygon[0]] - position) < 0) { // Center of cube is (0, 0, 0)
            normal = -normal;
        }

        for (unsigned long index : polygon) {
            vertexNormals[index] += normal;
        }
    }

    for (auto& normal : vertexNormals) {
        normal.normalize();
    }
}

PolygonMeshObject::PolygonMeshObject(const PolygonMeshObject& other)
    : color(other.color),
      transparency(other.transparency),
      position(other.position),
      vertices(other.vertices),
      polygons(other.polygons),
      vertexNormals(other.vertexNormals) {
}
