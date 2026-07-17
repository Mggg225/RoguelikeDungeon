#include "OrbitalBullet.h"
#include "Enemy.h"
#include "Crate.h"
#include "Player.h"
#include "GameManager.h"
#include <QGraphicsScene>
#include <QPainter>
#include <QtMath>

OrbitalBullet::OrbitalBullet(QGraphicsScene *scene, Player *player,
                             qreal startAngle, qreal orbitRadius, bool clockwise,
                             int damage, QGraphicsItem *parent)
    : GameObject(parent), m_player(player),
      m_angle(startAngle), m_orbitRadius(orbitRadius),
      m_clockwise(clockwise), m_damage(damage)
{
    setZValue(3);

    // 随机选精灵图
    QStringList sprites = GameManager::instance().bulletSprites();
    if (!sprites.isEmpty()) {
        int idx = std::rand() % sprites.size();
        m_bulletPixmap.load(sprites[idx]);
    }

    // 设置初始位置
    QPointF pp = m_player->pos();
    setPos(pp.x() + qCos(m_angle) * m_orbitRadius,
           pp.y() + qSin(m_angle) * m_orbitRadius);
    scene->addItem(this);
}

void OrbitalBullet::update(qreal dt)
{
    // 旋转方向
    if (m_clockwise)
        m_angle += qDegreesToRadians(kRotationSpeed) * dt;
    else
        m_angle -= qDegreesToRadians(kRotationSpeed) * dt;

    // 位置 = 玩家位置 + 角度偏移
    QPointF playerPos = m_player->pos();
    qreal px = playerPos.x() + qCos(m_angle) * m_orbitRadius;
    qreal py = playerPos.y() + qSin(m_angle) * m_orbitRadius;
    setPos(px, py);

    // 穿透型碰撞（每目标 0.3s 命中冷却）
    for (auto it = m_hitCooldowns.begin(); it != m_hitCooldowns.end(); ) {
        it.value() -= dt;
        if (it.value() <= 0) it = m_hitCooldowns.erase(it);
        else ++it;
    }

    QList<QGraphicsItem*> hits = collidingItems();
    for (auto *item : hits) {
        if (m_hitCooldowns.contains(item)) continue;

        auto *enemy = dynamic_cast<Enemy*>(item);
        if (enemy) {
            enemy->takeDamage(m_damage);
            m_hitCooldowns[item] = kHitInterval;
            if (enemy->isDead()) {
                m_hitCooldowns.remove(item);
                enemy->triggerDeath();
            }
            continue;
        }
        auto *crate = dynamic_cast<Crate*>(item);
        if (crate) {
            crate->takeDamage(m_damage);
            m_hitCooldowns[item] = kHitInterval;
            if (crate->isDestroyed()) {
                m_hitCooldowns.remove(item);
                scene()->removeItem(crate);
                crate->deleteLater();
            }
        }
    }
}

QRectF OrbitalBullet::boundingRect() const
{
    return QRectF(-kBulletRadius, -kBulletRadius, kBulletRadius * 2, kBulletRadius * 2);
}

void OrbitalBullet::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    // 精灵图模式
    if (!m_bulletPixmap.isNull()) {
        QRectF target(-kBulletDrawSize / 2.0, -kBulletDrawSize / 2.0,
                      kBulletDrawSize, kBulletDrawSize);
        painter->drawPixmap(target.toRect(), m_bulletPixmap);
        return;
    }

    // 回退：蓝色弹幕 + 拖尾
    qreal tailDir = m_clockwise ? -1.0 : 1.0;
    painter->setPen(QPen(QColor(80, 80, 255, 120), 1.5));
    painter->drawLine(
        QPointF(qCos(m_angle) * tailDir * 6, qSin(m_angle) * tailDir * 6),
        QPointF(qCos(m_angle) * tailDir * 2, qSin(m_angle) * tailDir * 2)
    );
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(QColor(100, 140, 255)));
    painter->drawEllipse(QPointF(0, 0), kBulletRadius, kBulletRadius);
}
