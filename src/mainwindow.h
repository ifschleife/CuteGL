#pragma once

#include <QMainWindow>
#include <memory>


namespace Ui
{
    class MainWindow;
}
class InputManager;
class OpenGLWindow;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow() override;

private:
    void showFrameTime(float time_in_ms);

private slots:
    void main_loop();

private:
    std::unique_ptr<OpenGLWindow> m_glWindow;
    std::unique_ptr<InputManager> m_input_manager;
    std::unique_ptr<QTime> m_main_loop_time;
    std::unique_ptr<Ui::MainWindow> m_ui;
};
