#include "IECDeviceManagerWidget.h"
#include "ui_IECDeviceManagerWidget.h"

#include <QAction>
#include <QDebug>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QToolButton>
#include <QVariant>

#include "Emulator/EmulatorController.h"
#include "IECDeviceRowWidget.h"
#include "IECAsciiPrinterRowWidget.h"


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

void IECDeviceManagerWidget::setController(EmulatorController* ptrController)
{
    if (m_ptrController == ptrController)
        return;

    if (m_ptrController != nullptr)
        disconnect(m_ptrController, nullptr, this, nullptr);

    m_ptrController = ptrController;
    m_addRequestId = 0;
    m_configurationRequestId = 0;
    m_bRefreshPending = false;

    clearDeviceRows();

    ui->widget_OperationStatus->setState(IECOperationStatusWidget::State::Idle);
    ui->toolButton_Add->setEnabled(m_ptrController != nullptr);
    ui->toolButton_Refresh->setEnabled(m_ptrController != nullptr);

    if (m_ptrController == nullptr)
        return;

    connect(m_ptrController, &EmulatorController::requestDone,
            this, &IECDeviceManagerWidget::onRequestDone);

    connect(m_ptrController, &EmulatorController::iecDeviceChanged,
            this, &IECDeviceManagerWidget::onIECDeviceChanged);

    refreshConfigurations();
}

void IECDeviceManagerWidget::setupAddMenu()
{
    QMenu* ptrMenu = new QMenu(ui->toolButton_Add);

    ui->toolButton_Add->setMenu(ptrMenu);
    ui->toolButton_Add->setPopupMode(QToolButton::InstantPopup);

    connect(ptrMenu, &QMenu::triggered,
            this, &IECDeviceManagerWidget::onAddMenuTriggered);
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
    if (m_ptrController == nullptr || ptrAction == nullptr ||
        m_addRequestId != 0 || m_configurationRequestId != 0)
        return;

    const IECDeviceType type = static_cast<IECDeviceType>(ptrAction->data().toInt());

    m_addRequestId = m_ptrController->request(new EmulatorRequestAddIECDevice(type));

    if (m_addRequestId == 0)
    {
        ui->widget_OperationStatus->setState(
            IECOperationStatusWidget::State::Failed,
            tr("Could not submit IEC device request"));
        return;
    }

    ui->toolButton_Add->setEnabled(false);
    ui->toolButton_Refresh->setEnabled(false);
    ui->widget_OperationStatus->setState(IECOperationStatusWidget::State::Pending);
}

void IECDeviceManagerWidget::on_pushButton_Refresh_clicked()
{
    refreshConfigurations();
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

    m_configurationRequestId =
        m_ptrController->request(new EmulatorRequestGetIECConfigurations());

    if (m_configurationRequestId == 0)
    {
        qWarning() << "IECDeviceManagerWidget: could not submit configuration request";

        ui->widget_OperationStatus->setState(
            IECOperationStatusWidget::State::Failed,
            tr("Could not refresh IEC devices"));

        ui->toolButton_Add->setEnabled(m_ptrController != nullptr && m_addRequestId == 0);
        ui->toolButton_Refresh->setEnabled(m_ptrController != nullptr && m_addRequestId == 0);
        return;
    }

    ui->toolButton_Add->setEnabled(false);
    ui->toolButton_Refresh->setEnabled(false);
    ui->widget_OperationStatus->setState(IECOperationStatusWidget::State::Pending);
}

void IECDeviceManagerWidget::onRequestDone(QSharedPointer<EmulatorResponse> response)
{
    if (response.isNull())
        return;

    if (response->id() == m_addRequestId && m_addRequestId != 0)
    {
        m_addRequestId = 0;

        if (response->success())
        {
            ui->widget_OperationStatus->setState(IECOperationStatusWidget::State::Pending);
            refreshConfigurations();
        }
        else
        {
            ui->widget_OperationStatus->setState(
                IECOperationStatusWidget::State::Failed,
                tr("Could not add IEC device"));

            ui->toolButton_Add->setEnabled(true);
            ui->toolButton_Refresh->setEnabled(true);
        }

        return;
    }

    if (response->id() != m_configurationRequestId || m_configurationRequestId == 0)
        return;

    m_configurationRequestId = 0;

    bool bSuccess = false;

    if (response->type() == EmulatorRequest::Type::GetIECConfigurations &&
        response->success())
    {
        const EmulatorResponseIEC* ptrResponse =
            dynamic_cast<const EmulatorResponseIEC*>(response.data());

        if (ptrResponse != nullptr)
        {
            m_availableDeviceTypes = ptrResponse->availableDeviceTypes();
            m_addableDeviceTypes = ptrResponse->addableDeviceTypes();

            setConfigurations(ptrResponse->configurations());
            updateAddMenu();

            bSuccess = true;
        }
    }

    if (bSuccess)
    {
        if (m_addRequestId == 0)
            ui->widget_OperationStatus->setState(IECOperationStatusWidget::State::Success);
    }
    else
    {
        ui->widget_OperationStatus->setState(
            IECOperationStatusWidget::State::Failed,
            tr("Could not refresh IEC devices"));
    }

    ui->toolButton_Add->setEnabled(m_ptrController != nullptr && m_addRequestId == 0);
    ui->toolButton_Refresh->setEnabled(m_ptrController != nullptr && m_addRequestId == 0);

    if (m_bRefreshPending)
    {
        m_bRefreshPending = false;
        refreshConfigurations();
    }
}

