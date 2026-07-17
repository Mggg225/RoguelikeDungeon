#pragma once
#include <QGraphicsItem>
#include <QMovie>

class Player;
class QGraphicsScene;

class SkillLayer : public QGraphicsItem {
public:
    SkillLayer(Player *player, const QString &gifPath, int zValue,
               qreal offsetX = 0.0, qreal offsetY = 0.0, QGraphicsItem *parent = nullptr);
    ~SkillLayer() override;

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

private:
    Player *m_player;
    QMovie *m_movie;
    qreal m_offsetX;
    qreal m_offsetY;
    static constexpr int kSize = 80;
};

class SkillEffect {
public:
    SkillEffect(Player *player, QGraphicsScene *scene);
    ~SkillEffect();

    void start();        // 开始特效
    void stop();         // 停止并清除
    void update();       // 每帧跟随玩家
    bool isPlaying() const { return m_playing; }

private:
    void clear();
    Player *m_player;
    QGraphicsScene *m_scene;
    SkillLayer *m_back  = nullptr;
    SkillLayer *m_front = nullptr;
    bool m_playing = false;
};
