#include "IECFoundryDiskRowWidget.h"
#include "ui_IECFoundryDiskRowWidget.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QSettings>

#include "Emulator/EmulatorController.h"
#include "C64/IEC/IECDeviceRequest.h"
#include "C64/IEC/IECDeviceResponse.h"


IECFoundryDiskRowWidget::IECFoundryDiskRowWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::IECFoundryDiskRowWidget)
{
    ui->setupUi(this);

    ui->widget_ActivityLED->setLEDCount(2);

    ui->toolButton_Settings->hide();
    ui->toolButton_Remove->hide();

    m_activityTimer.setInterval(240);
    m_activityTimer.setTimerType(Qt::PreciseTimer);

    connect(&m_activityTimer, &QTimer::timeout,
            this, &IECFoundryDiskRowWidget::processNextSectorAccessEvent);

    connect(ui->toolButton_Power, &QToolButton::clicked,
            this, &IECFoundryDiskRowWidget::onPowerClicked);

    connect(ui->toolButton_Disk, &QToolButton::clicked,
            this, &IECFoundryDiskRowWidget::onDiskClicked);

    connect(ui->toolButton_Eject, &QToolButton::clicked,
            this, &IECFoundryDiskRowWidget::onEjectClicked);

    updateControls();
}


IECFoundryDiskRowWidget::~IECFoundryDiskRowWidget()
{
    delete ui;
}


void IECFoundryDiskRowWidget::setController(EmulatorController* ptrController)
{
    Q_ASSERT(ptrController != nullptr);
    Q_ASSERT(m_ptrController == nullptr);

    m_ptrController = ptrController;

    connect(m_ptrController, &EmulatorController::iecRequestDone,
            this, &IECFoundryDiskRowWidget::onIECRequestDone);

    updateControls();
}


void IECFoundryDiskRowWidget::setSlotConfiguration(
    IECDeviceConfiguration::DeviceId driveId,
    const IECFoundryDiskConfiguration& configuration)
{
    m_driveId = driveId;
    m_configuration = configuration;

    const QSignalBlocker blocker(ui->toolButton_Power);
    ui->toolButton_Power->setChecked(configuration.enabled());

    QFont font = ui->label_DeviceNumber->font();
    font.setStyleHint(QFont::Monospace);
    font.setFamily(QStringLiteral("Monospace"));
    ui->label_DeviceNumber->setFont(font);

    ui->label_DeviceNumber->setText(
        QStringLiteral("#%1")
            .arg(configuration.deviceNumber(), 2, 10, QChar('0')));

    ui->label_DeviceType->setText(tr("Disk"));

    updateImageDisplay();
    updateControls();
}


bool IECFoundryDiskRowWidget::submitRequest(
    IECRequest* ptrRequest, RequestPurpose purpose)
{
    if (m_ptrController == nullptr || m_requestId != 0 ||
        ptrRequest == nullptr)
    {
        delete ptrRequest;
        return false;
    }

    m_requestPurpose = purpose;
    m_requestId = m_ptrController->request(ptrRequest);

    if (m_requestId == 0)
    {
        m_requestPurpose = RequestPurpose::None;

        QMessageBox::warning(
            this, tr("IEC device"),
            tr("Could not submit IEC device request"));

        updateControls();
        return false;
    }

    updateControls();
    return true;
}


void IECFoundryDiskRowWidget::onPowerClicked(bool enabled)
{
    if (m_ptrController == nullptr || m_requestId != 0)
        return;

    m_pendingEnabled = enabled;

    const QSignalBlocker blocker(ui->toolButton_Power);
    ui->toolButton_Power->setChecked(m_configuration.enabled());

    submitRequest(
        new IECFoundryDriveSetDiskEnabledRequest(
            m_driveId, m_configuration.deviceNumber(), enabled),
        RequestPurpose::SetEnabled);
}


void IECFoundryDiskRowWidget::onDiskClicked()
{
    if (m_ptrController == nullptr || m_requestId != 0)
        return;

    QSettings settings;

    const QString lastDirectory =
        settings.value(QStringLiteral("IEC/LastImageDirectory")).toString();

    const QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Insert Disk Image"),
        lastDirectory,
        tr("Disk Images (*.d64 *.d71 *.d81);;All Files (*)"));

    if (fileName.isEmpty())
        return;

    settings.setValue(
        QStringLiteral("IEC/LastImageDirectory"),
        QFileInfo(fileName).absolutePath());

    submitRequest(
        new IECFoundryDriveInsertImageRequest(
            m_driveId, m_configuration.deviceNumber(), fileName),
        RequestPurpose::InsertImage);
}

void IECFoundryDiskRowWidget::onEjectClicked()
{
    if (m_ptrController == nullptr || m_requestId != 0 ||
        m_configuration.imageFileName().isEmpty())
    {
        return;
    }

    submitRequest(
        new IECFoundryDriveEjectImageRequest(
            m_driveId, m_configuration.deviceNumber()),
        RequestPurpose::EjectImage);
}


