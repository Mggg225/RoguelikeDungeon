#include "Wall.h"
#include <QPainter>

Wall::Wall(qreal x, qreal y, qreal w, qreal h, QGraphicsItem *parent)
    : GameObject(parent), m_w(w), m_h(h)
{
    setPos(x + w / 2.0, y + h / 2.0);
}

QRectF Wall::boundingRect() const
{
    return QRectF(-m_w / 2.0, -m_h / 2.0, m_w, m_h);
}

void Wall::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QRectF r = boundingRect();
    const QColor stone(85, 78, 65);
    const QColor stoneDark(60, 54, 44);
    const QColor mortar(50, 45, 38);

    painter->setBrush(QBrush(stone));
    painter->setPen(Qt::NoPen);
    painter->drawRect(r);

    // 砖缝线 - 根据墙的方向画
    if (m_w > m_h) {
        // 水平墙：垂直砖缝
        painter->setPen(QPen(mortar, 1));
        qreal step = 30;
        for (qreal x = -m_w / 2.0 + step; x < m_w / 2.0; x += step) {
            painter->drawLine(QPointF(x, -m_h / 2.0 + 1), QPointF(x, m_h / 2.0 - 1));
        }
        // 水平中线
        painter->setPen(QPen(stoneDark, 1));
        painter->drawLine(QPointF(-m_w / 2.0 + 1, 0), QPointF(m_w / 2.0 - 1, 0));

        // 顶底收边
        painter->setPen(QPen(stoneDark, 2));
        painter->drawLine(QPointF(-m_w / 2.0, -m_h / 2.0 + 1), QPointF(m_w / 2.0, -m_h / 2.0 + 1));
        painter->drawLine(QPointF(-m_w / 2.0, m_h / 2.0 - 1), QPointF(m_w / 2.0, m_h / 2.0 - 1));
    } else {
        // 垂直墙：水平砖缝
        painter->setPen(QPen(mortar, 1));
        qreal step = 30;
        for (qreal y = -m_h / 2.0 + step; y < m_h / 2.0; y += step) {
            painter->drawLine(QPointF(-m_w / 2.0 + 1, y), QPointF(m_w / 2.0 - 1, y));
        }
        // 垂直中线
        painter->setPen(QPen(stoneDark, 1));
        painter->drawLine(QPointF(0, -m_h / 2.0 + 1), QPointF(0, m_h / 2.0 - 1));

        // 左右收边
        painter->setPen(QPen(stoneDark, 2));
        painter->drawLine(QPointF(-m_w / 2.0 + 1, -m_h / 2.0), QPointF(-m_w / 2.0 + 1, m_h / 2.0));
        painter->drawLine(QPointF(m_w / 2.0 - 1, -m_h / 2.0), QPointF(m_w / 2.0 - 1, m_h / 2.0));
    }

    // 外边框
    painter->setPen(QPen(QColor(35, 32, 28), 2));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(r);
}
