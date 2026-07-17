#pragma once
#include <QSet>
#include <QKeyEvent>

class InputManager {
public:
    static InputManager& instance();

    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    bool isKeyDown(Qt::Key key) const;

private:
    InputManager() = default;
    QSet<Qt::Key> m_pressedKeys;
};
