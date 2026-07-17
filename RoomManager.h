#pragma once
#include "Door.h"
#include <QVector>
#include <QPointF>

class QGraphicsScene;
class Player;
class Enemy;
class Portal;

enum class DroppedItemType { DamageUp, FireRateUp, AddColumn, AddBullet, OrbitalRing };

struct DroppedItem {
    DroppedItemType type;
    qreal x, y;
};

enum class RoomType {
    Start,
    Normal,
    Treasure,
    Boss
};

struct RoomData {
    int id;
    RoomType type;
    int upTarget = -1;
    int downTarget = -1;
    int leftTarget = -1;
    int rightTarget = -1;
    int gridX = 0;
    int gridY = 0;
    int basicEnemyCount = 0;
    int shooterEnemyCount = 0;
    bool hasBoss = false;
    bool cleared = false;
    bool visited = false;
    QVector<DroppedItem> droppedItems;
};

// 障碍物模板 (相对坐标，中心位置)
struct ObstacleDef {
    qreal x, y;
};

// 房间内容模板
struct RoomTemplateDef {
    int basicCount;
    int shooterCount;
    QVector<ObstacleDef> crates;
};

class RoomManager {
public:
    static RoomManager& instance();

    void init(Player *player, QGraphicsScene *scene, class MainWindow *mainWindow = nullptr);

    void loadRoom(int roomId, Door::Direction entryDir = Door::Down);
    void roomCleared(int roomId);
    void goThroughDoor(Door::Direction dir);
    void teleportToRoom(int roomId);          // 小地图点击传送
    bool isRoomRevealed(int roomId) const;    // 是否在小地图上可见
    void spawnPortal();    // Boss 击败后生成传送门
    void nextFloor();      // 进入传送门 → 下一层

    int currentRoomId() const { return m_currentRoomId; }
    int floor() const { return m_floor; }
    RoomData* currentRoom() { return &m_rooms[m_currentRoomId]; }
    const QVector<RoomData>& rooms() const { return m_rooms; }

private:
    RoomManager() = default;
    void spawnEnemies(int roomId);
    void spawnCrates(int roomId);
    void clearScene();
    void placeWalls(int roomId);
    void placeDoors(int roomId);
    void placeFloor();
    void generateMap();
    QPointF doorPosition(Door::Direction dir);
    const RoomTemplateDef& randomTemplate();

    QGraphicsScene *m_scene = nullptr;
    Player *m_player = nullptr;
    class MainWindow *m_mainWindow = nullptr;
    QVector<RoomData> m_rooms;
    int m_currentRoomId = 0;
    int m_floor = 1;        // 当前楼层
    bool m_firstEntry = true;   // 本层首次进过渡房
    Portal *m_portal = nullptr;

    // 多个房间模板
    QVector<RoomTemplateDef> m_templates;
};
