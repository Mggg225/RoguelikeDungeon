#include "RoomManager.h"
#include "Player.h"
#include "BasicEnemy.h"
#include "ShooterEnemy.h"
#include "Boss.h"
#include "Wall.h"
#include "Crate.h"
#include "FloorBackground.h"
#include "GameManager.h"
#include "OrbitalRingItem.h"
#include "DamageUpItem.h"
#include "FireRateUpItem.h"
#include "AddColumnItem.h"
#include "AddBulletCountItem.h"
#include "OrbitalBullet.h"
#include "Portal.h"
#include "MainWindow.h"
#include "Item.h"
#include <QGraphicsScene>
#include <cstdlib>
#include <algorithm>
#include <random>
#include <QQueue>


RoomManager& RoomManager::instance()
{
    static RoomManager inst;
    return inst;
}

void RoomManager::init(Player *player, QGraphicsScene *scene, MainWindow *mainWindow)
{
    m_player = player;
    m_scene = scene;
    m_mainWindow = mainWindow;
    m_firstEntry = true;
    m_floor = 1;
    m_currentRoomId = 0;

    // 障碍物模板池
    m_templates = {
        // T0: 空旷，少敌
        {2, 1, {}},
        // T1: 中间十字柱群
        {2, 1, {{400, 300}, {400, 200}, {400, 400}, {300, 300}, {500, 300}}},
        // T2: 四角箱子
        {1, 2, {{150, 150}, {650, 150}, {150, 450}, {650, 450}}},
        // T3: 多柱子
        {3, 1, {{200, 200}, {600, 200}, {400, 400}, {200, 400}, {600, 400}}},
        // T4: 两侧墙柱
        {2, 2, {{100, 150}, {100, 300}, {100, 450}, {700, 150}, {700, 300}, {700, 450}}},
        // T5: 密集小房间感
        {3, 2, {{200, 130}, {600, 130}, {200, 470}, {600, 470}, {400, 200}, {400, 400}}},
    };

    // 房间连接关系（固定布局）
    generateMap();

    // 找到 Start 房间并加载（ID扫描重分配后索引可能不是0）
    for (int i = 0; i < m_rooms.size(); ++i) {
        if (m_rooms[i].type == RoomType::Start) { loadRoom(i); break; }
    }
}

const RoomTemplateDef& RoomManager::randomTemplate()
{
    int idx = std::rand() % m_templates.size();
    return m_templates[idx];
}

void RoomManager::loadRoom(int roomId, Door::Direction entryDir)
{
    // 环绕弹幕永存，不再清除
    clearScene();
    m_currentRoomId = roomId;

    auto &room = m_rooms[roomId];
    bool firstVisit = !room.visited;
    room.visited = true;

    if (room.type == RoomType::Boss && !room.cleared) {
        if (m_mainWindow) m_mainWindow->switchBgm(1);
    }

    // 进入过渡房 → 黑屏转场（仅每层首次）
    bool isFirstEntry = false;
    if (room.type == RoomType::Start) {
        isFirstEntry = m_firstEntry;
        if (m_firstEntry) {
            GameManager::instance().startTransition();
            m_firstEntry = false;
        }
    }

    placeFloor();
    placeWalls(roomId);
    placeDoors(roomId);

    if (room.type == RoomType::Start && isFirstEntry) {
        m_player->setPos(400, 300);
    } else {
        switch (entryDir) {
            case Door::Up:    m_player->setPos(400, 35); break;
            case Door::Down:  m_player->setPos(400, 565); break;
            case Door::Left:  m_player->setPos(35, 300); break;
            case Door::Right: m_player->setPos(765, 300); break;
        }
    }

    // 起始房永不生成敌人/箱子/锁门
    if (!room.cleared && room.type != RoomType::Start) {
        // 普通房随机选模板
        if (room.type == RoomType::Normal) {
            auto &tpl = randomTemplate();
            room.basicEnemyCount = tpl.basicCount;
            room.shooterEnemyCount = tpl.shooterCount;
        }
        spawnCrates(roomId);
        spawnEnemies(roomId);

        if (room.type == RoomType::Treasure && firstVisit) {
            new OrbitalRingItem(m_scene, QPointF(400, 300));
        }
        if (room.type == RoomType::Normal || room.type == RoomType::Boss) {
            for (auto *item : m_scene->items()) {
                auto *door = dynamic_cast<Door*>(item);
                if (door) door->setLocked(true);
            }
        }
    }

    // 已通关的 Boss 房重新生成传送门（被 clearScene 清除后恢复）
    if (room.type == RoomType::Boss && room.cleared && m_floor < 3) {
        spawnPortal();
    }

    // 重新生成已掉落道具（跨房间保留）
    for (const auto &di : room.droppedItems) {
        QPointF pos(di.x, di.y);
        switch (di.type) {
            case DroppedItemType::DamageUp:   new DamageUpItem(m_scene, pos); break;
            case DroppedItemType::FireRateUp: new FireRateUpItem(m_scene, pos); break;
            case DroppedItemType::AddColumn:  new AddColumnItem(m_scene, pos); break;
            case DroppedItemType::AddBullet:  new AddBulletCountItem(m_scene, pos); break;
            case DroppedItemType::OrbitalRing: new OrbitalRingItem(m_scene, pos); break;
        }
    }
}

