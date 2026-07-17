#include "OrbitalRingItem.h"
#include "Player.h"
#include "GameManager.h"
#include <QPainter>

OrbitalRingItem::OrbitalRingItem(QGraphicsScene *scene, const QPointF &pos)
    : Item(scene, pos) {}

void OrbitalRingItem::onPickup()
{
    auto &gm = GameManager::instance();
    if (gm.attackMode() == 1) {
        // 已有环绕 → 切回直线
        gm.setAttackMode(0);
        gm.setAttackCooldown(0.35);
    } else {
        // 直线 → 环绕
        gm.setAttackMode(1);
        gm.setAttackCooldown(0.0);
    }
}

void OrbitalRingItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setBrush(QBrush(QColor(100, 180, 255)));
    painter->setPen(QPen(Qt::blue, 2));
    painter->drawEllipse(QPointF(0, 0), kSize, kSize);
    // 内部环
    painter->setPen(QPen(Qt::white, 1));
    painter->drawEllipse(QPointF(0, 0), kSize * 0.5, kSize * 0.5);
}
QString OrbitalRingItem::pickupMessage() const {
    return GameManager::instance().attackMode() == 1
        ? "Orbital Ring removed - back to linear!"
        : "Orbital Ring - orbiting bullets!";
}
