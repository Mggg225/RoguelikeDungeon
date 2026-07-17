#include "Enemy.h"
#include <QGraphicsScene>

Enemy::Enemy(QGraphicsScene *scene, QGraphicsItem *parent)
    : Character(parent), m_scene(scene)
{
}

void Enemy::triggerDeath()
{
    if (!m_dying) {
        m_dying = true;
        m_deathTimer = 0.5;  // 死亡动画持续 0.5 秒
    }
}
