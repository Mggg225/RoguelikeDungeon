#pragma once
#include "GameObject.h"

class FloorBackground : public GameObject {
public:
    explicit FloorBackground(QGraphicsItem *parent = nullptr);
    void update(qreal) override {}
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;
    int type() const override { return QGraphicsItem::UserType + 30; }

private:
    static constexpr int kTileSize = 40;
    static constexpr int kRoomW = 800, kRoomH = 600;
};
