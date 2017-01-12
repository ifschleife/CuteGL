#pragma once

#include <QObject>

#include <set>
#include <unordered_map>


class QAction;


class InputManager : public QObject
{
public:
    bool isKeyPressed(Qt::Key keycode) const;
    bool isMouseButtonPressed(Qt::MouseButton button) const;

    void registerAction(int keycode, QAction* action);

private:
    void triggerAction(int, int) const;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    std::set<int> m_keys;
    std::unordered_map<int, QAction*> m_actions;
};
