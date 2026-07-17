#pragma once
#include <QGraphicsItem>
#include <QObject>

class GameObject : public QObject, public QGraphicsItem {
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    explicit GameObject(QGraphicsItem *parent = nullptr);
    virtual ~GameObject() = default;

    virtual void update(qreal dt) = 0;

    // boundingRect 和 paint 由子类实现
    QRectF boundingRect() const override = 0;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override = 0;
};
