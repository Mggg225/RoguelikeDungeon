#include "AddBulletCountItem.h"
#include "GameManager.h"
#include <QPainter>

AddBulletCountItem::AddBulletCountItem(QGraphicsScene *scene, const QPointF &pos)
    : Item(scene, pos) {}

void AddBulletCountItem::onPickup()
{
    GameManager::instance().setExtraBullets(GameManager::instance().extraBullets() + 1);
}

void AddBulletCountItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    // 橙色子弹图标
    painter->setBrush(QBrush(QColor(255, 160, 50)));
    painter->setPen(QPen(Qt::darkYellow, 2));
    // 菱形
    QPolygonF diamond;
    diamond << QPointF(0, -kSize) << QPointF(kSize, 0)
            << QPointF(0, kSize) << QPointF(-kSize, 0);
    painter->drawPolygon(diamond);
    painter->setPen(QPen(Qt::white, 1));
    painter->drawText(boundingRect(), Qt::AlignCenter, "+");
}

QString AddBulletCountItem::pickupMessage() const { return "Extra Bullets!"; }
