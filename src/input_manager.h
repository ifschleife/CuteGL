#pragma once

#include <QObject>

#include <set>


class InputManager : public QObject
{
    Q_OBJECT

public:
    bool isKeyPressed(Qt::Key keycode) const;

private:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    std::set<int> m_keys;
};