void IECDeviceManagerWidget::onIECDeviceChanged(
    EmulatorIECDeviceEvent::Type type,
    IECDeviceConfiguration::DeviceId deviceId)
{
    Q_UNUSED(deviceId)

    switch (type)
    {
    case EmulatorIECDeviceEvent::Type::Added:
    case EmulatorIECDeviceEvent::Type::Removed:
    case EmulatorIECDeviceEvent::Type::ConfigurationChanged:
    case EmulatorIECDeviceEvent::Type::Restored:
        refreshConfigurations();
        break;

    case EmulatorIECDeviceEvent::Type::Reset:
        break;
    }
}

IECDeviceRowWidget* IECDeviceManagerWidget::createDeviceRow(IECDeviceType type)
{
    IECDeviceRowWidget* ptrRow = nullptr;

    switch (type)
    {
    case IECDeviceType::PrinterASCII:
        ptrRow = new IECAsciiPrinterRowWidget();
        break;
    }

    if (ptrRow != nullptr)
        ptrRow->setController(m_ptrController);

    return ptrRow;
}

void IECDeviceManagerWidget::setConfigurations(
    const QList<IECDeviceManager::ConfigurationPtr>& configurations)
{
    QList<IECDeviceManager::ConfigurationPtr> sortedConfigurations;

    for (const IECDeviceManager::ConfigurationPtr& ptrConfiguration : configurations)
    {
        if (ptrConfiguration.isNull())
            continue;

        qsizetype insertIndex = 0;

        while (insertIndex < sortedConfigurations.size() &&
               sortedConfigurations[insertIndex]->deviceNumber() <= ptrConfiguration->deviceNumber())
            ++insertIndex;

        sortedConfigurations.insert(insertIndex, ptrConfiguration);
    }

    QHash<IECDeviceConfiguration::DeviceId, bool> activeIds;

    for (const IECDeviceManager::ConfigurationPtr& ptrConfiguration : sortedConfigurations)
        activeIds.insert(ptrConfiguration->id(), true);

    for (int index = ui->listWidget_Devices->count() - 1; index >= 0; --index)
    {
        QListWidgetItem* ptrItem = ui->listWidget_Devices->item(index);
        const IECDeviceConfiguration::DeviceId deviceId =
            ptrItem->data(Qt::UserRole).toULongLong();

        if (activeIds.contains(deviceId))
            continue;

        m_deviceRows.remove(deviceId);
        delete ui->listWidget_Devices->takeItem(index);
    }

    for (qsizetype index = 0; index < sortedConfigurations.size(); ++index)
    {
        const IECDeviceManager::ConfigurationPtr& ptrConfiguration = sortedConfigurations[index];
        const IECDeviceConfiguration::DeviceId deviceId = ptrConfiguration->id();

        IECDeviceRowWidget* ptrRow = m_deviceRows.value(deviceId, nullptr);
        QListWidgetItem* ptrItem = nullptr;

        for (int itemIndex = 0; itemIndex < ui->listWidget_Devices->count(); ++itemIndex)
        {
            QListWidgetItem* ptrCandidate = ui->listWidget_Devices->item(itemIndex);

            if (ptrCandidate->data(Qt::UserRole).toULongLong() == deviceId)
            {
                ptrItem = ptrCandidate;
                break;
            }
        }

        if (ptrRow == nullptr)
        {
            ptrRow = createDeviceRow(ptrConfiguration->type());

            if (ptrRow == nullptr)
            {
                qWarning() << "IECDeviceManagerWidget: unsupported device type"
                           << static_cast<int>(ptrConfiguration->type());
                continue;
            }

            ptrItem = new QListWidgetItem();
            ptrItem->setData(Qt::UserRole, QVariant::fromValue(deviceId));

            ui->listWidget_Devices->insertItem(static_cast<int>(index), ptrItem);
            ui->listWidget_Devices->setItemWidget(ptrItem, ptrRow);

            m_deviceRows.insert(deviceId, ptrRow);
        }
        else if (ptrItem != nullptr)
        {
            const int currentIndex = ui->listWidget_Devices->row(ptrItem);

            if (currentIndex != static_cast<int>(index))
            {
                ptrItem = ui->listWidget_Devices->takeItem(currentIndex);
                ui->listWidget_Devices->insertItem(static_cast<int>(index), ptrItem);
            }
        }

        ptrRow->setConfiguration(*ptrConfiguration);
        ptrItem->setSizeHint(ptrRow->sizeHint());
    }
}

void IECDeviceManagerWidget::clearDeviceRows()
{
    ui->listWidget_Devices->clear();
    m_deviceRows.clear();
    m_availableDeviceTypes.clear();
    m_addableDeviceTypes.clear();

    updateAddMenu();
}
