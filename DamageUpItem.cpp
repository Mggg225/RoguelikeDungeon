#include "DamageUpItem.h"
#include "GameManager.h"
#include <QPainter>

DamageUpItem::DamageUpItem(QGraphicsScene *scene, const QPointF &pos)
    : Item(scene, pos) {}

void DamageUpItem::onPickup()
{
    int current = GameManager::instance().attackDamage();
    GameManager::instance().setAttackDamage(static_cast<int>(current * GameManager::instance().damageItemMult()));
}

void DamageUpItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setBrush(QBrush(QColor(255, 80, 80)));
    painter->setPen(QPen(Qt::red, 2));
    painter->drawRect(QRectF(-kSize, -kSize, kSize*2, kSize*2));
    painter->setPen(QPen(Qt::white, 2));
    painter->drawText(boundingRect(), Qt::AlignCenter, "+");
}
QString DamageUpItem::pickupMessage() const {
    return QString("Damage Up x%1!").arg(GameManager::instance().damageItemMult(), 0, 'f', 1);
}
