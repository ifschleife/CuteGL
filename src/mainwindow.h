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
    ~MainWindow();

private:
    std::unique_ptr<TriangleWindow> _glWindow;
    std::unique_ptr<Ui::MainWindow> _ui;
};
