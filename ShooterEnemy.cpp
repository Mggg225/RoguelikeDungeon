#include "ShooterEnemy.h"
#include "Player.h"
#include "EnemyProjectile.h"
#include "Wall.h"
#include "Crate.h"
#include "GameManager.h"
#include <QGraphicsScene>
#include <QPainter>
#include <QtMath>
#include <QRandomGenerator>

ShooterEnemy::ShooterEnemy(QGraphicsScene *scene, Player *player,
                           const QPointF &pos, QGraphicsItem *parent)
    : Enemy(scene, parent), m_player(player)
{
    m_hp = static_cast<int>(4 * GameManager::instance().enemyHPMult());
    setPos(pos);
    setZValue(1);
    m_shootInterval = 1.0 + QRandomGenerator::global()->generateDouble() * 0.5;

    m_movieIdle   = new QMovie(":/assets/shooterEnemy_idle.gif");
    m_movieRun    = new QMovie(":/assets/shooterEnemy_run.gif");
    m_movieAttack = new QMovie(":/assets/shooterEnemy_attack.gif");
    m_diePixmap.load(":/assets/shooterEnemy_die.png");

    m_activeMovie = m_movieIdle;
    m_movieIdle->start();

    scene->addItem(this);
}

ShooterEnemy::~ShooterEnemy() {
    delete m_movieIdle;
    delete m_movieRun;
    delete m_movieAttack;
}

void ShooterEnemy::update(qreal dt)
{
    // ----- 死亡动画 -----
    if (m_dying) {
        m_deathTimer -= dt;
        if (m_deathTimer <= 0) {
            scene()->removeItem(this);
            deleteLater();
        }
        return;
    }

    // ----- 攻击闪计时 -----
    m_attackFlash -= dt;

    // ----- 保持距离 AI -----
    QPointF toPlayer = m_player->pos() - pos();
    qreal dist = qSqrt(toPlayer.x() * toPlayer.x() + toPlayer.y() * toPlayer.y());
    if (dist < 1) return;

    qreal dx = toPlayer.x() / dist;
    qreal dy = toPlayer.y() / dist;

    // 朝向
    if (dx < -0.1)      m_facingLeft = true;
    else if (dx > 0.1)  m_facingLeft = false;

    qreal ox = x(), oy = y();
    qreal sx = ox, sy = oy;
    if (dist < kPreferredDist - 20) {
        sx = ox - dx * m_speed * dt; sy = oy - dy * m_speed * dt;
    } else if (dist > kPreferredDist + 20) {
        sx = ox + dx * m_speed * dt; sy = oy + dy * m_speed * dt;
    }

    // 滑动碰撞：先试 XY，撞了只试 X，撞了只试 Y，都撞才退回去
    auto hitsObstacle = [&](qreal tx, qreal ty) {
        setPos(tx, ty);
        for (auto *item : collidingItems()) {
            if (dynamic_cast<Wall*>(item) || dynamic_cast<Crate*>(item))
                return true;
        }
        return false;
    };
    if (hitsObstacle(sx, sy)) {
        if (!hitsObstacle(sx, oy)) {
            // X 方向通过
        } else if (!hitsObstacle(ox, sy)) {
            // Y 方向通过
        } else {
            setPos(ox, oy);  // 都撞，退回
        }
    }

    // ----- 射击 -----
    m_shootTimer -= dt;
    if (m_shootTimer <= 0) {
        m_shootTimer = m_shootInterval;
        new EnemyProjectile(m_scene, pos(), dx, dy, 1);

        // 触发攻击动画
        m_attackFlash = 0.35;
        // 切换到攻击 GIF
        m_movieIdle->stop();
        m_movieRun->stop();
        m_activeMovie = m_movieAttack;
        m_movieAttack->start();
    }

    // ----- 动画切换 -----
    bool moving = (dist < kPreferredDist - 40 || dist > kPreferredDist + 40);

    if (m_attackFlash > 0) {
        // 攻击动画优先
    } else if (m_activeMovie == m_movieAttack) {
        // 攻击动画刚结束，切回正确的状态
        m_movieAttack->stop();
        if (moving) {
            m_activeMovie = m_movieRun;
            m_movieRun->start();
        } else {
            m_activeMovie = m_movieIdle;
            m_movieIdle->start();
        }
    } else if (moving != m_wasMoving) {
        // 移动状态改变：切 idle ↔ run
        if (moving) {
            m_movieIdle->stop();
            m_activeMovie = m_movieRun;
            m_movieRun->start();
        } else {
            m_movieRun->stop();
            m_activeMovie = m_movieIdle;
            m_movieIdle->start();
        }
    }
    m_wasMoving = moving;
}

QRectF ShooterEnemy::boundingRect() const
{
    return QRectF(-kColR, -kColR, kColR * 2, kColR * 2);
}

void ShooterEnemy::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    // ----- 死亡：显示 die.png -----
    if (m_dying) {
        if (!m_diePixmap.isNull()) {
            QRectF target(-kDrawSize / 2.0, -kDrawSize / 2.0, kDrawSize, kDrawSize);
            painter->drawPixmap(target.toRect(), m_diePixmap);
        }
        return;
    }

    // ----- GIF 动画（带水平翻转） -----
    if (m_activeMovie) {
        QPixmap frame = m_activeMovie->currentPixmap();
        if (!frame.isNull()) {
            QRectF target(-kDrawSize / 2.0, -kDrawSize / 2.0, kDrawSize, kDrawSize);
            if (m_facingLeft) {
                QPixmap flipped = frame.transformed(QTransform().scale(-1, 1),
                                                     Qt::SmoothTransformation);
                painter->drawPixmap(target.toRect(), flipped);
            } else {
                painter->drawPixmap(target.toRect(), frame);
            }
        }
    }
}
