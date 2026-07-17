#include "Player.h"
#include "Enemy.h"
#include "InputManager.h"
#include "Item.h"
#include "Projectile.h"
#include "OrbitalBullet.h"
#include "Door.h"
#include "RoomManager.h"
#include "Wall.h"
#include "Crate.h"
#include "Portal.h"
#include "SkillEffect.h"
#include <QGraphicsScene>
#include <QtMath>
#include <cmath>
#include <QPointer>

Player::Player(QGraphicsScene *scene, QGraphicsItem *parent)
    : Character(parent), m_scene(scene)
{
    setPos(400, 300);
    setZValue(2);  // 确保在特效上层
    setFlag(QGraphicsItem::ItemIsFocusable);
    scene->addItem(this);

    m_movieIdle = new QMovie(GameManager::instance().spriteIdle());
    m_movieRun  = new QMovie(GameManager::instance().spriteRun());
    m_diePixmap.load(GameManager::instance().spriteDie());
    m_sfxSkill = new QSoundEffect(this);


    // 攻击音效
    m_sfxShoot = new QSoundEffect(this);
    m_sfxShoot->setSource(QUrl("qrc:/assets/sfx_shoot.wav"));
    m_sfxShoot->setVolume(1);

    m_sfxMambo = new QSoundEffect(this);
    m_sfxMambo->setSource(QUrl("qrc:/assets/sfx_mambo.wav"));
    m_sfxMambo->setVolume(1);

    m_sfxCustom = new QSoundEffect(this);
    m_sfxCustom->setVolume(1);

    m_skillEffect = new SkillEffect(this, m_scene);

    m_activeMovie = m_movieIdle;
    m_movieIdle->start();

    m_sfxSkill->setVolume(1);
}

Player::~Player() {
    delete m_movieIdle;
    delete m_movieRun;
}

void Player::reloadSprites()
{
    m_dead = false;

    // 停止并删除旧动画对象，重建
    if (m_movieIdle) { m_movieIdle->stop(); delete m_movieIdle; }
    if (m_movieRun)  { m_movieRun->stop();  delete m_movieRun; }

    m_movieIdle = new QMovie(GameManager::instance().spriteIdle());
    m_movieRun  = new QMovie(GameManager::instance().spriteRun());

    m_diePixmap = QPixmap(GameManager::instance().spriteDie());

    m_skillKeyWasDown = false;
    m_activeMovie = m_movieIdle;
    m_movieIdle->start();
}

