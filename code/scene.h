#ifndef SCENE_H
#define SCENE_H

#include <QGraphicsScene>
#include <QObject>
#include <QWidget>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QTransform>

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
    bool isDragging;       
    QPointF lastMousePos;  
    double lastAngleX;     
    double lastAngleY;     
};

#endif // SCENE_H
