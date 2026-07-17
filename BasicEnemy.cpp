#include "BasicEnemy.h"
#include "Player.h"
#include "GameManager.h"
#include "Wall.h"
#include "Crate.h"
#include <QGraphicsScene>
#include <QPainter>
#include <QtMath>

BasicEnemy::BasicEnemy(QGraphicsScene *scene, Player *player,
                       const QPointF &pos, QGraphicsItem *parent)
    : Enemy(scene, parent), m_player(player)
{
    m_hp = static_cast<int>(4 * GameManager::instance().enemyHPMult());
    setPos(pos);
    setZValue(1);

    m_movieIdle = new QMovie(":/assets/basicEnemy_idle.gif");
    m_movieRun  = new QMovie(":/assets/basicEnemy_run.gif");
    m_diePixmap.load(":/assets/basicEnemy_die.png");

    m_activeMovie = m_movieIdle;
    m_movieIdle->start();

    scene->addItem(this);
}

BasicEnemy::~BasicEnemy() {
    delete m_movieIdle;
    delete m_movieRun;
}

void BasicEnemy::update(qreal dt)
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

    // ----- 追踪 AI -----
    QPointF toPlayer = m_player->pos() - pos();
    qreal dist = qSqrt(toPlayer.x() * toPlayer.x() + toPlayer.y() * toPlayer.y());
    if (dist < 1) return;

    qreal dx = toPlayer.x() / dist;
    qreal dy = toPlayer.y() / dist;

    // 朝向（左右翻转）
    if (dx < -0.1)      m_facingLeft = true;
    else if (dx > 0.1)  m_facingLeft = false;

    // 动画切换
    bool moving = true;  // BasicEnemy 始终追踪
    if (moving && !m_wasMoving) {
        m_movieIdle->stop();
        m_activeMovie = m_movieRun;
        m_movieRun->start();
    }
    m_wasMoving = moving;

    qreal ox = x(), oy = y();
    qreal nx = ox + dx * m_speed * dt;
    qreal ny = oy + dy * m_speed * dt;

    // 滑动碰撞：先试 XY，撞了只试 X，撞了只试 Y，都撞才退回去
    setPos(nx, ny);
    auto hitsObstacle = [&](qreal tx, qreal ty) {
        setPos(tx, ty);
        for (auto *item : collidingItems()) {
            if (dynamic_cast<Wall*>(item) || dynamic_cast<Crate*>(item))
                return true;
        }
        return false;
    };
    if (hitsObstacle(nx, ny)) {
        if (!hitsObstacle(nx, oy)) {
            // X 方向通过，沿墙滑动
        } else if (!hitsObstacle(ox, ny)) {
            // Y 方向通过，沿墙滑动
        } else {
            // 两个方向都撞，退回
            setPos(ox, oy);
        }
    }

    // 接触伤害
    if (collidesWithItem(m_player)) {
        GameManager::instance().takeDamage(
            static_cast<int>(m_contactDamage * GameManager::instance().enemyDamageMult()));
    }
}

QRectF BasicEnemy::boundingRect() const
{
    return QRectF(-kColR, -kColR, kColR * 2, kColR * 2);
}

void BasicEnemy::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
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