void Player::update(qreal dt)
{
    // 死亡或转场期间冻结
    if (m_dead) return;
    if (GameManager::instance().isInTransition()) return;
    if (GameManager::instance().playerHP() <= 0) {
        m_dead = true;
        if (m_activeMovie) m_activeMovie->stop();
        return;
    }

    auto &input = InputManager::instance();
    qreal dx = 0, dy = 0;
    if (input.isKeyDown(Qt::Key_W)) dy -= 1;
    if (input.isKeyDown(Qt::Key_S)) dy += 1;
    if (input.isKeyDown(Qt::Key_A)) dx -= 1;
    if (input.isKeyDown(Qt::Key_D)) dx += 1;

    // 动画 + 方向
    bool moving = (dx != 0 || dy != 0);
    if (input.isKeyDown(Qt::Key_A) && !input.isKeyDown(Qt::Key_D))
        m_facingLeft = true;
    else if (input.isKeyDown(Qt::Key_D) && !input.isKeyDown(Qt::Key_A))
        m_facingLeft = false;

    if (moving && !m_wasMoving) {
        m_movieIdle->stop();
        m_activeMovie = m_movieRun;
        m_movieRun->start();
    } else if (!moving && m_wasMoving) {
        m_movieRun->stop();
        m_activeMovie = m_movieIdle;
        m_movieIdle->start();
    }
    m_wasMoving = moving;

    // 归一化
    if (moving) {
        qreal len = qSqrt(dx * dx + dy * dy);
        dx /= len; dy /= len;
    }

    // 移动
    qreal speed = GameManager::instance().godMode() ? m_speed * 1.5 : m_speed;
    speed *= GameManager::instance().playerSpeedMult();
    qreal newX = x() + dx * speed * dt;
    qreal newY = y() + dy * speed * dt;
    newX = qBound(kRadius + 10.0, newX, 800.0 - kRadius - 10.0);
    newY = qBound(kRadius + 10.0, newY, 600.0 - kRadius - 10.0);
    setPos(newX, newY);

    // 碰撞推开
    for (auto *item : collidingItems()) {
        auto *wall = dynamic_cast<Wall*>(item);
        auto *crate = dynamic_cast<Crate*>(item);
        if (wall || crate) {
            QRectF ob = wall ? wall->sceneBoundingRect() : crate->sceneBoundingRect();
            QRectF pb = sceneBoundingRect();
            qreal overlapX = std::min(pb.right(), ob.right()) - std::max(pb.left(), ob.left());
            qreal overlapY = std::min(pb.bottom(), ob.bottom()) - std::max(pb.top(), ob.top());
            if (overlapX < overlapY) {
                if (pb.center().x() < ob.center().x()) newX -= overlapX;
                else newX += overlapX;
            } else {
                if (pb.center().y() < ob.center().y()) newY -= overlapY;
                else newY += overlapY;
            }
        }
    }
    setPos(newX, newY);

    // 道具拾取
    Item *nearbyItem = nullptr;
    for (auto *item : m_scene->items()) {
        auto *pickup = dynamic_cast<Item*>(item);
        if (pickup && collidesWithItem(pickup)) { nearbyItem = pickup; break; }
    }
    GameManager::instance().setNearbyItem(nearbyItem ? nearbyItem->pickupMessage() : "");
    if (nearbyItem && InputManager::instance().isKeyDown(Qt::Key_G)) {
        nearbyItem->onPickup();
        GameManager::instance().showPickupMsg(nearbyItem->pickupMessage());
        m_scene->removeItem(nearbyItem);
        nearbyItem->deleteLater();
        GameManager::instance().setNearbyItem("");
    }

    // 传送门碰撞（Boss 房进入下一层）
    for (auto *item : collidingItems()) {
        if (dynamic_cast<Portal*>(item)) {
            RoomManager::instance().nextFloor();
            break;
        }
    }

    // 门碰撞
    QList<QGraphicsItem*> hits = collidingItems();
    for (auto *item : hits) {
        auto *door = dynamic_cast<Door*>(item);
        if (door && !door->isLocked()) { RoomManager::instance().goThroughDoor(door->direction()); break; }
    }

    // ===== 攻击 =====
    int mode = GameManager::instance().attackMode();
    if (mode == 1) {
        // 环绕弹幕：自动维护，不需要按键
        manageOrbitals();
    } else {
        // 直线/上帝模式：方向键触发
        m_attackCooldown -= dt;
        if (m_attackCooldown <= 0) {
            bool anyArrow = input.isKeyDown(Qt::Key_Up)   || input.isKeyDown(Qt::Key_Down)
                         || input.isKeyDown(Qt::Key_Left) || input.isKeyDown(Qt::Key_Right);
            if (anyArrow) {
                if (mode == 0) {
                    if (input.isKeyDown(Qt::Key_Up))    shootFan(0, -1);
                    else if (input.isKeyDown(Qt::Key_Down))  shootFan(0,  1);
                    else if (input.isKeyDown(Qt::Key_Left))  shootFan(-1, 0);
                    else if (input.isKeyDown(Qt::Key_Right)) shootFan( 1, 0);
                } else if (mode == 2) {
                    shootRing(18);
                }
                m_attackCooldown = GameManager::instance().attackCooldown();
            }
        }
    }

    // 技能激活 — 空格上升沿触发
    bool spaceDown = input.isKeyDown(Qt::Key_Space);
    if (spaceDown && !m_skillKeyWasDown) {
        bool activated = GameManager::instance().activateSkill(5, 8);
        if (activated) {
            m_skillEffect->start();
            QString sfxPath = GameManager::instance().skillSfxPath();
            if(!sfxPath.isEmpty()){

                m_sfxSkill->setSource(QUrl("qrc" + sfxPath));
                m_sfxSkill->play();
            }
        }

    }
    m_skillKeyWasDown = spaceDown;

    // 特效跟随 + 结束时停止
    if (m_skillEffect->isPlaying()) {
        m_skillEffect->update();
        if (!GameManager::instance().skillActive()) m_skillEffect->stop();
    }

    // 炸弹
    if (input.isKeyDown(Qt::Key_1) && GameManager::instance().hasBomb()
        && GameManager::instance().bombCooldown() <= 0) {
        QPointF bombPos = pos();
        auto *bombMarker = new QGraphicsEllipseItem(-12, -12, 24, 24);
        bombMarker->setPos(bombPos);
        bombMarker->setPen(QPen(Qt::red, 3));
        bombMarker->setBrush(QBrush(QColor(40, 40, 40)));
        bombMarker->setZValue(10);
        m_scene->addItem(bombMarker);
        QTimer::singleShot(2000, [bombPos, bombMarker, this]() {
            for (auto *item : m_scene->items()) {
                auto *enemy = dynamic_cast<Enemy*>(item);
                if (enemy) {
                    QPointF delta = enemy->pos() - bombPos;
                    if (delta.x()*delta.x() + delta.y()*delta.y() <= 120*120) {
                        enemy->takeDamage(GameManager::instance().attackDamage() * 3);
                        if (enemy->isDead()) enemy->triggerDeath();
                    }
                }
            }
            m_scene->removeItem(bombMarker);
            delete bombMarker;
        });
        GameManager::instance().setBombCooldown(5.0);
    }
    GameManager::instance().tickBombCooldown(dt);

    // 受伤闪烁
    int currentHP = GameManager::instance().playerHP();
    if (currentHP < m_lastHP) m_damageFlash = 0.2;
    m_lastHP = currentHP;
    m_damageFlash -= dt;
}

