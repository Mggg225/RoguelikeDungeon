#pragma once
#include "GameObject.h"

class Wall : public GameObject {
public:
    Wall(qreal x, qreal y, qreal w, qreal h, QGraphicsItem *parent = nullptr);

    void update(qreal) override {}
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

    int type() const override { return QGraphicsItem::UserType + 20; }

private:
    qreal m_w, m_h;
};
