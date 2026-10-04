#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "Configuration/ConfigurationManager.h"
#include "Configuration/C64Configuration.h"
#include "Emulator/EmulatorController.h"
#include "Output/Video/VideoController.h"


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