// ===== 扇形弹幕：列数 × 每列子弹数 =====
static void emitFan(Player *p, qreal dx, qreal dy)
{
    int damage = GameManager::instance().attackDamage();
    int columns = 1 + GameManager::instance().extraColumns();
    int bulletsPerCol = 1 + GameManager::instance().extraBullets();
    qreal baseAngle = qAtan2(dy, dx);
    qreal spread = (columns - 1) * qDegreesToRadians(10.0);
    for (int c = 0; c < columns; ++c) {
        qreal colAngle = baseAngle;
        if (columns > 1)
            colAngle = baseAngle - spread / 2.0 + c * qDegreesToRadians(10.0);
        for (int b = 0; b < bulletsPerCol; ++b) {
            qreal bulletAngle = colAngle;
            if (bulletsPerCol > 1) {
                qreal ms = (bulletsPerCol - 1) * qDegreesToRadians(5.0);
                bulletAngle = colAngle - ms / 2.0 + b * qDegreesToRadians(5.0);
            }
            new Projectile(p->scene(), p->pos(), qCos(bulletAngle), qSin(bulletAngle), damage, p);
        }
    }
}

void Player::shootFan(qreal dx, qreal dy)
{
    emitFan(this, dx, dy);
    if (GameManager::instance().doubleAttack()) {
        QPointer<Player> self(this);
        QTimer::singleShot(100, this, [self, dx, dy]() {
            if (self) emitFan(self, dx, dy);
        });
    }
    playSfx();
}

void Player::shoot(qreal dx, qreal dy)
{
    new Projectile(m_scene, pos(), dx, dy, GameManager::instance().attackDamage(), this);
    if (GameManager::instance().doubleAttack()) {
        QPointer<Player> self(this);
        QTimer::singleShot(100, this, [self, dx, dy]() {
            if (self) new Projectile(self->m_scene, self->pos(), dx, dy,
                                     GameManager::instance().attackDamage(), self);
        });
    }
    playSfx();
}

void Player::shootRing(int count)
{
    for (int i = 0; i < count; ++i) {
        qreal angle = 2.0 * M_PI * i / count;
        new Projectile(m_scene, pos(), qCos(angle), qSin(angle),
                       GameManager::instance().attackDamage(), this);
    }
    if (GameManager::instance().doubleAttack()) {
        QPointer<Player> self(this);
        QTimer::singleShot(100, this, [self, count]() {
            if (!self) return;
            for (int i = 0; i < count; ++i) {
                qreal angle = 2.0 * M_PI * i / count;
                new Projectile(self->m_scene, self->pos(), qCos(angle), qSin(angle),
                               GameManager::instance().attackDamage(), self);
            }
        });
    }
    playSfx();
}

// ===== 环绕弹幕管理 =====
void Player::manageOrbitals()
{
    int rings = 1 + GameManager::instance().extraColumns();
    if (GameManager::instance().doubleAttack()) rings++;  // 技能：加一圈
    int bulletsPerRing = 6 + GameManager::instance().extraBullets();
    int targetCount = rings * bulletsPerRing;

    if (m_orbitalList.size() != targetCount) {
        rebuildOrbitals();
        return;
    }
}

