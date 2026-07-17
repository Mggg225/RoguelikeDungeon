#pragma once
#include "GameObject.h"

class Player;
class QGraphicsScene;

class Item : public GameObject {
public:
    Item(QGraphicsScene *scene, const QPointF &pos,
         QGraphicsItem *parent = nullptr);

    virtual void onPickup() = 0;
    virtual QString pickupMessage() const = 0;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override = 0;
    void update(qreal) override {}

protected:
    static constexpr qreal kSize = 10.0;
};
