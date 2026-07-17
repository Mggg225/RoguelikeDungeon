#include "Door.h"
#include <QPainter>

Door::Door(Direction dir, int targetRoomId, QGraphicsItem *parent)
    : GameObject(parent), m_direction(dir), m_targetRoom(targetRoomId)
{
    setZValue(0);
}

QRectF Door::boundingRect() const
{
    int w = doorW(), h = doorH();
    return QRectF(-w / 2.0, -h / 2.0, w, h);
}

void Door::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    int w = doorW(), h = doorH();
    QColor bg = m_locked ? QColor(120, 60, 60) : QColor(100, 80, 60);
    painter->setBrush(QBrush(bg));
    painter->setPen(QPen(Qt::darkYellow, 1));
    painter->drawRect(QRectF(-w / 2.0, -h / 2.0, w, h));
}
