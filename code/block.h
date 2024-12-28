#ifndef BLOCK_H
#define BLOCK_H

#include "cube.h"
#include <vector>
#include <functional>

class Block {
private:
    QVector3D center;
    std::vector<std::shared_ptr<Cube>> cubes;

public:
    Block(const QVector3D& center = QVector3D(0, 0, 0));

    void addCube(std::shared_ptr<Cube> cube);
    void move(const QVector3D& offset);
    void scale(float factor);
    void rotate(const QMatrix4x4& rotationMatrix);

    void clear() {cubes.clear();};

    QVector3D getCenter() const { return center; }
    void setCenter(const QVector3D& newCenter) { center = newCenter; }

    std::vector<std::shared_ptr<Cube>>::iterator begin() { return cubes.begin(); }
    std::vector<std::shared_ptr<Cube>>::iterator end() { return cubes.end(); }
    std::vector<std::shared_ptr<Cube>>::const_iterator begin() const { return cubes.begin(); }
    std::vector<std::shared_ptr<Cube>>::const_iterator end() const { return cubes.end(); }
    std::vector<std::shared_ptr<Cube>>::const_iterator cbegin() const { return cubes.cbegin(); }
    std::vector<std::shared_ptr<Cube>>::const_iterator cend() const { return cubes.cend(); }
};

#endif // BLOCK_H
