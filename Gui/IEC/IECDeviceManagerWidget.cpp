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
#include "IECAsciiPrinterRowWidget.h"
#include "IECFoundryDiskRowWidget.h"


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
    case IECDeviceType::FoundryDisk:
        return tr("Foundry Disk");
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

            setConfigurations(ptrResponse->configurations(), ptrResponse->diskImageLoaded());
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
    case IECDeviceType::FoundryDisk:
        ptrRow = new IECFoundryDiskRowWidget();
        break;
    }

    if (ptrRow != nullptr)
        ptrRow->setController(m_ptrController);

    return ptrRow;
}

void IECDeviceManagerWidget::setConfigurations(const QList<IECDeviceManager::ConfigurationPtr>& configurations, const QHash<IECDeviceConfiguration::DeviceId, bool>& diskImageLoaded)
{
    QList<IECDeviceManager::ConfigurationPtr> sortedConfigurations;

    for (const IECDeviceManager::ConfigurationPtr& ptrConfiguration : configurations)
    {
        if (ptrConfiguration.isNull())
            continue;

        qsizetype insertIndex = 0;

        while (insertIndex < sortedConfigurations.size() &&
               sortedConfigurations[insertIndex]->deviceNumber() <= ptrConfiguration->deviceNumber())
        {
            ++insertIndex;
        }

        sortedConfigurations.insert(insertIndex, ptrConfiguration);
    }

    for (int index = ui->listWidget_Devices->count() - 1; index >= 0; --index)
    {
        QListWidgetItem* ptrItem = ui->listWidget_Devices->item(index);
        QWidget* ptrWidget = ui->listWidget_Devices->itemWidget(ptrItem);

        if (ptrWidget != nullptr)
        {
            ui->listWidget_Devices->removeItemWidget(ptrItem);
            delete ptrWidget;
        }

        delete ui->listWidget_Devices->takeItem(index);
    }

    m_deviceRows.clear();

    for (const IECDeviceManager::ConfigurationPtr& ptrConfiguration : sortedConfigurations)
    {
        IECDeviceRowWidget* ptrRow = createDeviceRow(ptrConfiguration->type());

        if (ptrRow == nullptr)
        {
            qWarning() << "IECDeviceManagerWidget: unsupported device type"
                       << static_cast<int>(ptrConfiguration->type());
            continue;
        }

        ptrRow->setConfiguration(*ptrConfiguration);
        IECFoundryDiskRowWidget* ptrDiskRow = dynamic_cast<IECFoundryDiskRowWidget*>(ptrRow);
        if (ptrDiskRow != nullptr)
        {
            ptrDiskRow->setImageLoaded(diskImageLoaded.value(ptrConfiguration->id(), false));
        }

        QListWidgetItem* ptrItem = new QListWidgetItem();
        ptrItem->setData(Qt::UserRole, QVariant::fromValue(ptrConfiguration->id()));

        ui->listWidget_Devices->addItem(ptrItem);
        ui->listWidget_Devices->setItemWidget(ptrItem, ptrRow);
        ptrItem->setSizeHint(ptrRow->sizeHint());

        m_deviceRows.insert(ptrConfiguration->id(), ptrRow);
    }
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
    for (int index = ui->listWidget_Devices->count() - 1; index >= 0; --index)
    {
        QListWidgetItem* ptrItem = ui->listWidget_Devices->item(index);
        QWidget* ptrWidget = ui->listWidget_Devices->itemWidget(ptrItem);

        ui->listWidget_Devices->removeItemWidget(ptrItem);

        delete ui->listWidget_Devices->takeItem(index);

        if (ptrWidget != nullptr)
            delete ptrWidget;
    }

    m_deviceRows.clear();
}
