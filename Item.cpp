#include "Item.h"
#include <QGraphicsScene>

Item::Item(QGraphicsScene *scene, const QPointF &pos, QGraphicsItem *parent)
    : GameObject(parent)
{
    setPos(pos);
    setZValue(1);
    scene->addItem(this);
}

QRectF Item::boundingRect() const
{
    return QRectF(-kSize, -kSize, kSize * 2, kSize * 2);
}
