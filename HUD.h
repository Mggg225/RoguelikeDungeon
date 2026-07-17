#pragma once
#include <QGraphicsItem>
#include <QFont>
#include <QVector>

class QGraphicsScene;
class QGraphicsSceneMouseEvent;

class HUD : public QGraphicsItem {
public:
    explicit HUD(QGraphicsScene *scene, QGraphicsItem *parent = nullptr);

    void updateHUD();
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

    void showMessage(const QString &msg);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QGraphicsScene *m_scene;
    QString m_message;
    QFont m_font;

    // 小地图相关 — 缓存上一次 paint 时算出的房间格子位置，mousePressEvent 复用
    struct MapCell {
        int roomId;
        QRectF rect;  // 场景坐标下的矩形
        bool clickable;  // 已访问 = 可点击传送
    };
    QVector<MapCell> m_mapCells;
    QRectF m_mapBounds;  // 整个小地图背景区域（用于判断点击是否在小地图内）
};
