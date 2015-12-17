#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "trianglewindow.h"


MainWindow::MainWindow(QWidget* parent /*=0*/)
    : QMainWindow{parent}
    , _glWindow(std::make_unique<TriangleWindow>())
    , _ui{std::make_unique<Ui::MainWindow>()}
{
    _ui->setupUi(this);

    QSurfaceFormat format;
    format.setSamples(16);

    _glWindow->setFormat(format);
    _glWindow->resize(640, 480);

    QWidget* glContainer = QWidget::createWindowContainer(_glWindow.get(), this);
    _ui->mainLayout->replaceWidget(_ui->glWidgetContainer, glContainer, Qt::FindDirectChildrenOnly);

    connect(_ui->buttonSpin, &QPushButton::clicked, _glWindow.get(), &TriangleWindow::setAnimating);
}

MainWindow::~MainWindow()
{
}
