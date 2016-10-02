#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "opengl_window.h"
#include "util.h"


MainWindow::MainWindow(QWidget* parent /*=0*/)
	: QMainWindow{parent}
	, m_glWindow{std::make_unique<OpenGLWindow>()}
	, m_ui{std::make_unique<Ui::MainWindow>()}
{
	m_ui->setupUi(this);

	QSurfaceFormat format;
	format.setSamples(1);
	format.setProfile(QSurfaceFormat::CoreProfile);
    format.setVersion(4, 3);
	DEBUG_CALL(format.setOption(QSurfaceFormat::DebugContext));

	m_glWindow->setFormat(format);
	m_glWindow->resize(1024, 768);

	QWidget* glContainer = QWidget::createWindowContainer(m_glWindow.get(), this);
	m_ui->mainLayout->replaceWidget(m_ui->glWidgetContainer, glContainer, Qt::FindDirectChildrenOnly);

	connect(m_ui->buttonSpin,      &QPushButton::clicked, m_glWindow.get(), &OpenGLWindow::setAnimating);
	connect(m_ui->buttonWireFrame, &QPushButton::clicked, m_glWindow.get(), &OpenGLWindow::showWireFrame);

	connect(m_glWindow.get(), &OpenGLWindow::frameTime, this, &MainWindow::showFrameTime);
}

MainWindow::~MainWindow() = default;


void MainWindow::showFrameTime(float time_in_ms)
{
	const QString text = QString("%1 ms").arg(time_in_ms);
	m_ui->frameTimeLabel->setText(text);
}
