#pragma once
#include "GameObject.h"
#include <QPixmap>

class QGraphicsScene;
class GameObject;

class Projectile : public GameObject {
public:
    Projectile(QGraphicsScene *scene, const QPointF &pos,
               qreal dx, qreal dy, int damage,
               GameObject *source = nullptr,
               QGraphicsItem *parent = nullptr);

    void update(qreal dt) override;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;
    int damage() const { return m_damage; }

protected:
    qreal m_dx, m_dy;
    int m_damage;
    QPixmap m_bulletPixmap;
    static constexpr int kBulletDrawSize = 16;
    qreal m_speed = 300.0;
    static constexpr qreal kRadius = 3.0;
    GameObject *m_source = nullptr;
};
