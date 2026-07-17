#pragma once
#include "Item.h"

class FireRateUpItem : public Item {
public:
    FireRateUpItem(QGraphicsScene *scene, const QPointF &pos);
    void onPickup() override;
    QString pickupMessage() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;
};
