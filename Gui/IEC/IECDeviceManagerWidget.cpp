#include "IECDeviceManagerWidget.h"
#include "ui_IECDeviceManagerWidget.h"

#include <QAction>
#include <QDebug>
#include <QMenu>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QToolButton>
#include <QVariant>

#include "Emulator/EmulatorController.h"
#include "C64/IEC/IECSystemRequest.h"
#include "C64/IEC/IECSystemResponse.h"
#include "C64/IEC/IECFoundryDriveConfiguration.h"
#include "C64/IEC/IECAsciiPrinterConfiguration.h"

#include "IECAsciiPrinterRowWidget.h"
#include "IECFoundryDriveRowWidget.h"


IECDeviceManagerWidget::IECDeviceManagerWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::IECDeviceManagerWidget)
{
    ui->setupUi(this);

    setupAddMenu();

    ui->treeWidget_Devices->setSelectionMode(QAbstractItemView::NoSelection);
    ui->treeWidget_Devices->setFocusPolicy(Qt::NoFocus);
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

    updateControls();

    if (m_ptrController == nullptr)
        return;

    connect(m_ptrController, &EmulatorController::iecRequestDone,
            this, &IECDeviceManagerWidget::onIECRequestDone);

    connect(m_ptrController, &EmulatorController::iecSystemEventsChanged,
            this, &IECDeviceManagerWidget::onIECSystemEventsChanged);

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

    case IECDeviceType::FoundryDrive:
        return tr("Foundry Drive");
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
    {
        return;
    }

    const IECDeviceType type =
        static_cast<IECDeviceType>(ptrAction->data().toInt());

    m_addRequestId =
        m_ptrController->request(new IECSystemAddDeviceRequest(type));

    if (m_addRequestId == 0)
    {
        ui->widget_OperationStatus->setState(
            IECOperationStatusWidget::State::Failed,
            tr("Could not submit IEC device request"));

        return;
    }

    updateControls();

    ui->widget_OperationStatus->setState(
        IECOperationStatusWidget::State::Pending);
}


void IECDeviceManagerWidget::on_toolButton_Refresh_clicked()
{
    refreshConfigurations();
}


void IECDeviceManagerWidget::refreshConfigurations()
{
    if (m_ptrController == nullptr)
        return;

    if (m_configurationRequestId != 0 || m_addRequestId != 0)
    {
        m_bRefreshPending = true;
        return;
    }

    m_configurationRequestId =
        m_ptrController->request(new IECSystemGetConfigurationsRequest());

    if (m_configurationRequestId == 0)
    {
        qWarning() << "IECDeviceManagerWidget: could not submit configuration request";

        ui->widget_OperationStatus->setState(
            IECOperationStatusWidget::State::Failed,
            tr("Could not refresh IEC devices"));

        updateControls();
        return;
    }

    updateControls();

    ui->widget_OperationStatus->setState(
        IECOperationStatusWidget::State::Pending);
}


void IECDeviceManagerWidget::onIECRequestDone(QSharedPointer<IECResponse> response)
{
    if (response.isNull())
        return;

    if (m_addRequestId != 0 && response->requestId() == m_addRequestId)
    {
        m_addRequestId = 0;

        const IECSystemAddDeviceResponse* ptrAddResponse =
            dynamic_cast<const IECSystemAddDeviceResponse*>(response.data());

        if (response->success() && ptrAddResponse != nullptr &&
            ptrAddResponse->deviceId() != 0)
        {
            ui->widget_OperationStatus->setState(
                IECOperationStatusWidget::State::Pending);

            updateControls();
            refreshConfigurations();
            return;
        }

        ui->widget_OperationStatus->setState(
            IECOperationStatusWidget::State::Failed,
            tr("Could not add IEC device"));

        updateControls();
        return;
    }

    if (m_configurationRequestId == 0 ||
        response->requestId() != m_configurationRequestId)
    {
        return;
    }

    m_configurationRequestId = 0;

    const IECSystemGetConfigurationsResponse* ptrResponse =
        dynamic_cast<const IECSystemGetConfigurationsResponse*>(response.data());

    if (response->success() && ptrResponse != nullptr)
    {
        setConfigurations(ptrResponse);

        m_availableDeviceTypes = ptrResponse->availableDeviceTypes();
        m_addableDeviceTypes = ptrResponse->addableDeviceTypes();

        updateAddMenu();

        ui->widget_OperationStatus->setState(
            IECOperationStatusWidget::State::Success);
    }
    else
    {
        ui->widget_OperationStatus->setState(
            IECOperationStatusWidget::State::Failed,
            tr("Could not refresh IEC devices"));
    }

    updateControls();

    if (m_bRefreshPending)
    {
        m_bRefreshPending = false;
        refreshConfigurations();
    }
}