void RoomManager::roomCleared(int roomId)
{
    m_rooms[roomId].cleared = true;
    for (auto *item : m_scene->items()) {
        auto *door = dynamic_cast<Door*>(item);
        if (door) {
            door->setLocked(false);
            door->setOpen(true);
        }
    }

    // 战斗房间清怪后回 1 滴血
    auto &room = m_rooms[roomId];
    if (room.type == RoomType::Normal || room.type == RoomType::Boss) {
        GameManager::instance().healPlayer(1);
    }

    // Boss 房清怪后：3层通关，否则生成传送门
    if (room.type == RoomType::Boss) {
        if (m_floor >= 3) {
            if (m_mainWindow) m_mainWindow->switchBgm(0);
            GameManager::instance().setVictory(true);
            return;
        }
        if (m_mainWindow) m_mainWindow->switchBgm(0);
        spawnPortal();
        GameManager::instance().showPickupMsg("BOSS DEFEATED! Enter the portal...");
        return;
    }

    int roll = std::rand() % 100;
    if (roll < 15) {
        new DamageUpItem(m_scene, QPointF(400, 300));
    } else if (roll < 30) {
        new FireRateUpItem(m_scene, QPointF(400, 300));
    } else if (roll < 45) {
        new AddColumnItem(m_scene, QPointF(400, 300));
    } else if (roll < 60) {
        new AddBulletCountItem(m_scene, QPointF(400, 300));
    }
}

void RoomManager::goThroughDoor(Door::Direction dir)
{
    auto &room = m_rooms[m_currentRoomId];
    int target = -1;
    switch (dir) {
        case Door::Up:    target = room.upTarget; break;
        case Door::Down:  target = room.downTarget; break;
        case Door::Left:  target = room.leftTarget; break;
        case Door::Right: target = room.rightTarget; break;
    }
    if (target >= 0) {
        Door::Direction entryDir;
        switch (dir) {
            case Door::Up:    entryDir = Door::Down; break;
            case Door::Down:  entryDir = Door::Up; break;
            case Door::Left:  entryDir = Door::Right; break;
            case Door::Right: entryDir = Door::Left; break;
        }
        loadRoom(target, entryDir);
    }
}

void RoomManager::spawnEnemies(int roomId)
{
    auto &room = m_rooms[roomId];

    // 收集场景中已有的障碍物位置（箱子）
    QVector<QPointF> obstacles;
    for (auto *item : m_scene->items()) {
        auto *crate = dynamic_cast<Crate*>(item);
        if (crate) obstacles.append(crate->pos());
    }

    // 安全生成：随机位置避让箱子、墙壁、玩家出生点
    const qreal safeDist = 50.0;  // 离障碍物的最小距离
    const qreal margin = 50.0;    // 离墙边距
    const int maxTries = 50;

    auto safePos = [&](qreal playerX, qreal playerY) -> QPointF {
        for (int t = 0; t < maxTries; ++t) {
            qreal ex = margin + (std::rand() % (int)(800.0 - margin * 2));
            qreal ey = margin + (std::rand() % (int)(600.0 - margin * 2));

            // 离玩家太近 → 跳过
            qreal dx = ex - playerX, dy = ey - playerY;
            if (dx*dx + dy*dy < 100*100) continue;  // 100px 内不行

            // 离任意箱子太近 → 跳过
            bool hit = false;
            for (auto &obs : obstacles) {
                qreal ox = ex - obs.x(), oy = ey - obs.y();
                if (ox*ox + oy*oy < safeDist*safeDist) { hit = true; break; }
            }
            if (!hit) return QPointF(ex, ey);
        }
        // 50 次没找到好位置，给一个兜底
        return QPointF(margin + std::rand() % (int)(800.0 - margin * 2),
                       margin + std::rand() % (int)(600.0 - margin * 2));
    };

    auto &gm = GameManager::instance();
    qreal cntMult = gm.enemyCountMult();
    int floorExtra = gm.isHard() ? (m_floor - 1) * 2 : 0; // 困难模式每层+2

    for (int i = 0; i < qMax(0, static_cast<int>(room.basicEnemyCount * cntMult) + floorExtra); ++i) {
        QPointF pos = safePos(m_player->pos().x(), m_player->pos().y());
        new BasicEnemy(m_scene, m_player, pos);
    }
    for (int i = 0; i < qMax(0, static_cast<int>(room.shooterEnemyCount * cntMult) + floorExtra); ++i) {
        QPointF pos = safePos(m_player->pos().x(), m_player->pos().y());
        new ShooterEnemy(m_scene, m_player, pos);
    }
    if (room.hasBoss) {
        new Boss(m_scene, m_player, QPointF(400, 300));
    }
}

