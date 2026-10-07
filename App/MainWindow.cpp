#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QMessageBox>


#include "Configuration/ConfigurationManager.h"
#include "Configuration/C64Configuration.h"
#include "Configuration/C64KeyboardDefaults.h"
#include "Configuration/HostPlatform.h"

#include "Emulator/EmulatorController.h"
#include "Input/Keyboard/KeyboardController.h"
#include "Output/Video/VideoController.h"

#include "Gui/Video/VideoWindowWidget.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}
MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::initialize()
{
    m_ptrConfigurationManager = new ConfigurationManager(this);
    if (!m_ptrConfigurationManager->initialize())
    {
        QMessageBox::critical(this, tr("Configuration error"), tr("The configuration could not be initialized."));
        return false;
    }

    if (!validateConfiguration())
        return false;

    m_ptrEmulatorController = new EmulatorController(this);
    connect(m_ptrEmulatorController, &EmulatorController::romSetLoaded, this, &MainWindow::romSetLoaded);

    m_ptrVideoController = new VideoController(m_ptrEmulatorController->machine(), this);

    m_ptrVideoWindowWidget = new VideoWindowWidget();
    m_ptrVideoWindowWidget->setWindowTitle(QStringLiteral("C64Foundry"));

    m_ptrKeyboardController = new KeyboardController(m_ptrConfigurationManager, this);
    qApp->installEventFilter(m_ptrKeyboardController);
    connect(m_ptrKeyboardController, &KeyboardController::input, m_ptrEmulatorController, &EmulatorController::input);

    m_ptrVideoController->setVideoWidget(m_ptrVideoWindowWidget);
    m_ptrVideoWindowWidget->show();

    m_ptrEmulatorController->loadROMSet(m_ptrConfigurationManager->activeConfiguration()->romSet);

    ui->dockWidgetContents_EmulatorControl->setController(m_ptrEmulatorController);

    return true;
}

bool MainWindow::validateConfiguration()
{
    const C64Configuration* ptrConfiguration = m_ptrConfigurationManager->activeConfiguration();
    if (ptrConfiguration == nullptr)
    {
        QMessageBox::critical(this, tr("Configuration error"), tr("No active configuration is available."));
        return false;
    }

    const HostPlatform hostPlatform;

    if (ptrConfiguration->platform.type() != hostPlatform.type())
    {
        QMessageBox::critical(this, tr("Configuration platform mismatch"), tr("The active configuration was created for %1, but C64Foundry is currently running on %2.").arg(ptrConfiguration->platform.name(), hostPlatform.name()));
        return false;
    }

    if (!C64KeyboardDefaults::isSupported(hostPlatform.type()))
    {
        QMessageBox::critical(this, tr("Unsupported platform"), tr("Keyboard support for %1 is not available.").arg(hostPlatform.name()));
        return false;
    }

    return true;
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