void Player::rebuildOrbitals()
{
    for (auto *b : m_orbitalList) {
        if (b->scene()) b->scene()->removeItem(b);
        delete b;
    }
    m_orbitalList.clear();

    int rings = 1 + GameManager::instance().extraColumns();
    if (GameManager::instance().doubleAttack()) rings++;
    int bulletsPerRing = 6 + GameManager::instance().extraBullets();
    int damage = GameManager::instance().attackDamage() * 2;  // 环绕弹幕伤害×2

    for (int r = 0; r < rings; ++r) {
        qreal radius = 70.0 + r * 30.0;
        bool clockwise = (r % 2 == 0);
        qreal stagger = (r % 2 == 0) ? 0.0 : M_PI / bulletsPerRing;

        for (int b = 0; b < bulletsPerRing; ++b) {
            qreal angle = 2.0 * M_PI * b / bulletsPerRing + stagger;
            auto *bullet = new OrbitalBullet(m_scene, this, angle, radius, clockwise, damage);
            m_orbitalList.append(bullet);
        }
    }

    GameManager::instance().setAttackCooldown(0.0);
}

void Player::playSfx()
{
    int choice = GameManager::instance().sfxChoice();
    if (choice == 2) {
        QString path = GameManager::instance().customSfxPath();
        if (!path.isEmpty()) {
            m_sfxCustom->setSource(QUrl::fromLocalFile(path));
            m_sfxCustom->play();
            return;
        }
        // 没选文件，回退到默认
        m_sfxShoot->play();
    } else if (choice == 0) {
        m_sfxShoot->play();
    } else {
        m_sfxMambo->play();
    }
}

void Player::clearOrbitals()
{
    for (auto *b : m_orbitalList) {
        if (b->scene()) b->scene()->removeItem(b);
        delete b;
    }
    m_orbitalList.clear();
    m_dead = false;  // 新游戏重置死亡状态
}

// ===== 绘制 =====
QRectF Player::boundingRect() const
{
    return QRectF(-kRadius, -kRadius, kRadius * 2, kRadius * 2);
}

void Player::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (m_dead) {
        if (!m_diePixmap.isNull()) {
            QRectF target(-kDrawSize / 2.0, -kDrawSize / 2.0, kDrawSize, kDrawSize);
            painter->drawPixmap(target.toRect(), m_diePixmap);
        }
        return;
    }

    // 冷却弧形圈（仅直线/上帝模式）
    int mode = GameManager::instance().attackMode();
    if (mode == 0 || mode == 2) {
        qreal maxCD = GameManager::instance().attackCooldown();
        qreal progress = 1.0 - (m_attackCooldown / qMax(maxCD, 0.001));
        painter->setPen(QPen(QColor(255, 255, 255, 180), 2));
        int spanAngle = static_cast<int>(progress * 360 * 16);
        if (spanAngle > 0) {
            painter->drawArc(QRectF(-kRadius - 6, -kRadius - 6, (kRadius+6)*2, (kRadius+6)*2),
                             90 * 16, -spanAngle);
        }
    }

    // GIF
    if (m_activeMovie) {
        QPixmap frame = m_activeMovie->currentPixmap();
        if (!frame.isNull()) {
            QRectF target(-kDrawSize / 2.0, -kDrawSize / 2.0, kDrawSize, kDrawSize);
            if (m_facingLeft) {
                QPixmap flipped = frame.transformed(QTransform().scale(-1, 1), Qt::SmoothTransformation);
                painter->drawPixmap(target.toRect(), flipped);
            } else {
                painter->drawPixmap(target.toRect(), frame);
            }
        } else {
            painter->setBrush(QBrush(QColor(60, 200, 60)));
            painter->setPen(QPen(Qt::darkGreen, 2));
            painter->drawEllipse(QPointF(0, 0), kRadius, kRadius);
        }
    }

    auto &gm = GameManager::instance();
    if (gm.godMode()) {
        painter->setPen(QPen(QColor(255, 200, 40, 180), 3));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(QPointF(0, 0), kRadius + 5, kRadius + 5);
    }
    if (gm.isInvincible()) {
        int alpha = static_cast<int>(std::abs(std::sin(gm.playerHP() * 10 + m_attackCooldown * 20)) * 100 + 80);
        painter->setBrush(QBrush(QColor(255, 255, 255, alpha)));
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(0, 0), kRadius + 4, kRadius + 4);
    } else if (m_damageFlash > 0) {
        painter->setBrush(QBrush(QColor(255, 40, 40, 120)));
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(0, 0), kRadius + 3, kRadius + 3);
    }
}
