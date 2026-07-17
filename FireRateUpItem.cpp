#include "FireRateUpItem.h"
#include "GameManager.h"
#include <QPainter>
#include <algorithm>

FireRateUpItem::FireRateUpItem(QGraphicsScene *scene, const QPointF &pos)
    : Item(scene, pos) {}

void FireRateUpItem::onPickup()
{
    qreal current = GameManager::instance().attackCooldown();
    GameManager::instance().setAttackCooldown(std::max(0.15, current * GameManager::instance().fireRateMult()));
}

void FireRateUpItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setBrush(QBrush(QColor(255, 200, 50)));
    painter->setPen(QPen(Qt::yellow, 2));
    QPolygonF tri;
    tri << QPointF(0, -kSize) << QPointF(kSize, kSize) << QPointF(-kSize, kSize);
    painter->drawPolygon(tri);
}
QString FireRateUpItem::pickupMessage() const { return "Fire Rate Up!"; }
