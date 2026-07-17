#pragma once
#include "GameObject.h"

class Character : public GameObject {
public:
    explicit Character(QGraphicsItem *parent = nullptr);
    int hp() const { return m_hp; }
    virtual void takeDamage(int dmg);
    bool isDead() const { return m_hp <= 0; }

protected:
    int m_hp = 1;
};
