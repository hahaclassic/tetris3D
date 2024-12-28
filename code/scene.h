#ifndef SCENE_H
#define SCENE_H

#include <QGraphicsScene>
#include <QObject>
#include <QWidget>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QTransform>
#include <QDebug>

class Scene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit Scene(QObject *parent = nullptr);

signals:
    void mouseMoveDeltaEvent(const QPointF& delta);

    void wheelScrolledEvent(const int& delta);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;

    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

    // void wheelEvent(QGraphicsSceneWheelEvent* event) override;
private:
    bool isDragging;       // Флаг, указывающий на то, что происходит перетаскивание
    QPointF lastMousePos;  // Последняя позиция мыши
    double lastAngleX;     // Угол вращения вокруг оси X
    double lastAngleY;     // Угол вращения вокруг оси Y
};

#endif // SCENE_H
