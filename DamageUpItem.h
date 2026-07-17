#pragma once
#include "Item.h"

class DamageUpItem : public Item {
public:
    DamageUpItem(QGraphicsScene *scene, const QPointF &pos);
    void onPickup() override;
    QString pickupMessage() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;
};
