#pragma once
#include "Projectile.h"

class EnemyProjectile : public Projectile {
public:
    EnemyProjectile(QGraphicsScene *scene, const QPointF &pos,
                    qreal dx, qreal dy, int damage,
                    QGraphicsItem *parent = nullptr);

    void update(qreal dt) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;
};
