#pragma once
#include "Enemy.h"
#include <QPixmap>
#include <QMovie>

class Player;

class BasicEnemy : public Enemy {
public:
    BasicEnemy(QGraphicsScene *scene, Player *player,
               const QPointF &pos, QGraphicsItem *parent = nullptr);
    ~BasicEnemy() override;

    void update(qreal dt) override;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

private:
    Player *m_player;
    qreal m_speed = 120.0;
    int m_contactDamage = 1;
    static constexpr qreal kColR = 14.0;
    static constexpr int kDrawSize = 36;

    QMovie *m_movieIdle = nullptr;
    QMovie *m_movieRun  = nullptr;
    QMovie *m_activeMovie = nullptr;
    QPixmap m_diePixmap;
    bool m_facingLeft = false;
    bool m_wasMoving = false;
};
