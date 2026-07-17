#pragma once
#include "GameObject.h"
#include <QMap>
#include <QPixmap>
class QGraphicsItem;
class QGraphicsScene;
class Player;

class OrbitalBullet : public GameObject {
public:
    OrbitalBullet(QGraphicsScene *scene, Player *player,
                  qreal startAngle, qreal orbitRadius, bool clockwise,
                  int damage,
                  QGraphicsItem *parent = nullptr);

    void update(qreal dt) override;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

    void setAngle(qreal a) { m_angle = a; }
    qreal bulletAngle() const { return m_angle; }

    int type() const override { return QGraphicsItem::UserType + 10; }

private:
    Player *m_player;
    qreal m_angle;
    qreal m_orbitRadius;
    bool m_clockwise;
    int m_damage;
    QPixmap m_bulletPixmap;
    QMap<QGraphicsItem*, qreal> m_hitCooldowns;
    static constexpr qreal kHitInterval = 0.3;
    static constexpr qreal kRotationSpeed = 150.0;
    static constexpr qreal kBulletRadius = 4.0;
    static constexpr int kBulletDrawSize = 16;
};
