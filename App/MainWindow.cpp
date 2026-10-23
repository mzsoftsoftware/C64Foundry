#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QSettings>
#include <QMessageBox>
#include <QDebug>

#include "Configuration/ConfigurationManager.h"
#include "Configuration/C64Configuration.h"
#include "Configuration/C64KeyboardDefaults.h"
#include "Configuration/HostPlatform.h"

#include "Emulator/EmulatorController.h"
#include "Input/Keyboard/KeyboardController.h"
#include "Output/Video/VideoController.h"

#include "Storage/PRG/C64PRGFile.h"


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
    connect(m_ptrEmulatorController, &EmulatorController::requestDone, this, &MainWindow::onIECConfigurationsRestored);

    ui->dockWidgetContents_EmulatorControl->setController(m_ptrEmulatorController);
    ui->dockWidgetContents_IECControl->setController(m_ptrEmulatorController);
    connect(m_ptrEmulatorController, &EmulatorController::romSetLoaded, this, &MainWindow::romSetLoaded);

    m_ptrVideoController = new VideoController(m_ptrEmulatorController->machine(), this);
    m_ptrVideoController->setVideoWidget(ui->dockWidgetContents_VideoWindow);

    m_ptrKeyboardController = new KeyboardController(m_ptrConfigurationManager, this);
    qApp->installEventFilter(m_ptrKeyboardController);
    connect(m_ptrKeyboardController, &KeyboardController::input, m_ptrEmulatorController, &EmulatorController::input);

    ui->dockWidgetContents_VirtualKeyboard->setMode(C64KeyboardWidget::Mode::Input);
    connect(ui->dockWidgetContents_VirtualKeyboard, &C64KeyboardWidget::keyPressed, m_ptrKeyboardController, &KeyboardController::virtualKeyPressed);
    connect(ui->dockWidgetContents_VirtualKeyboard, &C64KeyboardWidget::keyReleased, m_ptrKeyboardController, &KeyboardController::virtualKeyReleased);
    connect(m_ptrKeyboardController, &KeyboardController::keyStateChanged, ui->dockWidgetContents_VirtualKeyboard, &C64KeyboardWidget::setKeyboardKeyPressed);
    connect(m_ptrKeyboardController, &KeyboardController::keyboardStateReset, ui->dockWidgetContents_VirtualKeyboard, &C64KeyboardWidget::resetKeyboardState);

    setupDockWidgetsMenu();
    restoreWindowLayout();

    m_restoreIECRequestId = m_ptrEmulatorController->restoreIECConfigurations(m_ptrConfigurationManager->activeConfiguration()->iecDevices);

    if (m_restoreIECRequestId == 0)
    {
        QMessageBox::critical(this, tr("IEC configuration error"), tr("Could not submit the IEC configuration restore request."));
        return false;
    }

    C64PRGFile prgFile;

    if (prgFile.load(QStringLiteral("/tmp/BasicSys.prg")))
    {
        qDebug() << "PRG load address:" << Qt::hex << prgFile.loadAddress();
        qDebug() << "PRG data size:" << Qt::dec << prgFile.data().size();
        qDebug() << "BASIC detected:" << prgFile.isBasicProgram();

        if (prgFile.isBasicProgram())
            qDebug() << "BASIC end address:" << Qt::hex << prgFile.basicEndAddress();
    }
    else
    {
        qWarning() << "PRG loading failed:" << prgFile.errorString();
    }

    return true;
}

void MainWindow::setupDockWidgetsMenu()
{
    QAction* ptrSeparator = ui->menuView->actions().first();

    ui->menuView->insertAction(ptrSeparator, ui->dockWidget_EmulatorControl->toggleViewAction());
    ui->menuView->insertAction(ptrSeparator, ui->dockWidget_IECControl->toggleViewAction());
    ui->menuView->insertAction(ptrSeparator, ui->dockWidget_Video->toggleViewAction());
    ui->menuView->insertAction(ptrSeparator, ui->dockWidget_VirtualKeyboard->toggleViewAction());

    connect(ui->actionResetWindowLayout, &QAction::triggered, this, &MainWindow::resetWindowLayout);
}

void MainWindow::saveWindowLayout()
{
    QSettings settings;

    settings.beginGroup(QStringLiteral("MainWindow"));
    settings.setValue(QStringLiteral("Geometry"), saveGeometry());
    settings.setValue(QStringLiteral("State"), saveState());
    settings.endGroup();
}
void MainWindow::restoreWindowLayout()
{
    QSettings settings;

    settings.beginGroup(QStringLiteral("MainWindow"));

    if (settings.contains(QStringLiteral("Geometry")))
        restoreGeometry(settings.value(QStringLiteral("Geometry")).toByteArray());

    if (settings.contains(QStringLiteral("State")))
        restoreState(settings.value(QStringLiteral("State")).toByteArray());

    settings.endGroup();
}
void MainWindow::resetWindowLayout()
{
    ui->dockWidget_EmulatorControl->setFloating(false);
    ui->dockWidget_IECControl->setFloating(false);
    ui->dockWidget_Video->setFloating(false);
    ui->dockWidget_VirtualKeyboard->setFloating(false);

    addDockWidget(Qt::TopDockWidgetArea, ui->dockWidget_EmulatorControl);
    splitDockWidget(ui->dockWidget_EmulatorControl, ui->dockWidget_IECControl, Qt::Horizontal);
    addDockWidget(Qt::RightDockWidgetArea, ui->dockWidget_Video);
    addDockWidget(Qt::BottomDockWidgetArea, ui->dockWidget_VirtualKeyboard);

    ui->dockWidget_EmulatorControl->show();
    ui->dockWidget_IECControl->show();
    ui->dockWidget_Video->show();
    ui->dockWidget_VirtualKeyboard->show();
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
    saveWindowLayout();

    QApplication::quit();
    QMainWindow::closeEvent(ptrEvent);
}

void MainWindow::onIECConfigurationsRestored(QSharedPointer<EmulatorResponse> response)
{
    if (response.isNull())
        return;

    if (m_restoreIECRequestId == 0)
        return;

    if (response->id() != m_restoreIECRequestId)
        return;

    if (response->type() != EmulatorRequest::Type::RestoreIECConfigurations)
        return;

    m_restoreIECRequestId = 0;

    if (!response->success())
    {
        QMessageBox::critical(this, tr("IEC configuration error"), tr("The saved IEC device configurations could not be restored."));

        QApplication::quit();
        return;
    }

    qDebug() << "MainWindow: IEC configurations restored successfully";
    m_ptrConfigurationManager->startIECSynchronization(m_ptrEmulatorController);
    m_ptrEmulatorController->loadROMSet(m_ptrConfigurationManager->activeConfiguration()->romSet);
}
