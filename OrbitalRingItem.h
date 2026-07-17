#pragma once
#include "Item.h"

class OrbitalRingItem : public Item {
public:
    OrbitalRingItem(QGraphicsScene *scene, const QPointF &pos);
    void onPickup() override;
    QString pickupMessage() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;
};
