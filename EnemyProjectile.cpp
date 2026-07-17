#include "EnemyProjectile.h"
#include "Player.h"
#include "GameManager.h"
#include <QGraphicsScene>
#include <QPainter>
#include <QtMath>

EnemyProjectile::EnemyProjectile(QGraphicsScene *scene, const QPointF &pos,
                                 qreal dx, qreal dy, int damage,
                                 QGraphicsItem *parent)
    : Projectile(scene, pos, dx, dy, damage, nullptr, parent)
{
    setZValue(2);
}

void EnemyProjectile::update(qreal dt)
{
    qreal newX = x() + m_dx * m_speed * dt;
    qreal newY = y() + m_dy * m_speed * dt;

    if (newX < 10 || newX > 790 || newY < 10 || newY > 590) {
        scene()->removeItem(this);
        deleteLater();
        return;
    }

    setPos(newX, newY);

    // 只碰撞玩家
    QList<QGraphicsItem*> hits = collidingItems();
    for (auto *item : hits) {
        auto *player = dynamic_cast<Player*>(item);
        if (player) {
            GameManager::instance().takeDamage(m_damage);
            scene()->removeItem(this);
            deleteLater();
            return;
        }
    }
}

void EnemyProjectile::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setBrush(QBrush(QColor(255, 40, 40)));
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(QPointF(0, 0), 5.0, 5.0);
}
