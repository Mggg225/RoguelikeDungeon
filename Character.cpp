#include "Character.h"

Character::Character(QGraphicsItem *parent)
    : GameObject(parent)
{
}

void Character::takeDamage(int dmg)
{
    m_hp -= dmg;
    if (m_hp < 0) m_hp = 0;
}
