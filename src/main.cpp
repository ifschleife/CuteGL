#include "mainwindow.h"

#include <QApplication>
#include <memory>


int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    std::unique_ptr<MainWindow> main_window = std::make_unique<MainWindow>();
    main_window->show();

    return app.exec();
}
