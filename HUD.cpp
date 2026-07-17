#include "HUD.h"
#include "GameManager.h"
#include "RoomManager.h"
#include <QPainter>
#include <QFont>
#include <QGraphicsSceneMouseEvent>

HUD::HUD(QGraphicsScene *scene, QGraphicsItem *parent)
    : QGraphicsItem(parent), m_scene(scene)
{
    setZValue(100);
    setAcceptedMouseButtons(Qt::LeftButton);  // 接收鼠标点击（小地图传送）
    m_font = QFont("monospace", 12);
}

void HUD::updateHUD()
{
    update();
}

QRectF HUD::boundingRect() const
{
    return QRectF(0, 0, 800, 600);
}

void HUD::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    auto &gm = GameManager::instance();

    // ========== 玩家血条（左上） ==========
    painter->setFont(m_font);
    painter->setPen(Qt::white);

    int hp = gm.playerHP();
    int maxHp = gm.maxHP();
    int barWidth = 200;
    int barHeight = 16;
    int barX = 15, barY = 10;

    painter->setBrush(QBrush(QColor(40, 40, 40, 180)));
    painter->drawRect(barX, barY, barWidth, barHeight);
    painter->setBrush(QBrush(QColor(200, 30, 30, 200)));
    painter->drawRect(barX, barY, barWidth * hp / maxHp, barHeight);
    painter->setPen(Qt::white);
    painter->drawText(QRectF(barX, barY, barWidth, barHeight), Qt::AlignCenter,
                      QString("HP: %1/%2").arg(hp).arg(maxHp));

    // ========== 技能状态（HP 条下方） ==========
    if (gm.skillActive()) {
        painter->setFont(QFont("monospace", 9));
        painter->setPen(QColor(100, 220, 255));
        painter->drawText(QRectF(15, 28, 200, 14), Qt::AlignLeft,
                          QString("Skill Active  %1s").arg(gm.skillTimer(), 0, 'f', 1));
    } else if (gm.skillCooldown() > 0) {
        painter->setFont(QFont("monospace", 9));
        painter->setPen(QColor(160, 160, 160));
        painter->drawText(QRectF(15, 28, 200, 14), Qt::AlignLeft,
                          QString("CD: %1s").arg(gm.skillCooldown(), 0, 'f', 1));
    }

    // ========== Boss 血条（顶部居中，仅 Boss 房间显示） ==========
    if (gm.bossMaxHP() > 0 && gm.bossHP() > 0) {
        int bBarW = 300, bBarH = 12;
        int bBarX = 250, bBarY = 8;
        int bHp = gm.bossHP();
        int bMax = gm.bossMaxHP();

        painter->setBrush(QBrush(QColor(20, 20, 20, 200)));
        painter->setPen(QPen(QColor(180, 60, 60), 1));
        painter->drawRoundedRect(QRectF(bBarX, bBarY, bBarW, bBarH), 3, 3);
        painter->setBrush(QBrush(QColor(180, 40, 180)));
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(QRectF(bBarX, bBarY, bBarW * bHp / bMax, bBarH), 3, 3);
        painter->setFont(QFont("monospace", 9));
        painter->setPen(Qt::white);
        painter->drawText(QRectF(bBarX, bBarY, bBarW, bBarH), Qt::AlignCenter,
                          QString("BOSS  %1/%2").arg(bHp).arg(bMax));
    }

    // ========== 小地图 (右上角) ==========
    const int mapX = 600, mapY = 5;
    const int cellSize = 22;
    const int mapW = 180, mapH = 120;

    // 背景
    painter->setBrush(QBrush(QColor(20, 20, 20, 200)));
    painter->setPen(QPen(QColor(80, 80, 80), 1));
    const QRectF mapBg(mapX, mapY, mapW, mapH);
    m_mapBounds = mapBg;
    painter->drawRoundedRect(mapBg, 4, 4);

    auto &rm = RoomManager::instance();
    const auto &rooms = rm.rooms();
    int curId = rm.currentRoomId();

    // 计算可视范围（只用 revealed 的房间）
    int minGX = 99, maxGX = -1, minGY = 99, maxGY = -1;
    for (const auto &r : rooms) {
        if (!rm.isRoomRevealed(r.id)) continue;
        if (r.gridX < minGX) minGX = r.gridX;
        if (r.gridX > maxGX) maxGX = r.gridX;
        if (r.gridY < minGY) minGY = r.gridY;
        if (r.gridY > maxGY) maxGY = r.gridY;
    }
    // 边界情况：至少有一个房间
    if (minGX > maxGX) { minGX = maxGX = 2; minGY = maxGY = 1; }
    int gridW = maxGX - minGX + 1;
    int gridH = maxGY - minGY + 1;

    // 居中偏移
    int offsetX = mapX + (mapW - gridW * cellSize) / 2;
    int offsetY = mapY + 12;

    auto roomCenter = [&](const RoomData &r) -> QPointF {
        return QPointF(offsetX + (r.gridX - minGX) * cellSize + cellSize / 2,
                       offsetY + (r.gridY - minGY) * cellSize + cellSize / 2);
    };

    // 清空上一帧的 cell 缓存
    m_mapCells.clear();

    // ----- 连线（只画两个都是 revealed 的房间之间的连线） -----
    painter->setPen(QPen(QColor(60, 60, 60), 1.5));
    for (const auto &r : rooms) {
        if (!rm.isRoomRevealed(r.id)) continue;
        QPointF from = roomCenter(r);
        auto drawLineTo = [&](int targetId) {
            if (targetId < 0 || targetId >= rooms.size()) return;
            if (!rm.isRoomRevealed(targetId)) return;
            QPointF to = roomCenter(rooms[targetId]);
            // 如果两个房间都已访问，连线亮一些
            if (r.visited && rooms[targetId].visited)
                painter->setPen(QPen(QColor(120, 120, 120), 1.5));
            else
                painter->setPen(QPen(QColor(50, 50, 50), 1.5));
            painter->drawLine(from, to);
        };
        drawLineTo(r.upTarget);
        drawLineTo(r.downTarget);
        drawLineTo(r.leftTarget);
        drawLineTo(r.rightTarget);
    }

    // ----- 房间格子 -----
    for (const auto &r : rooms) {
        if (!rm.isRoomRevealed(r.id)) continue;  // ★ 隐藏未揭示的房间

        QPointF c = roomCenter(r);
        QRectF cell(c.x() - 7, c.y() - 7, 14, 14);

        // 缓存给 mousePressEvent 用
        m_mapCells.append({r.id, cell, r.visited});

        QColor color;
        if (r.visited) {
            // 已探索：亮色
            if (r.type == RoomType::Start)        color = QColor(80, 200, 80);
            else if (r.type == RoomType::Treasure) color = QColor(220, 200, 40);
            else if (r.type == RoomType::Boss)     color = QColor(200, 60, 60);
            else                                   color = r.cleared ? QColor(140, 140, 140) : QColor(100, 100, 100);
        } else {
            // 未探索但相邻已探索：暗色
            if (r.type == RoomType::Treasure) color = QColor(80, 70, 20);
            else if (r.type == RoomType::Boss) color = QColor(80, 25, 25);
            else                              color = QColor(40, 40, 40);
        }

        painter->setBrush(QBrush(color));

        // 当前房间高亮边框
        if (r.id == curId) {
            painter->setPen(QPen(QColor(255, 255, 255), 2));
        } else if (r.visited) {
            painter->setPen(QPen(QColor(80, 80, 80), 1));
        } else {
            painter->setPen(QPen(QColor(40, 40, 40), 1));
        }
        painter->drawRoundedRect(cell, 3, 3);

        // Boss 房间标记 X
        if (r.type == RoomType::Boss) {
            QColor xColor = r.visited ? Qt::white : QColor(120, 120, 120);
            painter->setPen(QPen(xColor, 1.5));
            painter->drawLine(QPointF(c.x() - 4, c.y() - 4), QPointF(c.x() + 4, c.y() + 4));
            painter->drawLine(QPointF(c.x() + 4, c.y() - 4), QPointF(c.x() - 4, c.y() + 4));
        }

        // 可点击传送的提示：鼠标悬停效果用括号标记
        // （实际 hover 效果需要额外事件，这里省略，仅通过 click 传送）
    }

    // 楼层 + 小地图标题
    painter->setFont(QFont("monospace", 7));
    painter->setPen(QColor(140, 140, 140));
    painter->drawText(QRectF(mapX, mapY, mapW, 10), Qt::AlignHCenter,
                      QString("Floor %1 — MAP (click to travel)").arg(rm.floor()));

    // ========== 附近道具提示（底部居中） ==========
    if (!gm.nearbyItem().isEmpty()) {
        painter->setFont(QFont("monospace", 11));
        painter->setPen(QColor(255, 220, 60));
        painter->drawText(QRectF(150, 560, 500, 20), Qt::AlignCenter,
                          gm.nearbyItem() + "  [G] Pickup");
    }

    // ========== 起始房间教程（转场期间显示） ==========
    if (gm.isInTransition()) {
        painter->setFont(QFont("monospace", 10));
        painter->setPen(QColor(180, 180, 180));
        int tx = 250, ty = 510;
        painter->drawText(QRectF(tx, ty, 300, 80),
                          Qt::AlignCenter | Qt::TextWordWrap,
                          "WASD: Move  |  Arrows: Attack\n"
                          "G: Pickup Item  |  1: Use Bomb\n"
                          "Walk into door to exit room");
    }

    // ========== 无敌模式提示 ==========
    if (gm.godMode()) {
        painter->setFont(QFont("monospace", 11, QFont::Bold));
        painter->setPen(QColor(255, 200, 40));
        painter->drawText(QRectF(700, 2, 90, 16), Qt::AlignRight, "GOD MODE");
    }

    // ========== 拾取通知 ==========
    if (!gm.pickupMsg().isEmpty()) {
        painter->setFont(QFont("monospace", 14, QFont::Bold));
        painter->setPen(QColor(255, 220, 60));
        painter->drawText(QRectF(200, 40, 400, 30), Qt::AlignCenter, gm.pickupMsg());
    }

    // ========== 炸弹图标 ==========
    painter->setFont(m_font);
    int bombX = 750, bombY = 10;
    painter->setPen(Qt::white);
    if (gm.hasBomb()) {
        painter->setBrush(QBrush(QColor(40, 40, 40)));
        painter->drawEllipse(QPointF(bombX, bombY + 8), 10, 10);
        painter->setPen(QPen(QColor(255, 150, 50), 1.5));
        painter->drawLine(QPointF(bombX + 10, bombY + 3), QPointF(bombX + 14, bombY));
        painter->setPen(Qt::red);
        painter->drawPoint(QPointF(bombX + 14, bombY));
        qreal cd = gm.bombCooldown();
        if (cd > 0) {
            painter->setPen(Qt::white);
            painter->setFont(QFont("monospace", 10));
            painter->drawText(QRectF(bombX - 20, bombY + 20, 60, 20),
                              Qt::AlignCenter, QString::number(cd, 'f', 1));
        }
    }

    // ========== 转场黑屏 ==========
    if (gm.isInTransition()) {
        qreal t = gm.transitionTimer();
        // 前 0.3s 渐入，中间保持，最后 0.3s 渐出
        int alpha = 255;
        if (t > 1.2)       alpha = static_cast<int>(255 * (1.5 - t) / 0.3);  // 0.3s 渐出
        else if (t < 0.3)  alpha = static_cast<int>(255 * t / 0.3);           // 0.3s 渐入
        if (alpha > 255) alpha = 255;
        if (alpha < 0)   alpha = 0;

        painter->fillRect(QRectF(0, 0, 800, 600), QColor(0, 0, 0, alpha));
        if (alpha > 100) {
            painter->setFont(QFont("monospace", 32, QFont::Bold));
            painter->setPen(QColor(255, 255, 255, alpha));
            painter->drawText(QRectF(0, 200, 800, 60), Qt::AlignCenter,
                              QString("Floor %1").arg(RoomManager::instance().floor()));
        }
    }

    // ========== 消息提示 ==========
    if (!m_message.isEmpty()) {
        painter->setFont(QFont("monospace", 28));
        painter->setPen(Qt::white);
        painter->drawText(QRectF(0, 0, 800, 600), Qt::AlignCenter, m_message);
    }
}

void HUD::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // 只处理小地图区域内的点击
    QPointF pos = event->pos();
    if (!m_mapBounds.contains(pos)) {
        event->ignore();
        return;
    }

    // 遍历缓存的格子，找到被点击的那个
    for (const auto &cell : m_mapCells) {
        if (cell.rect.contains(pos) && cell.clickable) {
            // 传送到已访问过的房间
            RoomManager::instance().teleportToRoom(cell.roomId);
            event->accept();
            return;
        }
    }

    event->ignore();
}

void HUD::showMessage(const QString &msg)
{
    m_message = msg;
}
