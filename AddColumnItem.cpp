#include "AddColumnItem.h"
#include "GameManager.h"
#include <QPainter>

AddColumnItem::AddColumnItem(QGraphicsScene *scene, const QPointF &pos)
    : Item(scene, pos) {}

void AddColumnItem::onPickup()
{
    GameManager::instance().setExtraColumns(GameManager::instance().extraColumns() + 1);
}

void AddColumnItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    // 绿色双竖条图标
    painter->setBrush(QBrush(QColor(80, 220, 120)));
    painter->setPen(QPen(Qt::darkGreen, 2));
    painter->drawRect(QRectF(-kSize, -kSize, kSize*2, kSize*2));
    painter->setPen(QPen(Qt::white, 2));
    painter->drawLine(QPointF(-3, -6), QPointF(-3, 6));
    painter->drawLine(QPointF(3, -6), QPointF(3, 6));
}

QString AddColumnItem::pickupMessage() const { return "Extra Column!"; }
