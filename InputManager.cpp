#include "InputManager.h"

InputManager& InputManager::instance()
{
    static InputManager inst;
    return inst;
}

void InputManager::keyPressEvent(QKeyEvent *event)
{
    m_pressedKeys.insert(static_cast<Qt::Key>(event->key()));
}

void InputManager::keyReleaseEvent(QKeyEvent *event)
{
    m_pressedKeys.remove(static_cast<Qt::Key>(event->key()));
}

bool InputManager::isKeyDown(Qt::Key key) const
{
    return m_pressedKeys.contains(key);
}