void IECDeviceManagerWidget::onIECSystemEventsChanged(IECSystemEventList events)
{
    for (const IECSystemEventPtr& ptrEvent : events)
    {
        if (ptrEvent.isNull())
            continue;

        switch (ptrEvent->type)
        {
        case IECSystemEvent::Type::DeviceAdded:
        case IECSystemEvent::Type::DeviceRemoved:
        case IECSystemEvent::Type::ConfigurationChanged:
        case IECSystemEvent::Type::ConfigurationsRestored:
            refreshConfigurations();
            return;

        case IECSystemEvent::Type::Reset:
            break;
        }
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

    case IECDeviceType::FoundryDrive:
        ptrRow = new IECFoundryDriveRowWidget();
        break;
    }

    if (ptrRow != nullptr)
        ptrRow->setController(m_ptrController);

    return ptrRow;
}


quint8 IECDeviceManagerWidget::deviceStartNumber(
    const IECDeviceConfiguration& configuration) const
{
    if (configuration.type() == IECDeviceType::PrinterASCII)
    {
        const IECAsciiPrinterConfiguration* ptrPrinterConfiguration =
            dynamic_cast<const IECAsciiPrinterConfiguration*>(&configuration);

        if (ptrPrinterConfiguration != nullptr)
            return ptrPrinterConfiguration->deviceNumber();
    }

    if (configuration.type() == IECDeviceType::FoundryDrive)
    {
        const IECFoundryDriveConfiguration* ptrDriveConfiguration =
            dynamic_cast<const IECFoundryDriveConfiguration*>(&configuration);

        if (ptrDriveConfiguration != nullptr)
            return ptrDriveConfiguration->startDeviceNumber();
    }

    return 255;
}


void IECDeviceManagerWidget::setConfigurations(
    const IECSystemGetConfigurationsResponse* ptrResponse)
{
    if (ptrResponse == nullptr)
        return;

    QList<QSharedPointer<IECDeviceConfiguration>> sortedConfigurations;

    for (const QSharedPointer<IECDeviceConfiguration>& ptrConfiguration :
         ptrResponse->configurations())
    {
        if (ptrConfiguration.isNull())
            continue;

        const quint8 deviceNumber = deviceStartNumber(*ptrConfiguration);

        qsizetype insertIndex = 0;

        while (insertIndex < sortedConfigurations.size() &&
               deviceStartNumber(*sortedConfigurations[insertIndex]) <= deviceNumber)
        {
            ++insertIndex;
        }

        sortedConfigurations.insert(insertIndex, ptrConfiguration);
    }

    clearDeviceList();

    for (const QSharedPointer<IECDeviceConfiguration>& ptrConfiguration :
         sortedConfigurations)
    {
        IECDeviceRowWidget* ptrRow = createDeviceRow(ptrConfiguration->type());

        if (ptrRow == nullptr)
        {
            qWarning() << "IECDeviceManagerWidget: unsupported device type"
                       << static_cast<int>(ptrConfiguration->type());

            continue;
        }

        ptrRow->setConfiguration(*ptrConfiguration);

        QTreeWidgetItem* ptrItem = new QTreeWidgetItem();

        ptrItem->setData(0, Qt::UserRole,
                         QVariant::fromValue(ptrConfiguration->id()));

        ui->treeWidget_Devices->addTopLevelItem(ptrItem);
        ui->treeWidget_Devices->setItemWidget(ptrItem, 0, ptrRow);
        ptrItem->setSizeHint(0, ptrRow->sizeHint());

        m_deviceRows.insert(ptrConfiguration->id(), ptrRow);
    }

    ui->treeWidget_Devices->expandAll();
}


void IECDeviceManagerWidget::clearDeviceRows()
{
    clearDeviceList();

    m_availableDeviceTypes.clear();
    m_addableDeviceTypes.clear();

    updateAddMenu();
}


void IECDeviceManagerWidget::clearDeviceList()
{
    while (ui->treeWidget_Devices->topLevelItemCount() > 0)
    {
        QTreeWidgetItem* ptrItem =
            ui->treeWidget_Devices->takeTopLevelItem(0);

        QWidget* ptrWidget = ui->treeWidget_Devices->itemWidget(ptrItem, 0);

        ui->treeWidget_Devices->removeItemWidget(ptrItem, 0);

        delete ptrWidget;
        delete ptrItem;
    }

    m_deviceRows.clear();
}


void IECDeviceManagerWidget::updateControls()
{
    const bool bAvailable = m_ptrController != nullptr;
    const bool bBusy = m_addRequestId != 0 || m_configurationRequestId != 0;

    ui->toolButton_Add->setEnabled(bAvailable && !bBusy);
    ui->toolButton_Refresh->setEnabled(bAvailable && !bBusy);
}
