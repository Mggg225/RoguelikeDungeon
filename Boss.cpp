#include "Boss.h"
#include "Player.h"
#include "EnemyProjectile.h"
#include "GameManager.h"
#include <QGraphicsScene>
#include <QPainter>
#include <QtMath>

static const QString kSpritePath = ":/assets/boss1.png";

Boss::Boss(QGraphicsScene *scene, Player *player,
           const QPointF &pos, QGraphicsItem *parent)
    : Enemy(scene, parent), m_player(player)
{
    m_maxHp = GameManager::instance().bossBaseHP();
    m_hp = m_maxHp;
    setPos(pos);
    setZValue(2);
    setScale(0.0);

    m_spriteLoaded = m_sprite.load(kSpritePath);
    if (!m_spriteLoaded) {
        qWarning("Boss sprite not found: %s", qPrintable(kSpritePath));
    }

    scene->addItem(this);
    GameManager::instance().setBossHP(m_hp);
    GameManager::instance().setBossMaxHP(m_maxHp);
}

void Boss::update(qreal dt)
{
    // 死亡动画
    if (m_dying) {
        m_deathTimer -= dt;
        if (m_deathTimer <= 0) {
            GameManager::instance().setBossHP(0);
            scene()->removeItem(this);
            deleteLater();
        }
        return;
    }

    // 出场动画阶段
    if (m_spawnTimer > 0) {
        m_spawnTimer -= dt;
        qreal t = 1.0 - m_spawnTimer / 1.2;
        // ease-out elastic-like: 快速涨到1.0然后稍微回弹
        m_spawnScale = 1.0 - qExp(-t * 6.0) * qCos(t * M_PI * 2);
        if (m_spawnScale > 1.2) m_spawnScale = 1.2;
        if (m_spawnTimer <= 0) {
            m_spawnScale = 1.0;
        }
        setScale(m_spawnScale);
        return; // 动画期间不行动
    }

    // 阶段检测
    if (m_hp <= m_maxHp / 2 && !m_phase2) {
        m_phase2 = true;
        m_speed = 160.0;
        m_fanCount = 5;
        m_ringTimer = 0.0;
    }

    // 追踪移动
    QPointF toPlayer = m_player->pos() - pos();
    qreal dist = qSqrt(toPlayer.x() * toPlayer.x() + toPlayer.y() * toPlayer.y());
    if (dist > 1) {
        qreal dx = toPlayer.x() / dist;
        qreal dy = toPlayer.y() / dist;
        setPos(x() + dx * m_speed * dt, y() + dy * m_speed * dt);
    }

    // 扇形发射
    m_fanTimer -= dt;
    qreal fanInterval = m_phase2 ? 1.5 : 2.5;
    if (m_fanTimer <= 0) {
        m_fanTimer = fanInterval;
        shootFan();
    }

    // 环形发射
    if (m_phase2) {
        m_ringTimer -= dt;
        if (m_ringTimer <= 0) {
            m_ringTimer = 3.0;
            shootRing();
        }
    }
}

void Boss::shootFan()
{
    QPointF toPlayer = m_player->pos() - pos();
    qreal baseAngle = qAtan2(toPlayer.y(), toPlayer.x());
    qreal spread = qDegreesToRadians(20.0);
    qreal startAngle = baseAngle - (m_fanCount - 1) * spread / 2.0;

    for (int i = 0; i < m_fanCount; ++i) {
        qreal angle = startAngle + i * spread;
        new EnemyProjectile(m_scene, pos(),
                       qCos(angle), qSin(angle), 1);
    }
}

void Boss::shootRing()
{
    const int count = 8;
    for (int i = 0; i < count; ++i) {
        qreal angle = 2.0 * M_PI * i / count;
        new EnemyProjectile(m_scene, pos(),
                       qCos(angle), qSin(angle), 1);
    }
}

QRectF Boss::boundingRect() const
{
    qreal hw = m_spriteLoaded ? kDrawW / 2.0 : 20.0;
    qreal hh = m_spriteLoaded ? kDrawH / 2.0 : 20.0;
    return QRectF(-hw, -hh, hw * 2, hh * 2);
}

void Boss::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (m_spawnTimer > 0) {
        qreal flash = 1.0 - m_spawnTimer / 1.2;
        painter->setOpacity(0.4 + 0.6 * flash);
        if (m_spriteLoaded) {
            painter->drawPixmap(QRect(-kDrawW/2, -kDrawH/2, kDrawW, kDrawH), m_sprite);
        }
        painter->setOpacity(1.0);
        int alpha = static_cast<int>(200 + 55 * qSin(flash * M_PI * 4));
        painter->setBrush(QBrush(QColor(255, 255, 200, alpha)));
        painter->setPen(QPen(Qt::white, 3));
        painter->drawEllipse(QPointF(0, 0), qMax(kDrawW, kDrawH) / 2 + 4, qMax(kDrawW, kDrawH) / 2 + 4);
        return;
    }

    if (m_spriteLoaded) {
        painter->drawPixmap(QRect(-kDrawW/2, -kDrawH/2, kDrawW, kDrawH), m_sprite);
    } else {
        painter->setBrush(QBrush(m_phase2 ? QColor(180, 40, 200) : QColor(140, 40, 160)));
        painter->setPen(QPen(Qt::darkMagenta, 3));
        painter->drawEllipse(QPointF(0, 0), 20, 20);
    }

    if (m_phase2) {
        painter->setBrush(QBrush(QColor(255, 0, 0, 50)));
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(0, 0), qMax(kDrawW, kDrawH) / 2, qMax(kDrawW, kDrawH) / 2);
    }
}

void Boss::takeDamage(int dmg)
{
    if (m_spawnTimer > 0) return; // 出场动画无敌
    Enemy::takeDamage(dmg);
    GameManager::instance().setBossHP(m_hp);
}
