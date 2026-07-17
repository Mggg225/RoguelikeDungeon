#include "FloorBackground.h"
#include <QPainter>

FloorBackground::FloorBackground(QGraphicsItem *parent)
    : GameObject(parent)
{
    setPos(400, 300);
    setZValue(-10);
}

QRectF FloorBackground::boundingRect() const
{
    return QRectF(-400, -300, 800, 600);
}

void FloorBackground::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    const int ts = kTileSize;
    const QColor dark(55, 50, 45);
    const QColor light(68, 62, 55);
    const QColor grout(40, 36, 32);

    // 绘制地板砖
    for (int row = 0; row < 600; row += ts) {
        for (int col = 0; col < 800; col += ts) {
            QColor tile = ((row / ts + col / ts) % 2 == 0) ? dark : light;
            painter->setBrush(QBrush(tile));
            painter->setPen(QPen(grout, 1));
            painter->drawRect(QRectF(col - 400, row - 300, ts, ts));
        }
    }

    // 四周暗角
    QLinearGradient edgeL(0, 0, 60, 0);
    edgeL.setColorAt(0, QColor(0, 0, 0, 80));
    edgeL.setColorAt(1, QColor(0, 0, 0, 0));
    painter->fillRect(QRectF(-400, -300, 60, 600), edgeL);

    QLinearGradient edgeR(740, 0, 800, 0);
    edgeR.setColorAt(0, QColor(0, 0, 0, 0));
    edgeR.setColorAt(1, QColor(0, 0, 0, 80));
    painter->fillRect(QRectF(340, -300, 60, 600), edgeR);

    QLinearGradient edgeT(0, 0, 0, 60);
    edgeT.setColorAt(0, QColor(0, 0, 0, 80));
    edgeT.setColorAt(1, QColor(0, 0, 0, 0));
    painter->fillRect(QRectF(-400, -300, 800, 60), edgeT);

    QLinearGradient edgeB(0, 540, 0, 600);
    edgeB.setColorAt(0, QColor(0, 0, 0, 0));
    edgeB.setColorAt(1, QColor(0, 0, 0, 80));
    painter->fillRect(QRectF(-400, 240, 800, 60), edgeB);
}
