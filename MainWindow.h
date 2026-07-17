#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTimer>
#include <QKeyEvent>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QComboBox>
#include <QMovie>


class Player;
class HUD;

struct CharacterInfo {
    QString name;
    QString assetPath;   // GIF or PNG for char select preview
    QString idlePath;    // 待机 GIF
    QString runPath;     // 行走 GIF
    QString diePath;     // 死亡 PNG
    int health;          // 初始 HP
    QString description; // 背景介绍
    QString skillDesc;   // 技能描述
    bool hasSkill;       // 是否有主动技能
    QStringList bulletSprites; // 子弹精灵图（空=程序化白色圆）
    QString skillSfxInfo;//技能音效 (空 = 无音效）
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void gameLoop();
    void onStartClicked();
    void onSettingsClicked();
    void onQuitClicked();
    void onConfirmClicked();
    void onBackClicked();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

public:
    void switchBgm(int state);

private:
    void setupMenuPage();
    void setupGamePage();
    void setupSettingsPage();
    void setupCharacterSelectPage();
    void startGame();
    void returnToMenu();
    void refreshCharSelect();
    void setupPauseOverlay();
    void togglePause();
    static void fadeIn(QMediaPlayer *player, qreal targetVol = 0.5, int duration = 2000);
    static void fadeOut(QMediaPlayer *player, int duration = 1500);

    QStackedWidget *m_stack;

    // Menu page widgets
    QWidget *m_menuPage;
    QPushButton *m_btnStart;
    QPushButton *m_btnSettings;
    QPushButton *m_btnQuit;

    // Settings page
    QWidget *m_settingsPage;
    QComboBox *m_comboSfx;

    // Character select page
    QWidget *m_charSelectPage;
    QLabel *m_charNameLabel;
    QLabel *m_charPreview;
    QMovie *m_charMovie = nullptr;
    QLabel *m_charDescLabel;
    QLabel *m_charSkillLabel;
    QVector<CharacterInfo> m_characters;
    int m_charIndex = 0;
    QPushButton *m_btnConfirm;

    // Difficulty select page
    QWidget *m_diffPage;
    QLabel *m_diffNormalLabel;
    QLabel *m_diffHardLabel;
    int m_diffIndex = 0;  // 0=Normal, 1=Hard
    void setupDifficultyPage();
    void refreshDiffSelect();
    void onConfirmDiff();

    // Pause overlay
    QWidget *m_pauseOverlay;
    QWidget *m_pausePanel;
    QLabel *m_pauseCharImg;

    // Game page widgets
    QWidget *m_gamePage;
    QGraphicsView *m_view;
    QGraphicsScene *m_scene;
    QTimer *m_timer;
    Player *m_player = nullptr;
    HUD *m_hud = nullptr;
    bool m_gameStarted = false;
    bool m_paused = false;

    //bgm
    QMediaPlayer *m_bgm;
    int bgmState;// 0为菜单页面  1为普通房间  2为boss的bgm

};