void RoomManager::spawnCrates(int roomId)
{
    auto &room = m_rooms[roomId];
    if (room.type != RoomType::Normal) return;

    // 使用随机模板的障碍物
    int tplIdx = std::rand() % m_templates.size();
    const auto &tpl = m_templates[tplIdx];
    for (const auto &obs : tpl.crates) {
        auto *crate = new Crate(QPointF(obs.x, obs.y));
        m_scene->addItem(crate);
    }
}

void RoomManager::clearScene()
{
    // 保存当前房间的道具掉落记录
    auto &room = m_rooms[m_currentRoomId];
    room.droppedItems.clear();

    QList<QGraphicsItem*> items = m_scene->items();
    for (auto *item : items) {
        auto *go = dynamic_cast<GameObject*>(item);
        if (go && go != m_player && !dynamic_cast<OrbitalBullet*>(go)) {
            if (dynamic_cast<Portal*>(go)) m_portal = nullptr;

            // 保存道具到房间数据
            auto *it = dynamic_cast<Item*>(go);
            if (it) {
                DroppedItemType t;
                if (dynamic_cast<DamageUpItem*>(it))      t = DroppedItemType::DamageUp;
                else if (dynamic_cast<FireRateUpItem*>(it)) t = DroppedItemType::FireRateUp;
                else if (dynamic_cast<AddColumnItem*>(it))  t = DroppedItemType::AddColumn;
                else if (dynamic_cast<AddBulletCountItem*>(it)) t = DroppedItemType::AddBullet;
                else if (dynamic_cast<OrbitalRingItem*>(it))    t = DroppedItemType::OrbitalRing;
                else { m_scene->removeItem(item); delete item; continue; }
                room.droppedItems.append({t, it->pos().x(), it->pos().y()});
            }

            m_scene->removeItem(item);
            delete item;
        }
    }
}

void RoomManager::placeFloor()
{
    auto *floor = new FloorBackground;
    m_scene->addItem(floor);
}

void RoomManager::teleportToRoom(int roomId)
{
    if (roomId < 0 || roomId >= m_rooms.size()) return;
    if (!m_rooms[roomId].visited) return;

    loadRoom(roomId, Door::Down);
}

bool RoomManager::isRoomRevealed(int roomId) const
{
    if (roomId < 0 || roomId >= m_rooms.size()) return false;
    const auto &r = m_rooms[roomId];
    if (r.visited) return true;

    auto check = [&](int target) {
        if (target >= 0 && target < m_rooms.size() && m_rooms[target].visited)
            return true;
        return false;
    };
    return check(r.upTarget) || check(r.downTarget)
        || check(r.leftTarget) || check(r.rightTarget);
}

void RoomManager::spawnPortal()
{
    if (m_portal) return;  // 已存在

    m_portal = new Portal;
    m_portal->setPos(400, 300);
    m_scene->addItem(m_portal);
}

void RoomManager::nextFloor()
{
    m_floor++;
    m_firstEntry = true;
    if (m_portal) {
        m_scene->removeItem(m_portal);
        delete m_portal;
        m_portal = nullptr;
    }

    // 清空场景（保留玩家和环绕弹幕；道具由 clearScene 保存到房间数据）
    clearScene();

    // 生成新地图
    generateMap();

    // 找到 Start 房间并加载
    for (int i = 0; i < m_rooms.size(); ++i) {
        if (m_rooms[i].type == RoomType::Start) {
            loadRoom(i);
            m_player->setPos(400, 300);
            break;
        }
    }

    GameManager::instance().showPickupMsg(QString("Floor %1").arg(m_floor));
}


