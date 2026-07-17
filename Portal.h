#pragma once
#include "GameObject.h"
#include <QPixmap>

class Portal : public GameObject {
public:
    explicit Portal(QGraphicsItem *parent = nullptr);

    void update(qreal) override {}
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;
    int type() const override { return QGraphicsItem::UserType + 40; }

private:
    QPixmap m_sprite;
    static constexpr int kSize = 32;
};
