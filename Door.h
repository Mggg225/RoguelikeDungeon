#pragma once
#include "GameObject.h"

class Door : public GameObject {
public:
    enum Direction { Up, Down, Left, Right };

    Door(Direction dir, int targetRoomId,
         QGraphicsItem *parent = nullptr);

    void update(qreal) override {}
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

    Direction direction() const { return m_direction; }
    int targetRoom() const { return m_targetRoom; }
    bool isLocked() const { return m_locked; }
    void setLocked(bool locked) { m_locked = locked; }
    void setOpen(bool open) { m_open = open; }
    bool isOpen() const { return m_open; }

private:
    Direction m_direction;
    int m_targetRoom;
    bool m_locked = false;
    bool m_open = true;
    int doorW() const { return (m_direction == Up || m_direction == Down) ? 40 : 12; }
    int doorH() const { return (m_direction == Up || m_direction == Down) ? 12 : 40; }
};
