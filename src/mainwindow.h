#pragma once

#include <QMainWindow>
#include <memory>


namespace Ui
{
    class MainWindow;
}
class OpenGLWindow;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow() override;

private:
    bool eventFilter(QObject* watched, QEvent* event) override;

    void showFrameTime(float time_in_ms);

private:
    std::unique_ptr<OpenGLWindow> m_glWindow;
    std::unique_ptr<Ui::MainWindow> m_ui;
};
