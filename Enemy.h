#pragma once
#include "Character.h"

class Player;
class QGraphicsScene;

class Enemy : public Character {
public:
    Enemy(QGraphicsScene *scene, QGraphicsItem *parent = nullptr);
    int type() const override { return QGraphicsItem::UserType + 5; }

    virtual bool isDying() const { return m_dying; }
    virtual void triggerDeath();

protected:
    QGraphicsScene *m_scene;
    bool m_dying = false;
    qreal m_deathTimer = 0.0;
};
