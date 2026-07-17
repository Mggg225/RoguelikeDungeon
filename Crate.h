#pragma once
#include "GameObject.h"

class Crate : public GameObject {
public:
    Crate(const QPointF &pos, QGraphicsItem *parent = nullptr);

    void update(qreal) override {}
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

    void takeDamage(int dmg);
    bool isDestroyed() const { return m_hp <= 0; }
    int type() const override { return QGraphicsItem::UserType + 21; }

    static constexpr qreal kSize = 14.0;

private:
    int m_hp = 3;
};
