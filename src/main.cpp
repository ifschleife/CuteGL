#include "mainwindow.h"

#include <QApplication>
#include <QDir>
#include <QSettings>
#include <memory>


int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setOrganizationName("Triangulus Inc.");
    app.setApplicationName("CuteGL");

    QSettings settings;
    QDir current_path = QDir::current();
    current_path.cdUp();
    QDir::setCurrent(current_path.absolutePath());

    settings.setValue("path/root", current_path.absolutePath());
    settings.setValue("path/assets", "assets");
    settings.setValue("path/shaders", "src/shaders");

    std::unique_ptr<MainWindow> main_window = std::make_unique<MainWindow>();
    main_window->show();

    return app.exec();
}