void RoomManager::placeWalls(int roomId)
{
    auto &room = m_rooms[roomId];
    const qreal wt = 10, dh = 22;
    const qreal cx = 400, cy = 300, rw = 800, rh = 600;

    if (room.upTarget >= 0) {
        m_scene->addItem(new Wall(0, 0, cx - dh, wt));
        m_scene->addItem(new Wall(cx + dh, 0, rw - cx - dh, wt));
    } else {
        m_scene->addItem(new Wall(0, 0, rw, wt));
    }
    if (room.downTarget >= 0) {
        m_scene->addItem(new Wall(0, rh - wt, cx - dh, wt));
        m_scene->addItem(new Wall(cx + dh, rh - wt, rw - cx - dh, wt));
    } else {
        m_scene->addItem(new Wall(0, rh - wt, rw, wt));
    }
    if (room.leftTarget >= 0) {
        m_scene->addItem(new Wall(0, 0, wt, cy - dh));
        m_scene->addItem(new Wall(0, cy + dh, wt, rh - cy - dh));
    } else {
        m_scene->addItem(new Wall(0, 0, wt, rh));
    }
    if (room.rightTarget >= 0) {
        m_scene->addItem(new Wall(rw - wt, 0, wt, cy - dh));
        m_scene->addItem(new Wall(rw - wt, cy + dh, wt, rh - cy - dh));
    } else {
        m_scene->addItem(new Wall(rw - wt, 0, wt, rh));
    }
}

void RoomManager::placeDoors(int roomId)
{
    auto &room = m_rooms[roomId];
    if (room.upTarget >= 0) {
        auto *door = new Door(Door::Up, room.upTarget);
        door->setPos(400, 5);
        m_scene->addItem(door);
    }
    if (room.downTarget >= 0) {
        auto *door = new Door(Door::Down, room.downTarget);
        door->setPos(400, 595);
        m_scene->addItem(door);
    }
    if (room.leftTarget >= 0) {
        auto *door = new Door(Door::Left, room.leftTarget);
        door->setPos(5, 300);
        m_scene->addItem(door);
    }
    if (room.rightTarget >= 0) {
        auto *door = new Door(Door::Right, room.rightTarget);
        door->setPos(795, 300);
        m_scene->addItem(door);
    }
}

QPointF RoomManager::doorPosition(Door::Direction dir)
{
    switch (dir) {
        case Door::Up:    return QPointF(400, 5);
        case Door::Down:  return QPointF(400, 595);
        case Door::Left:  return QPointF(5, 300);
        case Door::Right: return QPointF(795, 300);
    }
    return QPointF(400, 300);
}



