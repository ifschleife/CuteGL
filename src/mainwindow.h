#pragma once

#include <QMainWindow>
#include <memory>


namespace Ui { class MainWindow; }
class TriangleWindow;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow() override;

private:
	void showFrameTime(float time_in_ms);

private:
    std::unique_ptr<TriangleWindow> m_glWindow;
    std::unique_ptr<Ui::MainWindow> m_ui;
};
