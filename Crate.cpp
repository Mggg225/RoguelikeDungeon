#include "Crate.h"
#include <QPainter>

Crate::Crate(const QPointF &pos, QGraphicsItem *parent)
    : GameObject(parent)
{
    setPos(pos);
    setZValue(0);
}

QRectF Crate::boundingRect() const
{
    return QRectF(-kSize, -kSize, kSize * 2, kSize * 2);
}

void Crate::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    // 木箱主体
    painter->setBrush(QBrush(QColor(120, 90, 50)));
    painter->setPen(QPen(QColor(80, 55, 25), 2));
    painter->drawRect(boundingRect());

    // 木板横纹
    painter->setPen(QPen(QColor(90, 65, 30), 1));
    painter->drawLine(QPointF(-kSize, -kSize/3), QPointF(kSize, -kSize/3));
    painter->drawLine(QPointF(-kSize, kSize/3), QPointF(kSize, kSize/3));

    // 十字绑带
    painter->setPen(QPen(QColor(160, 130, 80), 1.5));
    painter->drawLine(QPointF(-kSize + 3, -kSize + 3), QPointF(kSize - 3, kSize - 3));
    painter->drawLine(QPointF(-kSize + 3, kSize - 3), QPointF(kSize - 3, -kSize + 3));

    // 受击变色
    if (m_hp <= 1) {
        painter->setBrush(QBrush(QColor(255, 0, 0, 80)));
        painter->setPen(Qt::NoPen);
        painter->drawRect(boundingRect());
    }
}

void Crate::takeDamage(int dmg)
{
    m_hp -= dmg;
    if (m_hp < 0) m_hp = 0;
}
