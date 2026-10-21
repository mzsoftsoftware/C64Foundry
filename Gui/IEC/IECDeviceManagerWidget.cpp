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

QString IECDeviceManagerWidget::deviceTypeName(IECDeviceType type) const
{
    switch (type)
    {
    case IECDeviceType::PrinterASCII:
        return tr("ASCII Printer");
    }

    return tr("Unknown Device");
}
void IECDeviceManagerWidget::setupAddMenu()
{
    QMenu* ptrMenu = new QMenu(ui->toolButton_Add);

    ui->toolButton_Add->setMenu(ptrMenu);
    ui->toolButton_Add->setPopupMode(QToolButton::InstantPopup);

    connect(ptrMenu, &QMenu::triggered, this, &IECDeviceManagerWidget::onAddMenuTriggered);
}
void IECDeviceManagerWidget::updateAddMenu()
{
    QMenu* ptrMenu = ui->toolButton_Add->menu();

    if (ptrMenu == nullptr)
        return;

    ptrMenu->clear();

    for (IECDeviceType type : m_availableDeviceTypes)
    {
        QAction* ptrAction = ptrMenu->addAction(deviceTypeName(type));

        ptrAction->setData(static_cast<int>(type));
        ptrAction->setEnabled(m_addableDeviceTypes.contains(type));
    }
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

    setBusy(true);
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
    QList<IECDeviceManager::ConfigurationPtr> sortedConfigurations;
    QHash<IECDeviceConfiguration::DeviceId, IECDeviceManager::ConfigurationPtr> newConfigurations;

    for (const IECDeviceManager::ConfigurationPtr& ptrConfiguration : configurations)
    {
        if (ptrConfiguration.isNull())
            continue;

        newConfigurations.insert(ptrConfiguration->id(), ptrConfiguration);

        qsizetype insertIndex = 0;

        while (insertIndex < sortedConfigurations.size() && sortedConfigurations[insertIndex]->deviceNumber() <= ptrConfiguration->deviceNumber())
            ++insertIndex;

        sortedConfigurations.insert(insertIndex, ptrConfiguration);
    }

    // Nicht mehr vorhandene Geräte entfernen.
    for (int index = ui->listWidget_Devices->count() - 1; index >= 0; --index)
    {
        QListWidgetItem* ptrItem = ui->listWidget_Devices->item(index);

        const IECDeviceConfiguration::DeviceId deviceId = ptrItem->data(Qt::UserRole).toULongLong();

        if (newConfigurations.contains(deviceId))
            continue;

        m_deviceRows.remove(deviceId);
        m_confirmedSetRequests.remove(deviceId);

        delete ui->listWidget_Devices->takeItem(index);
    }

    // Vorhandene Zeilen aktualisieren und neue Zeilen erzeugen.
    for (qsizetype index = 0; index < sortedConfigurations.size(); ++index)
    {
        const IECDeviceManager::ConfigurationPtr& ptrConfiguration = sortedConfigurations[index];
        const IECDeviceConfiguration::DeviceId deviceId = ptrConfiguration->id();

        IECDeviceRowWidget* ptrRow = m_deviceRows.value(deviceId, nullptr);
        QListWidgetItem* ptrItem = nullptr;

        if (ptrRow == nullptr)
        {
            ptrRow = new IECDeviceRowWidget();

            connect(ptrRow, &IECDeviceRowWidget::enabledChanged, this, &IECDeviceManagerWidget::onDeviceEnabledChanged);
            connect(ptrRow, &IECDeviceRowWidget::settingsRequested, this, &IECDeviceManagerWidget::deviceSettingsRequested);
            connect(ptrRow, &IECDeviceRowWidget::removeRequested, this, &IECDeviceManagerWidget::onDeviceRemoveRequested);

            ptrItem = new QListWidgetItem();
            ptrItem->setData(Qt::UserRole, QVariant::fromValue(deviceId));

            ui->listWidget_Devices->insertItem(static_cast<int>(index), ptrItem);
            ui->listWidget_Devices->setItemWidget(ptrItem, ptrRow);

            m_deviceRows.insert(deviceId, ptrRow);
        }
        else
        {
            for (int itemIndex = 0; itemIndex < ui->listWidget_Devices->count(); ++itemIndex)
            {
                QListWidgetItem* ptrCandidate = ui->listWidget_Devices->item(itemIndex);

                if (ptrCandidate->data(Qt::UserRole).toULongLong() == deviceId)
                {
                    ptrItem = ptrCandidate;
                    break;
                }
            }
        }

        if (ptrItem == nullptr)
            continue;

        // Bestehende Zeilen bleiben erhalten.
        ptrRow->setConfiguration(*ptrConfiguration);
        ptrItem->setSizeHint(ptrRow->sizeHint());

        // Erfolg erst nach Übernahme der bestätigten Konfiguration anzeigen.
        if (m_confirmedSetRequests.contains(deviceId))
        {
            ptrRow->setOperationState(IECOperationStatusWidget::State::Success);
            m_confirmedSetRequests.remove(deviceId);
            ptrRow->setBusy(false);
        }
    }

    m_configurations = newConfigurations;
    updateAddMenu();
    updateBusyState();
}

