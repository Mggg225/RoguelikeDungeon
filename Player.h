#pragma once
#include "Character.h"
#include "GameManager.h"
#include <QTimer>
#include <QPainter>
#include <QList>
#include <QMovie>
#include <QTransform>
#include <QPixmap>
#include <QSoundEffect>
#include <QDebug>

class QGraphicsScene;
class OrbitalBullet;
class SkillEffect;

class Player : public Character {
public:
    explicit Player(QGraphicsScene *scene, QGraphicsItem *parent = nullptr);
    ~Player() override;

    void update(qreal dt) override;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

    void clearOrbitals();
    void reloadSprites();  // 选择角色后重新加载精灵图

private:
    qreal m_speed = 200.0;
    static constexpr qreal kRadius = 12.0;
    static constexpr int kDrawSize = 48;
    qreal m_attackCooldown = 0.0;
    QGraphicsScene *m_scene;

    // 直线弹幕
    void shoot(qreal dx, qreal dy);
    // 扇形弹幕（mode=0：列数+子弹数联动）
    void shootFan(qreal dx, qreal dy);
    // 环形弹幕（mode=2上帝模式）
    void shootRing(int count);

    // 环绕弹幕（mode=1：自动永存，跨房间）
    QList<OrbitalBullet*> m_orbitalList;
    void manageOrbitals();
    void rebuildOrbitals();

    // 受伤闪烁
    qreal m_damageFlash = 0.0;
    int m_lastHP = 8;

    // GIF 动画
    QMovie *m_movieIdle = nullptr;
    QMovie *m_movieRun  = nullptr;
    QMovie *m_activeMovie = nullptr;
    bool m_facingLeft = false;
    bool m_wasMoving = false;

    // 死亡
    QPixmap m_diePixmap;
    bool m_dead = false;

    // 音效
    QSoundEffect *m_sfxShoot = nullptr;
    QSoundEffect *m_sfxMambo = nullptr;
    QSoundEffect *m_sfxCustom = nullptr;
    QSoundEffect *m_sfxSkill = nullptr;
    void playSfx();
    bool m_skillKeyWasDown = false;


    // 技能特效
    SkillEffect *m_skillEffect = nullptr;
};
