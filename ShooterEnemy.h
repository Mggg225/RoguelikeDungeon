#pragma once
#include "Enemy.h"
#include <QPixmap>
#include <QMovie>

class Player;
class QGraphicsScene;

class ShooterEnemy : public Enemy {
public:
    ShooterEnemy(QGraphicsScene *scene, Player *player,
                 const QPointF &pos, QGraphicsItem *parent = nullptr);
    ~ShooterEnemy() override;

    void update(qreal dt) override;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

private:
    Player *m_player;
    qreal m_speed = 80.0;
    qreal m_shootTimer = 0.0;
    qreal m_shootInterval = 1.5;
    static constexpr qreal kColR = 14.0;
    static constexpr int kDrawSize = 36;
    static constexpr qreal kPreferredDist = 180.0;

    // GIF 动画
    QMovie *m_movieIdle   = nullptr;
    QMovie *m_movieRun    = nullptr;
    QMovie *m_movieAttack = nullptr;
    QMovie *m_activeMovie = nullptr;

    // 死亡图片 + 攻击闪
    QPixmap m_diePixmap;
    qreal m_attackFlash = 0.0;   // >0 时显示攻击动画
    bool m_facingLeft = false;
    bool m_wasMoving = false;
};