void RoomManager::generateMap()
{
    const int GRID_W = 5;
    const int GRID_H = 4;

    // ---------- 初始化网格 ----------
    struct Cell {
        int id = -1;
        int up = -1, down = -1, left = -1, right = -1;
        int gridX = 0, gridY = 0;
        bool visited = false;
    };
    Cell grid[GRID_H][GRID_W];
    for (int y = 0; y < GRID_H; y++)
        for (int x = 0; x < GRID_W; x++) {
            grid[y][x].gridX = x;
            grid[y][x].gridY = y;
        }

    // ---------- 过渡房 (2, 2) 永远 id=0, type=Start ----------
    const int hubX = 2, hubY = 2;

    // ---------- 第 1 步：DFS 探索，从 (2, 3) 开始 ----------
    const int dfsX = 2, dfsY = 3;

    // 过渡房和 DFS 根之间双向连接
    grid[hubY][hubX].visited = true;
    grid[hubY][hubX].id = 0;
    grid[hubY][hubX].down = -2;       // 过渡房 ↓ → DFS 根
    grid[dfsY][dfsX].up = -2;         // DFS 根 ↑ → 过渡房

    QVector<QPoint> stack;
    stack.push_back({dfsX, dfsY});
    grid[dfsY][dfsX].visited = true;

    const int DX[] = {0, 0, -1, 1};   // 上、下、左、右
    const int DY[] = {-1, 1, 0, 0};

    std::random_device rd;
    std::mt19937 rng(rd());

    while (!stack.isEmpty()) {
        QPoint cur = stack.last();
        int cx = cur.x(), cy = cur.y();

        // 收集未访问邻居
        QVector<int> neighbors;
        for (int i = 0; i < 4; i++) {
            int nx = cx + DX[i], ny = cy + DY[i];
            if (nx >= 0 && nx < GRID_W && ny >= 0 && ny < GRID_H
                && !grid[ny][nx].visited) {
                neighbors.append(i);
            }
        }

        if (neighbors.isEmpty()) {
            stack.pop_back();
        } else {
            std::uniform_int_distribution<int> dist(0, neighbors.size() - 1);
            int chosen = neighbors[dist(rng)];
            int nx = cx + DX[chosen], ny = cy + DY[chosen];

            // 双向连接标记（-2）
            switch (chosen) {
            case 0: grid[cy][cx].up    = -2; grid[ny][nx].down  = -2; break;
            case 1: grid[cy][cx].down  = -2; grid[ny][nx].up    = -2; break;
            case 2: grid[cy][cx].left  = -2; grid[ny][nx].right = -2; break;
            case 3: grid[cy][cx].right = -2; grid[ny][nx].left  = -2; break;
            }

            grid[ny][nx].visited = true;
            stack.push_back({nx, ny});
        }
    }

    // ---------- 第 2 步：统一分配 ID ----------
    // 起始格 = 房间 0，其余按扫描顺序分配 1, 2, 3...
    int nextId = 0;
    for (int y = 0; y < GRID_H; y++)
        for (int x = 0; x < GRID_W; x++)
            if (grid[y][x].visited)
                grid[y][x].id = nextId++;

    const int totalRooms = nextId;

    // ---------- 第 3 步：-2 → 真实 roomId ----------
    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            if (!grid[y][x].visited) continue;
            if (grid[y][x].up    == -2 && y > 0       && grid[y-1][x].visited) grid[y][x].up    = grid[y-1][x].id;
            if (grid[y][x].down  == -2 && y < GRID_H-1 && grid[y+1][x].visited) grid[y][x].down  = grid[y+1][x].id;
            if (grid[y][x].left  == -2 && x > 0       && grid[y][x-1].visited) grid[y][x].left  = grid[y][x-1].id;
            if (grid[y][x].right == -2 && x < GRID_W-1 && grid[y][x+1].visited) grid[y][x].right = grid[y][x+1].id;
        }
    }

    // ---------- 第 4 步：BFS 计算深度 ----------
    int startRoomId = grid[hubY][hubX].id;  // 过渡房是 BFS 根
    QVector<int> bfsDepth(totalRooms, -1);
    QQueue<int> q;
    bfsDepth[startRoomId] = 0;
    q.enqueue(startRoomId);

    while (!q.isEmpty()) {
        int curId = q.dequeue();
        for (int y = 0; y < GRID_H; y++) {
            for (int x = 0; x < GRID_W; x++) {
                if (grid[y][x].id != curId) continue;
                auto check = [&](int nid) {
                    if (nid >= 0 && bfsDepth[nid] < 0) {
                        bfsDepth[nid] = bfsDepth[curId] + 1;
                        q.enqueue(nid);
                    }
                };
                check(grid[y][x].up);
                check(grid[y][x].down);
                check(grid[y][x].left);
                check(grid[y][x].right);
            }
        }
    }

    // ---------- 第 5 步：分配类型 ----------
    // 房间 0 (起始房) 硬编码为 Start
    int bossId = -1, treasureId = -1;

    if (totalRooms >= 2) {
        // 找最深节点 → Boss（排除房间 0）
        int maxDepth = 0;
        for (int i = 0; i < totalRooms; i++) {
            if (i == startRoomId) continue;
            if (bfsDepth[i] > maxDepth) {
                maxDepth = bfsDepth[i];
                bossId = i;
            }
        }

        // 找中等深度节点 → Treasure
        QVector<int> mids;
        for (int i = 0; i < totalRooms; i++) {
            if (i == startRoomId || i == bossId) continue;
            if (bfsDepth[i] >= 2 && bfsDepth[i] < maxDepth)
                mids.append(i);
        }
        if (!mids.isEmpty()) {
            std::uniform_int_distribution<int> pick(0, mids.size() - 1);
            treasureId = mids[pick(rng)];
        } else if (bossId >= 0) {
            treasureId = bossId;  // 兜底：房间太少，Boss 房兼做道具房
        }
    }

    // ---------- 第 6 步：构建 m_rooms ----------
    m_rooms.clear();
    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            if (!grid[y][x].visited) continue;

            RoomData room;
            room.id          = grid[y][x].id;
            room.upTarget    = grid[y][x].up;
            room.downTarget  = grid[y][x].down;
            room.leftTarget  = grid[y][x].left;
            room.rightTarget = grid[y][x].right;
            room.gridX       = x;
            room.gridY       = y;
            room.cleared     = false;
            room.visited     = false;

            if (x == hubX && y == hubY)
                room.type = RoomType::Start;
            else if (room.id == bossId && bossId >= 0)
                room.type = RoomType::Boss;
            else if (room.id == treasureId && treasureId >= 0)
                room.type = RoomType::Treasure;
            else
                room.type = RoomType::Normal;

            room.basicEnemyCount   = 0;
            room.shooterEnemyCount = 0;
            room.hasBoss = (room.type == RoomType::Boss);

            m_rooms.append(room);
        }
    }
}
