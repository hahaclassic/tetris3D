#include "scene.h"

Scene::Scene(QObject *parent)
    : QGraphicsScene{parent}
{

}

void Scene::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::RightButton) {
        isDragging = true;
        lastMousePos = event->scenePos();
    }
    QGraphicsScene::mousePressEvent(event);
}

void Scene::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    if (isDragging) {
        QPointF currentPos = event->scenePos();
        QPointF delta = currentPos - lastMousePos;
        lastMousePos = currentPos;
        emit mouseMoveDeltaEvent(delta);
    }
    QGraphicsScene::mouseMoveEvent(event);
}

void Scene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        isDragging = false;
    }
    QGraphicsScene::mouseReleaseEvent(event);
}

//void Scene::wheelEvent(QGraphicsSceneWheelEvent* event) {
//    qreal delta = event->delta();

//    emit wheelScrolledEvent(delta);

//    event->accept();
//}
