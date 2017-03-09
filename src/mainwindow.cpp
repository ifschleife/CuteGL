#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "input_manager.h"
#include "opengl_window.h"
#include "util.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QKeyEvent>
#include <QSettings>
#include <QTime>
#include <QTimer>


MainWindow::MainWindow(QWidget* parent /*=0*/)
  : QMainWindow{parent}
  , m_glWindow{std::make_unique<OpenGLWindow>()}
  , m_input_manager{std::make_unique<InputManager>()}
  , m_main_loop_time{std::make_unique<QTime>()}
  , m_ui{std::make_unique<Ui::MainWindow>()}
  , m_right_mouse_action{new QAction{this}}
{
    m_ui->setupUi(this);

    QSurfaceFormat format;
    format.setSamples(1);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setVersion(4, 5);
    DEBUG_CALL(format.setOption(QSurfaceFormat::DebugContext));

    m_glWindow->setFormat(format);
    m_glWindow->resize(1024, 768);

    connect(m_ui->actionQuit, &QAction::triggered, this, &QMainWindow::close);

    QWidget* glContainer = QWidget::createWindowContainer(m_glWindow.get(), this);
    m_ui->mainLayout->replaceWidget(m_ui->glWidgetContainer, glContainer, Qt::FindDirectChildrenOnly);

    connect(m_ui->buttonSpin, &QPushButton::clicked, m_glWindow.get(), &OpenGLWindow::setAnimating);
    connect(m_ui->buttonWireFrame, &QPushButton::clicked, m_glWindow.get(), &OpenGLWindow::showWireFrame);

    connect(m_glWindow.get(), &OpenGLWindow::frameTime, this, &MainWindow::showFrameTime);

    installEventFilter(m_input_manager.get());
    // the OpenGLWindow is not really part of the hierarchy so we need to make sure it does not swallow events
    m_glWindow->installEventFilter(m_input_manager.get());

    connect(m_right_mouse_action, &QAction::triggered, this, &MainWindow::onRightMouseButtonPress);
    m_input_manager->registerAction(Qt::RightButton, m_right_mouse_action);

    setFocus();

    auto timer = new QTimer(this);
    timer->setTimerType(Qt::PreciseTimer);
    connect(timer, &QTimer::timeout, this, &MainWindow::main_loop);
    timer->start(0);

    m_main_loop_time->start();
}

MainWindow::~MainWindow() = default;

void MainWindow::showFrameTime(float time_in_ms)
{
    const QString text = QString("%1 ms").arg(time_in_ms);
    m_ui->frameTimeLabel->setText(text);
}

void MainWindow::main_loop()
{
    if (m_main_loop_time->elapsed() < 16) // game loop runs at vsync rate for now
        return;

    updateCameraRotation();
    updateCameraTranslation();

    m_glWindow->update();

    m_main_loop_time->start();
}

void MainWindow::updateCameraRotation()
{
    float yaw_angle = 0.0f;
    float pitch_angle = 0.0f;

    if (m_input_manager->isMouseButtonPressed(Qt::RightButton))
    {
        const QPoint center = geometry().center();
        const QPoint dist = center - QCursor::pos();

        yaw_angle = dist.x() / 5.0f;
        pitch_angle = -dist.y() / 5.0f;

        QCursor::setPos(center);
    }
    else // control camera yaw/pitch with keyboard
    {
        if (m_input_manager->isKeyPressed(Qt::Key_Left))
            yaw_angle = 5.0f;
        if (m_input_manager->isKeyPressed(Qt::Key_Right))
            yaw_angle = -5.0f;

        if (m_input_manager->isKeyPressed(Qt::Key_Up))
            pitch_angle = -1.0f;
        if (m_input_manager->isKeyPressed(Qt::Key_Down))
            pitch_angle = 1.0f;
    }

    m_glWindow->m_camera.change_yaw(yaw_angle);
    m_glWindow->m_camera.change_pitch(pitch_angle);
}

void MainWindow::updateCameraTranslation()
{
    const float forward_step = m_input_manager->isKeyPressed(Qt::Key_W) ? 0.05f : 0.0f;
    const float backward_step = m_input_manager->isKeyPressed(Qt::Key_S) ? 0.05f : 0.0f;
    const float left_step = m_input_manager->isKeyPressed(Qt::Key_A) ? 0.05f : 0.0f;
    const float right_step = m_input_manager->isKeyPressed(Qt::Key_D) ? 0.05f : 0.0f;

    m_glWindow->m_camera.move_forward(forward_step);
    m_glWindow->m_camera.move_backward(backward_step);
    m_glWindow->m_camera.move_left(left_step);
    m_glWindow->m_camera.move_right(right_step);
}

void MainWindow::on_actionLoadObject_triggered()
{
    QSettings settings;
    QString search_path = settings.value("path/last").toString();
    if (search_path.isNull())
        search_path = settings.value("path/assets").toString();

    const QString obj_file = QFileDialog::getOpenFileName(this, "Select Object", search_path,
                                                          "Wavefront Object File (*.obj)");
    if (obj_file.isNull())
        return;

    settings.setValue("path/last", QFileInfo(obj_file).absolutePath());

    m_glWindow->loadObject(obj_file);
}

void MainWindow::onRightMouseButtonPress()
{
    if (QEvent::MouseButtonPress == m_right_mouse_action->data())
    {
        QGuiApplication::setOverrideCursor({Qt::BlankCursor});
        m_mouse_location_backup = QCursor::pos();
        QCursor::setPos(geometry().center());
    }
    else if (QEvent::MouseButtonRelease == m_right_mouse_action->data())
    {
        QGuiApplication::restoreOverrideCursor();
        QCursor::setPos(m_mouse_location_backup);
    }
}
