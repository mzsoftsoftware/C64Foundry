#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "Configuration/ConfigurationManager.h"
#include "Configuration/C64Configuration.h"
#include "Emulator/EmulatorController.h"
#include "Input/Keyboard/KeyboardController.h"
#include "Output/Video/VideoController.h"
#include "Gui/Video/VideoWindowWidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_ptrConfigurationManager = new ConfigurationManager(this);
    if (!m_ptrConfigurationManager->initialize())
    {
        // Error handling later.
    }

    m_ptrEmulatorController = new EmulatorController(this);
    connect(m_ptrEmulatorController, &EmulatorController::romSetLoaded, this, &MainWindow::romSetLoaded);

    m_ptrVideoController = new VideoController(m_ptrEmulatorController->machine(), this);

    m_ptrVideoWindowWidget = new VideoWindowWidget();
    m_ptrVideoWindowWidget->setWindowTitle(QStringLiteral("C64Foundry"));

    m_ptrKeyboardController = new KeyboardController(this);
    connect(m_ptrVideoWindowWidget, &VideoWindowWidget::keyPressed, m_ptrKeyboardController, &KeyboardController::keyPressed);
    connect(m_ptrVideoWindowWidget, &VideoWindowWidget::keyReleased, m_ptrKeyboardController, &KeyboardController::keyReleased);
    connect(m_ptrVideoWindowWidget, &VideoWindowWidget::inputDeactivated, m_ptrKeyboardController, &KeyboardController::inputDeactivated);
    connect(m_ptrKeyboardController, &KeyboardController::input, m_ptrEmulatorController, &EmulatorController::input);

    m_ptrVideoController->setVideoWidget(m_ptrVideoWindowWidget);
    m_ptrVideoWindowWidget->show();

    m_ptrEmulatorController->loadROMSet(m_ptrConfigurationManager->activeConfiguration()->romSet);

    ui->dockWidgetContents_EmulatorControl->setController(m_ptrEmulatorController);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::romSetLoaded(const bool bLoaded)
{
    if (!bLoaded)
        qWarning() << "MainWindow: ROM set could not be loaded";
}

void MainWindow::closeEvent(QCloseEvent* ptrEvent)
{
    QApplication::quit();
    QMainWindow::closeEvent(ptrEvent);
}
