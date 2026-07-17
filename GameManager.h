#pragma once
#include <QtGlobal>
#include <QString>
#include <QStringList>

class GameManager {
public:
    static GameManager& instance();

    int playerHP() const { return m_playerHP; }
    void setPlayerHP(int hp) { m_playerHP = hp; }
    void healPlayer(int amount) {
        m_playerHP += amount;
        if (m_playerHP > m_maxHP) m_playerHP = m_maxHP;
    }
    int maxHP() const { return m_maxHP; }
    void setMaxHP(int hp) { m_maxHP = hp; }
    void takeDamage(int dmg) {
        if (m_godMode || m_invincibleTimer > 0) return;
        m_playerHP -= dmg;
        if (m_playerHP < 0) m_playerHP = 0;
        m_invincibleTimer = 0.3;
    }
    bool godMode() const { return m_godMode; }
    void toggleGodMode() { m_godMode = !m_godMode; }
    void tickInvincible(qreal dt) { m_invincibleTimer -= dt; }
    bool isInvincible() const { return m_invincibleTimer > 0; }

    int attackDamage() const { return m_godMode ? 1000 : m_attackDamage; }
    void setAttackDamage(int d) { m_attackDamage = d; }

    qreal attackCooldown() const { return m_godMode ? 0.05 : m_attackCooldown; }
    void setAttackCooldown(qreal cd) { m_attackCooldown = cd; }

    int attackMode() const { return m_godMode ? 2 : m_attackMode; }
    void setAttackMode(int mode) { m_attackMode = mode; }

    bool hasBomb() const { return m_hasBomb; }
    void setHasBomb(bool b) { m_hasBomb = b; }
    qreal bombCooldown() const { return m_bombCooldown; }
    void setBombCooldown(qreal cd) { m_bombCooldown = cd; }
    void tickBombCooldown(qreal dt) { m_bombCooldown -= dt; }

    int orbitalCount() const { return m_orbitalCount; }
    void incrementOrbital() { m_orbitalCount++; }
    void decrementOrbital() { m_orbitalCount--; if (m_orbitalCount < 0) m_orbitalCount = 0; }

    int extraColumns() const { return m_extraColumns; }
    void setExtraColumns(int v) { m_extraColumns = v; }

    int extraBullets() const { return m_extraBullets; }
    void setExtraBullets(int v) { m_extraBullets = v; }

    // 游戏状态
    bool isGameOver() const { return m_gameOver; }
    void setGameOver(bool v) { m_gameOver = v; }
    bool isVictory() const { return m_victory; }
    void setVictory(bool v) { m_victory = v; }

    // 拾取通知
    void showPickupMsg(const QString &msg) { m_pickupMsg = msg; m_pickupMsgTimer = 2.0; }
    QString pickupMsg() const { return m_pickupMsg; }
    void tickPickupMsg(qreal dt) { m_pickupMsgTimer -= dt; if (m_pickupMsgTimer <= 0) m_pickupMsg.clear(); }

    // 附近道具提示
    bool isInTransition() const { return m_inTransition; }
    void startTransition() { m_transitionTimer = 1.5; m_inTransition = true; }
    void tickTransition(qreal dt) {
        if (m_inTransition) { m_transitionTimer -= dt; if (m_transitionTimer <= 0) m_inTransition = false; }
    }
    qreal transitionTimer() const { return m_transitionTimer; }

    void setNearbyItem(const QString &s) { m_nearbyItem = s; }
    QString nearbyItem() const { return m_nearbyItem; }

    // Boss 血条
    int bossHP() const { return m_bossHP; }
    void setBossHP(int hp) { m_bossHP = hp; }
    int bossMaxHP() const { return m_bossMaxHP; }
    void setBossMaxHP(int mhp) { m_bossMaxHP = mhp; }


    int sfxChoice() const { return m_sfxChoice; }
    void setSfxChoice(int v) { m_sfxChoice = v; }
    QString spriteIdle() const { return m_spriteIdle; }
    void setSpriteIdle(const QString &p) { m_spriteIdle = p; }
    QString spriteRun() const { return m_spriteRun; }
    void setSpriteRun(const QString &p) { m_spriteRun = p; }
    QString spriteDie() const { return m_spriteDie; }
    void setSpriteDie(const QString &p) { m_spriteDie = p; }
    QStringList bulletSprites() const { return m_bulletSprites; }
    void setBulletSprites(const QStringList &l) { m_bulletSprites = l; }
    QString skillSfxPath() const{return m_skillSfxPath;}
    void setSkillSfxPath(const QString &p){m_skillSfxPath = p;}

