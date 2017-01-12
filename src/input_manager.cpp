#include "input_manager.h"

#include <QAction>
#include <QKeyEvent>
#include <QMouseEvent>


bool InputManager::isKeyPressed(Qt::Key keycode) const
{
    return end(m_keys) != m_keys.find(static_cast<int>(keycode));
}

bool InputManager::isMouseButtonPressed(Qt::MouseButton button) const
{
    return end(m_keys) != m_keys.find(static_cast<int>(button));
}

void InputManager::registerAction(int keycode, QAction* action)
{
    m_actions.insert(std::make_pair(keycode, action));
}

void InputManager::triggerAction(int keycode, int event_type) const
{
    const auto it = m_actions.find(keycode);
    if (end(m_actions) != it)
    {
        QAction* action = it->second;
        action->setData(event_type);
        it->second->trigger();
    }
}

bool InputManager::eventFilter(QObject* watched, QEvent* event)
{
    Q_UNUSED(watched);

    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
        if (!key_event->isAutoRepeat())
        {
            m_keys.insert(key_event->key());
            triggerAction(key_event->key(), QEvent::KeyPress);
            return true;
        }

    }
    else if (event->type() == QEvent::KeyRelease)
    {
        QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
        if (!key_event->isAutoRepeat())
        {
            m_keys.erase(key_event->key());
            triggerAction(key_event->key(), QEvent::KeyRelease);
            return true;
        }
    }
    else if (event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);
        m_keys.insert(mouse_event->button());
        triggerAction(mouse_event->button(), QEvent::MouseButtonPress);
        return true;
    }
    else if (event->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);
        m_keys.erase(mouse_event->button());
        triggerAction(mouse_event->button(), QEvent::MouseButtonRelease);
        return true;
    }

    return false;
}
