#include "IECDeviceManagerWidget.h"
#include "ui_IECDeviceManagerWidget.h"

#include <QDebug>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QAction>
#include <QVariant>
#include <QToolButton>

#include "Emulator/EmulatorController.h"

#include "IECDeviceRowWidget.h"


IECDeviceManagerWidget::IECDeviceManagerWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::IECDeviceManagerWidget)
{
    ui->setupUi(this);

    setupAddMenu();
    ui->listWidget_Devices->setSelectionMode(QAbstractItemView::NoSelection);
    ui->listWidget_Devices->setFocusPolicy(Qt::NoFocus);
}

IECDeviceManagerWidget::~IECDeviceManagerWidget()
{
    delete ui;
}

void IECDeviceManagerWidget::setupAddMenu()
{
    QMenu* ptrMenu = new QMenu(ui->toolButton_Add);

    QAction* ptrASCIIPrinterAction = ptrMenu->addAction(tr("ASCII Printer"));
    ptrASCIIPrinterAction->setData(static_cast<int>(IECDeviceType::PrinterASCII));

    ui->toolButton_Add->setMenu(ptrMenu);
    ui->toolButton_Add->setPopupMode(QToolButton::InstantPopup);

    connect(ptrMenu, &QMenu::triggered, this, &IECDeviceManagerWidget::onAddMenuTriggered);
}
void IECDeviceManagerWidget::onAddMenuTriggered(QAction* ptrAction)
{
    if (m_ptrController == nullptr || ptrAction == nullptr || m_addRequestId != 0)
        return;

    const IECDeviceType type = static_cast<IECDeviceType>(ptrAction->data().toInt());

    m_addRequestId = m_ptrController->request(new EmulatorRequestAddIECDevice(type));

    if (m_addRequestId == 0)
    {
        ui->widget_OperationStatus->setState(IECOperationStatusWidget::State::Failed, tr("Could not submit IEC device request"));
        return;
    }

    ui->toolButton_Add->setEnabled(false);
    ui->widget_OperationStatus->setState(IECOperationStatusWidget::State::Pending);
}

void IECDeviceManagerWidget::setController(EmulatorController* ptrController)
{
    if (m_ptrController == ptrController)
        return;

    if (m_ptrController != nullptr)
        disconnect(m_ptrController, nullptr, this, nullptr);

    m_ptrController = ptrController;
    m_configurationRequestId = 0;
    m_bRefreshPending = false;

    clearDeviceRows();

    if (m_ptrController == nullptr)
        return;

    connect(m_ptrController, &EmulatorController::requestDone, this, &IECDeviceManagerWidget::onRequestDone);
    connect(m_ptrController, &EmulatorController::iecDeviceChanged, this, &IECDeviceManagerWidget::onIECDeviceChanged);

    refreshConfigurations();
}
void IECDeviceManagerWidget::setConfigurations(const QList<IECDeviceManager::ConfigurationPtr>& configurations)
{
    clearDeviceRows();

    // Konfigurationen nach IEC-Gerätenummer sortiert einfügen.
    QList<IECDeviceManager::ConfigurationPtr> sortedConfigurations;

    for (const IECDeviceManager::ConfigurationPtr& ptrConfiguration : configurations)
    {
        if (ptrConfiguration.isNull())
            continue;

        qsizetype insertIndex = 0;

        while (insertIndex < sortedConfigurations.size() && sortedConfigurations[insertIndex]->deviceNumber() <= ptrConfiguration->deviceNumber())
            ++insertIndex;

        sortedConfigurations.insert(insertIndex, ptrConfiguration);
    }

    // Gerätezeilen erstellen.
    for (const IECDeviceManager::ConfigurationPtr& ptrConfiguration : sortedConfigurations)
    {
        IECDeviceRowWidget* ptrRow = new IECDeviceRowWidget();

        ptrRow->setConfiguration(*ptrConfiguration);

        connect(ptrRow, &IECDeviceRowWidget::enabledChanged, this, &IECDeviceManagerWidget::deviceEnabledChanged);
        connect(ptrRow, &IECDeviceRowWidget::settingsRequested, this, &IECDeviceManagerWidget::deviceSettingsRequested);
        connect(ptrRow, &IECDeviceRowWidget::removeRequested, this, &IECDeviceManagerWidget::deviceRemoveRequested);

        QListWidgetItem* ptrItem = new QListWidgetItem();

        ptrItem->setData(Qt::UserRole, QVariant::fromValue(ptrConfiguration->id()));
        ptrItem->setSizeHint(ptrRow->sizeHint());

        ui->listWidget_Devices->addItem(ptrItem);
        ui->listWidget_Devices->setItemWidget(ptrItem, ptrRow);

        m_deviceRows.insert(ptrConfiguration->id(), ptrRow);
    }
}

void IECDeviceManagerWidget::onRequestDone(QSharedPointer<EmulatorResponse> response)
{
    if (response.isNull())
        return;

    if (m_addRequestId != 0 && response->id() == m_addRequestId)
    {
        m_addRequestId = 0;

        ui->toolButton_Add->setEnabled(true);

        if (response->success())
        {
            ui->widget_OperationStatus->setState(IECOperationStatusWidget::State::Success);
            refreshConfigurations();
        }
        else
        {
            ui->widget_OperationStatus->setState(IECOperationStatusWidget::State::Failed, tr("Could not add IEC device"));
        }

        return;
    }

    if (response->id() != m_configurationRequestId || m_configurationRequestId == 0)
        return;

    m_configurationRequestId = 0;

    if (response->type() == EmulatorRequest::Type::GetIECConfigurations && response->success())
    {
        const EmulatorResponseIEC* ptrResponse = dynamic_cast<const EmulatorResponseIEC*>(response.data());

        if (ptrResponse != nullptr)
        {
            setConfigurations(ptrResponse->configurations());

            qDebug() << "IECDeviceManagerWidget: configurations received:" << ptrResponse->configurations().size();
        }
    }
    else
    {
        qWarning() << "IECDeviceManagerWidget: failed to retrieve IEC configurations";
    }

    if (m_bRefreshPending)
    {
        m_bRefreshPending = false;
        refreshConfigurations();
    }
}

void IECDeviceManagerWidget::onIECDeviceChanged(EmulatorIECDeviceEvent::Type type, IECDeviceConfiguration::DeviceId deviceId)
{
    Q_UNUSED(deviceId)

    switch (type)
    {
    case EmulatorIECDeviceEvent::Type::Added:
    case EmulatorIECDeviceEvent::Type::Removed:
    case EmulatorIECDeviceEvent::Type::ConfigurationChanged:
        refreshConfigurations();
        break;

    case EmulatorIECDeviceEvent::Type::Reset:
        // Reset verändert die Geräteliste nicht.
        // Gerätespezifische Statusanzeigen behandeln wir später.
        break;
    }
}

void IECDeviceManagerWidget::on_pushButton_Refresh_clicked()
{
    refreshConfigurations();
}

void IECDeviceManagerWidget::clearDeviceRows()
{
    ui->listWidget_Devices->clear();
    m_deviceRows.clear();
}
void IECDeviceManagerWidget::refreshConfigurations()
{
    if (m_ptrController == nullptr)
        return;

    if (m_configurationRequestId != 0)
    {
        m_bRefreshPending = true;
        return;
    }

    m_configurationRequestId = m_ptrController->request(new EmulatorRequestGetIECConfigurations());
}
