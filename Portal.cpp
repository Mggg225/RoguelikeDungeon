#include "Portal.h"
#include <QPainter>

Portal::Portal(QGraphicsItem *parent)
    : GameObject(parent)
{
    m_sprite.load(":/assets/Portal.png");
    setZValue(1);
}

QRectF Portal::boundingRect() const
{
    return QRectF(-kSize, -kSize, kSize * 2, kSize * 2);
}

void Portal::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (!m_sprite.isNull()) {
        painter->drawPixmap(QRect(-kSize, -kSize, kSize * 2, kSize * 2), m_sprite);
    }
}
