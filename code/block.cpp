#include "block.h"

// Конструктор
Block::Block(const QVector3D& center) : center(center) {}

// Добавление куба в блок
void Block::addCube(std::shared_ptr<Cube> cube) {
    cubes.push_back(cube);
}

void Block::move(const QVector3D& offset) {
    // Перемещаем центр блока
    center += offset;

    // Перемещаем все кубы
    for (auto& cube : cubes) {
        cube->move(offset);
    }
}

void Block::rotate(const QMatrix4x4& rotationMatrix) {
    for (auto& cube : cubes) {
        QVector3D relativePosition = cube->getPosition() - center;
        QVector3D rotatedPosition = rotationMatrix.map(relativePosition);
        cube->setPosition(center + rotatedPosition);
    }
}