    // 技能系统
    bool skillActive() const { return m_skillActive; }
    qreal skillTimer() const { return m_skillTimer; }
    qreal skillCooldown() const { return m_skillCooldown; }
    bool activateSkill(int duration, int cooldown) {
        if (m_skillCooldown > 0 || m_skillActive) return false;
        m_skillActive = true;
        m_skillTimer = duration;
        m_skillCooldown = cooldown;
        return true;
    }
    void tickSkill(qreal dt) {
        if (m_skillActive) { m_skillTimer -= dt; if (m_skillTimer <= 0) m_skillActive = false; }
        if (!m_skillActive && m_skillCooldown > 0) m_skillCooldown -= dt;
    }
    bool doubleAttack() const { return m_skillActive; }  // 火力全开：双倍攻击
    QString customSfxPath() const { return m_customSfxPath; }
    void setCustomSfxPath(const QString &p) { m_customSfxPath = p; }

    // ---- 难度系统 ----
    enum Difficulty { Normal = 0, Hard = 1 };
    Difficulty difficulty() const { return m_difficulty; }
    void setDifficulty(Difficulty d) { m_difficulty = d; }
    bool isHard() const { return m_difficulty == Hard; }

    qreal enemyHPMult()     const { return isHard() ? 2.0  : 1.0; }
    qreal enemyDamageMult() const { return isHard() ? 1.5  : 1.0; }
    qreal enemyCountMult()  const { return isHard() ? 2.0  : 1.0; }
    int   bossBaseHP()      const { return isHard() ? 500  : 50;  }
    qreal damageItemMult()  const { return isHard() ? 1.2  : 1.5; }
    qreal fireRateMult()    const { return isHard() ? 0.88 : 0.80; }
    qreal playerSpeedMult() const { return isHard() ? 1.25 : 1.0;  }

    void reset() {
        m_playerHP = m_maxHP;
        m_attackDamage = 1;
        m_attackCooldown = 0.35;
        m_attackMode = 0;
        m_hasBomb = false;
        m_bombCooldown = 0.0;
        m_orbitalCount = 0;
        m_extraColumns = 0;
        m_extraBullets = 0;
        m_invincibleTimer = 0.0;
        m_godMode = false;
        m_bossHP = 0;
        m_bossMaxHP = 0;
        m_gameOver = false;
        m_victory = false;
        m_inTransition = false;
        m_transitionTimer = 0.0;
        m_skillActive = false;
        m_skillTimer = 0.0;
        m_skillCooldown = 0.0;
    }

private:
    GameManager() = default;

    int m_playerHP = 8;
    int m_maxHP = 8;
    int m_attackDamage = 1;
    qreal m_attackCooldown = 0.35;
    int m_attackMode = 0; // 0=直线, 1=环绕
    bool m_hasBomb = false;
    qreal m_bombCooldown = 0.0;
    int m_orbitalCount = 0;
    int m_extraColumns = 0;
    int m_extraBullets = 0;
    qreal m_invincibleTimer = 0.0;
    bool m_godMode = false;
    QString m_pickupMsg;
    qreal m_pickupMsgTimer = 0.0;
    QString m_nearbyItem;
    int m_bossHP = 0;
    int m_bossMaxHP = 0;
    bool m_gameOver = false;
    bool m_victory = false;
    bool m_inTransition = false;
    qreal m_transitionTimer = 0.0;
    bool m_skillActive = false;
    qreal m_skillTimer = 0.0;
    qreal m_skillCooldown = 0.0;
    int m_sfxChoice = 0;
    QString m_customSfxPath;
    QString m_spriteIdle = ":/assets/player_idle.gif";
    QString m_spriteRun  = ":/assets/player_run.gif";
    QString m_spriteDie  = ":/assets/player_die.png";
    QStringList m_bulletSprites;
    QString m_skillSfxPath;
    Difficulty m_difficulty = Normal;
};
