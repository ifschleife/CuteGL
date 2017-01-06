#include "input_manager.h"

#include <QKeyEvent>


bool InputManager::isKeyPressed(Qt::Key keycode) const
{
    return end(m_keys) != m_keys.find(static_cast<int>(keycode));
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
            return true;
        }
    }
    else if (event->type() == QEvent::KeyRelease)
    {
        QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
        if (!key_event->isAutoRepeat())
        {
            m_keys.erase(key_event->key());
            return true;
        }
    }

    return false;
}
