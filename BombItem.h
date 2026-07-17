#pragma once
#include "Item.h"

class QGraphicsScene;

class BombItem : public Item {
public:
    BombItem(QGraphicsScene *scene, const QPointF &pos);
    void onPickup() override;
    QString pickupMessage() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;
};
