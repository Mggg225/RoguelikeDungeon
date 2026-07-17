#include "BombItem.h"
#include "GameManager.h"
#include <QPainter>

BombItem::BombItem(QGraphicsScene *scene, const QPointF &pos)
    : Item(scene, pos) {}

void BombItem::onPickup()
{
    GameManager::instance().setHasBomb(true);
}

void BombItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setBrush(QBrush(QColor(40, 40, 40)));
    painter->setPen(QPen(Qt::darkGray, 2));
    painter->drawEllipse(QPointF(0, 0), kSize, kSize);
    // 引线
    painter->setPen(QPen(QColor(255, 150, 50), 1.5));
    painter->drawLine(QPointF(kSize, -kSize/2), QPointF(kSize+4, -kSize));
    painter->setPen(QPen(Qt::red, 2));
    painter->drawPoint(QPointF(kSize+4, -kSize));
}
QString BombItem::pickupMessage() const { return "Bomb - press 1 to use!"; }