void IECDeviceManagerWidget::onRequestDone(QSharedPointer<EmulatorResponse> response)
{
    if (response.isNull())
        return;

    if (m_addRequestId != 0 && response->id() == m_addRequestId)
    {
        m_addRequestId = 0;

        setBusy(false);

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

    if (m_removeRequestId != 0 && response->id() == m_removeRequestId)
    {
        m_removeRequestId = 0;

        setBusy(false);

        if (response->success())
        {
            ui->widget_OperationStatus->setState(IECOperationStatusWidget::State::Success);
            refreshConfigurations();
        }
        else
        {
            ui->widget_OperationStatus->setState(IECOperationStatusWidget::State::Failed, tr("Could not remove IEC device"));
        }

        return;
    }

    if (m_setConfigurationRequests.contains(response->id()))
    {
        const IECDeviceConfiguration::DeviceId deviceId = m_setConfigurationRequests.take(response->id());

        IECDeviceRowWidget* ptrRow = m_deviceRows.value(deviceId, nullptr);

        if (response->success())
        {
            m_confirmedSetRequests.insert(deviceId, true);

            // Konfiguration abfragen. Danach wird Success angezeigt.
            refreshConfigurations();
        }
        else
        {
            m_confirmedSetRequests.remove(deviceId);

            if (ptrRow != nullptr)
            {
                ptrRow->setOperationState(IECOperationStatusWidget::State::Failed, tr("Could not update IEC device"));
                ptrRow->setBusy(false);
            }

            refreshConfigurations();
            updateBusyState();
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
            m_availableDeviceTypes = ptrResponse->availableDeviceTypes();
            m_addableDeviceTypes = ptrResponse->addableDeviceTypes();

            setConfigurations(ptrResponse->configurations());

            updateAddMenu();

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
void IECDeviceManagerWidget::onDeviceEnabledChanged(IECDeviceConfiguration::DeviceId deviceId, bool enabled)
{
    if (m_ptrController == nullptr)
        return;

    const IECDeviceManager::ConfigurationPtr ptrConfiguration = m_configurations.value(deviceId);
    IECDeviceRowWidget* ptrRow = m_deviceRows.value(deviceId, nullptr);

    if (ptrConfiguration.isNull() || ptrRow == nullptr)
        return;

    if (ptrConfiguration->enabled() == enabled)
        return;

    // Keine zweite Anfrage für dasselbe Gerät zulassen.
    if (m_setConfigurationRequests.values().contains(deviceId))
        return;

    QSharedPointer<IECDeviceConfiguration> ptrUpdatedConfiguration(ptrConfiguration->clone());
    ptrUpdatedConfiguration->setEnabled(enabled);

    const EmulatorRequest::RequestId requestId = m_ptrController->request(new EmulatorRequestSetIECConfiguration(*ptrUpdatedConfiguration));

    if (requestId == 0)
    {
        ptrRow->setOperationState(IECOperationStatusWidget::State::Failed, tr("Could not submit IEC configuration request"));
        return;
    }

    m_setConfigurationRequests.insert(requestId, deviceId);
    ptrRow->setBusy(true);
    ptrRow->setOperationState(IECOperationStatusWidget::State::Pending);
    updateBusyState();
}
void IECDeviceManagerWidget::onDeviceRemoveRequested(IECDeviceConfiguration::DeviceId deviceId)
{
    if (m_ptrController == nullptr || m_removeRequestId != 0 || m_addRequestId != 0)
        return;

    if (!m_configurations.contains(deviceId))
        return;

    m_removeRequestId = m_ptrController->request(new EmulatorRequestRemoveIECDevice(deviceId));

    if (m_removeRequestId == 0)
    {
        ui->widget_OperationStatus->setState(IECOperationStatusWidget::State::Failed, tr("Could not submit IEC remove request"));
        return;
    }

    setBusy(true);
    ui->widget_OperationStatus->setState(IECOperationStatusWidget::State::Pending);
}

void IECDeviceManagerWidget::on_pushButton_Refresh_clicked()
{
    refreshConfigurations();
}

void IECDeviceManagerWidget::clearDeviceRows()
{
    ui->listWidget_Devices->clear();

    m_availableDeviceTypes.clear();
    m_addableDeviceTypes.clear();
    m_deviceRows.clear();
    m_configurations.clear();
    m_setConfigurationRequests.clear();
    m_confirmedSetRequests.clear();

    updateAddMenu();
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

void IECDeviceManagerWidget::setBusy(bool busy)
{
    if (m_bBusy == busy)
        return;

    m_bBusy = busy;

    updateBusyState();
}

void IECDeviceManagerWidget::updateBusyState()
{
    const bool bDeviceOperationPending = !m_setConfigurationRequests.isEmpty() || !m_confirmedSetRequests.isEmpty();
    const bool bBusy = m_bBusy || bDeviceOperationPending;

    ui->toolButton_Add->setEnabled(!bBusy && m_ptrController != nullptr);
    ui->toolButton_Refresh->setEnabled(!bBusy && m_ptrController != nullptr);

    for (IECDeviceRowWidget* ptrRow : m_deviceRows)
    {
        if (ptrRow == nullptr)
            continue;

        ptrRow->setBusy(bBusy);
    }
}