void IECFoundryDiskRowWidget::onIECRequestDone(
    QSharedPointer<IECResponse> response)
{
    if (response.isNull() || m_requestId == 0 ||
        response->requestId() != m_requestId)
    {
        return;
    }

    const RequestPurpose purpose = m_requestPurpose;

    const IECFoundryDriveResponse* ptrDriveResponse =
        dynamic_cast<const IECFoundryDriveResponse*>(response.data());

    const QString errorMessage =
        ptrDriveResponse != nullptr
            ? ptrDriveResponse->errorMessage()
            : QString();

    finishRequest();

    if (!response->success())
    {
        const QString message = errorMessage.isEmpty()
        ? tr("IEC device operation failed")
        : errorMessage;

        QMessageBox::warning(this, tr("IEC device"), message);

        updateControls();
        return;
    }

    if (purpose == RequestPurpose::SetEnabled)
    {
        m_configuration.setEnabled(m_pendingEnabled);

        const QSignalBlocker blocker(ui->toolButton_Power);
        ui->toolButton_Power->setChecked(m_pendingEnabled);
    }

    updateControls();
}


void IECFoundryDiskRowWidget::finishRequest()
{
    m_requestId = 0;
    m_requestPurpose = RequestPurpose::None;
}


void IECFoundryDiskRowWidget::updateControls()
{
    const bool available = m_ptrController != nullptr &&
                           m_requestId == 0;

    ui->toolButton_Power->setEnabled(available);
    ui->toolButton_Disk->setEnabled(available);
    ui->toolButton_Eject->setEnabled(
        available && !m_configuration.imageFileName().isEmpty());
}


void IECFoundryDiskRowWidget::processDeviceEvent(const IECDeviceEvent& event)
{
    if (event.deviceId != m_driveId)
        return;

    const IECDriveImageEvent* ptrImageEvent =
        dynamic_cast<const IECDriveImageEvent*>(&event);

    if (ptrImageEvent != nullptr)
    {
        if (ptrImageEvent->deviceNumber != m_configuration.deviceNumber())
            return;

        switch (ptrImageEvent->type)
        {
        case IECDriveImageEvent::Type::ImageInserted:
            m_configuration.setImageFileName(ptrImageEvent->fileName);
            m_bImageLoaded = true;
            m_bImageInvalid = false;
            m_errorMessage.clear();
            break;

        case IECDriveImageEvent::Type::ImageEjected:
            m_configuration.setImageFileName(QString());
            m_bImageLoaded = false;
            m_bImageInvalid = false;
            m_errorMessage.clear();
            break;

        case IECDriveImageEvent::Type::ImageInvalid:
            m_configuration.setImageFileName(ptrImageEvent->fileName);
            m_bImageLoaded = false;
            m_bImageInvalid = true;
            m_errorMessage = ptrImageEvent->errorMessage;
            break;

        case IECDriveImageEvent::Type::Error:
            m_errorMessage = ptrImageEvent->errorMessage;
            break;
        }

        updateImageDisplay();
        updateControls();
        return;
    }

    const IECDriveAccessEvent* ptrAccessEvent =
        dynamic_cast<const IECDriveAccessEvent*>(&event);

    if (ptrAccessEvent == nullptr ||
        ptrAccessEvent->deviceNumber != m_configuration.deviceNumber())
    {
        return;
    }

    m_sectorAccessQueue.enqueue(*ptrAccessEvent);

    if (!m_activityTimer.isActive())
        m_activityTimer.start();
}


void IECFoundryDiskRowWidget::updateImageDisplay()
{
    const QString fileName = m_configuration.imageFileName();

    if (fileName.isEmpty())
    {
        ui->toolButton_DiskName->setText(tr("No disk"));
        ui->toolButton_DiskName->setToolTip(tr("No disk image inserted"));
        ui->toolButton_DiskName->setStyleSheet(QString());
        return;
    }

    const QString displayName = QFileInfo(fileName).fileName();

    ui->toolButton_DiskName->setText(displayName);

    if (m_bImageInvalid || !m_errorMessage.isEmpty())
    {
        ui->toolButton_DiskName->setStyleSheet(
            QStringLiteral("QToolButton { color: red; }"));

        ui->toolButton_DiskName->setToolTip(
            QStringLiteral("%1\n%2")
                .arg(fileName, m_errorMessage));
        return;
    }

    ui->toolButton_DiskName->setStyleSheet(QString());

    ui->toolButton_DiskName->setToolTip(
        QStringLiteral("%1\n%2")
            .arg(fileName,
                 m_bImageLoaded
                     ? tr("Image loaded")
                     : tr("Image status unknown")));
}


void IECFoundryDiskRowWidget::processNextSectorAccessEvent()
{
    if (m_sectorAccessQueue.isEmpty())
    {
        m_activityTimer.stop();
        return;
    }

    const IECDriveAccessEvent event = m_sectorAccessQueue.dequeue();

    ui->lcdNumber_TrackSector->display(
        QStringLiteral("%1.%2")
            .arg(event.track, 2, 10, QChar('0'))
            .arg(event.sector, 2, 10, QChar('0')));

    switch (event.operation)
    {
    case IECDriveAccessEvent::Operation::Read:
        ui->widget_ActivityLED->triggerActivity(0);
        break;

    case IECDriveAccessEvent::Operation::Write:
        ui->widget_ActivityLED->triggerActivity(1);
        break;
    }

    if (m_sectorAccessQueue.isEmpty())
        m_activityTimer.stop();
}
