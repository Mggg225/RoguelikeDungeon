#include "Projectile.h"
#include "Enemy.h"
#include "Crate.h"
#include "Player.h"
#include <QGraphicsScene>
#include <QPainter>
#include <QtMath>
#include "GameManager.h"

Projectile::Projectile(QGraphicsScene *scene, const QPointF &pos,
                       qreal dx, qreal dy, int damage,
                       GameObject *source,
                       QGraphicsItem *parent)
    : GameObject(parent), m_dx(dx), m_dy(dy), m_damage(damage), m_source(source)
{
    setPos(pos);
    setZValue(2);

    // 仅玩家子弹使用精灵图（敌人子弹 m_source == nullptr）
    if (m_source != nullptr) {
        QStringList sprites = GameManager::instance().bulletSprites();
        if (!sprites.isEmpty()) {
            int idx = std::rand() % sprites.size();
            m_bulletPixmap.load(sprites[idx]);
        }
    }

    scene->addItem(this);
}

void Projectile::update(qreal dt)
{
    qreal newX = x() + m_dx * m_speed * dt;
    qreal newY = y() + m_dy * m_speed * dt;

    if (newX < 10 || newX > 790 || newY < 10 || newY > 590) {
        scene()->removeItem(this);
        deleteLater();
        return;
    }

    setPos(newX, newY);

    // 碰撞检测（跳过发射者）
    QList<QGraphicsItem*> items = collidingItems();
    for (auto *item : items) {
        auto *go = dynamic_cast<GameObject*>(item);
        if (!go || go == m_source) continue;

        // 敌人子弹打玩家
        auto *player = dynamic_cast<Player*>(go);
        if (player && m_source) {
            // 这是敌人的子弹打到玩家
            player->takeDamage(m_damage);
            scene()->removeItem(this);
            deleteLater();
            return;
        }

        // 玩家子弹打敌人
        auto *enemy = dynamic_cast<Enemy*>(go);
        if (enemy) {
            enemy->takeDamage(m_damage);
            if (enemy->isDead()) {
                enemy->triggerDeath();
            }
            scene()->removeItem(this);
            deleteLater();
            return;
        }

        // 子弹打箱子
        auto *crate = dynamic_cast<Crate*>(go);
        if (crate) {
            crate->takeDamage(m_damage);
            if (crate->isDestroyed()) {
                scene()->removeItem(crate);
                crate->deleteLater();
            }
            scene()->removeItem(this);
            deleteLater();
            return;
        }
    }
}

QRectF Projectile::boundingRect() const
{
    return QRectF(-kRadius, -kRadius, kRadius * 2, kRadius * 2);
}

void Projectile::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (!m_bulletPixmap.isNull()) {
        QRectF target(-kBulletDrawSize / 2.0, -kBulletDrawSize / 2.0, kBulletDrawSize, kBulletDrawSize);
        painter->drawPixmap(target.toRect(), m_bulletPixmap);
    } else {
        painter->setBrush(QBrush(Qt::white));
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(0, 0), kRadius, kRadius);
    }
}
