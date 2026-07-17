#include "SkillEffect.h"
#include "Player.h"
#include <QPainter>
#include <QGraphicsScene>

// 偏移量常量：直接改这里的数字就行
static constexpr qreal kBackOffsetX  = 5.0;
static constexpr qreal kBackOffsetY  = -35.0;
static constexpr qreal kFrontOffsetX = 0.0;
static constexpr qreal kFrontOffsetY = 25.0;
// ===== SkillLayer =====
SkillLayer::SkillLayer(Player *player, const QString &gifPath, int zValue,
                       qreal offsetX, qreal offsetY, QGraphicsItem *parent)
    : QGraphicsItem(parent), m_player(player), m_offsetX(offsetX), m_offsetY(offsetY)
{
    setZValue(zValue);
    m_movie = new QMovie(gifPath);
    m_movie->start();
    m_movie->jumpToFrame(0);
    QPointF pp = player->pos();
    setPos(pp.x() + m_offsetX, pp.y() + m_offsetY);
}

SkillLayer::~SkillLayer() { delete m_movie; }

QRectF SkillLayer::boundingRect() const
{
    return QRectF(-kSize / 2.0, -kSize / 2.0, kSize, kSize);
}

void SkillLayer::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QPixmap frame = m_movie->currentPixmap();
    if (!frame.isNull()) {
        QRectF target(-kSize / 2.0, -kSize / 2.0, kSize, kSize);
        painter->drawPixmap(target.toRect(), frame);
    }
}

// ===== SkillEffect =====
SkillEffect::SkillEffect(Player *player, QGraphicsScene *scene)
    : m_player(player), m_scene(scene) {}

SkillEffect::~SkillEffect() { clear(); }

void SkillEffect::start()
{
    clear();

    m_back  = new SkillLayer(m_player, ":/assets/skill_fire_1.gif", 1, kBackOffsetX, kBackOffsetY);
    m_front = new SkillLayer(m_player, ":/assets/skill_fire_2.gif", 1, kFrontOffsetX, kFrontOffsetY);
    m_scene->addItem(m_back);
    m_scene->addItem(m_front);

    m_playing = true;
}

void SkillEffect::stop()  { clear(); }
void SkillEffect::clear()
{
    if (m_back)  { m_scene->removeItem(m_back);  delete m_back;  m_back  = nullptr; }
    if (m_front) { m_scene->removeItem(m_front); delete m_front; m_front = nullptr; }
    m_playing = false;
}

void SkillEffect::update()
{
    if (!m_playing) return;
    QPointF pp = m_player->pos();
    if (m_back)  m_back->setPos(pp.x() + kBackOffsetX,  pp.y() + kBackOffsetY);
    if (m_front) m_front->setPos(pp.x() + kFrontOffsetX, pp.y() + kFrontOffsetY);
}
