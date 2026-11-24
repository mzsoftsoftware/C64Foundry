#include "IECFoundryDiskRowWidget.h"
#include "ui_IECFoundryDiskRowWidget.h"

#include <QFileInfo>
#include <QFont>
#include <QSignalBlocker>


IECFoundryDiskRowWidget::IECFoundryDiskRowWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::IECFoundryDiskRowWidget)
{
    ui->setupUi(this);

    ui->widget_ActivityLED->setLEDCount(2);

    // Die Aktionen werden im nächsten Schritt über Device-Requests angebunden.
    ui->toolButton_Power->setEnabled(false);
    ui->toolButton_Disk->setEnabled(false);
    ui->toolButton_Eject->setEnabled(false);

    ui->toolButton_Settings->hide();
    ui->toolButton_Remove->hide();

    m_activityTimer.setInterval(240);
    m_activityTimer.setTimerType(Qt::PreciseTimer);

    connect(&m_activityTimer, &QTimer::timeout,
            this, &IECFoundryDiskRowWidget::processNextSectorAccessEvent);
}


IECFoundryDiskRowWidget::~IECFoundryDiskRowWidget()
{
    delete ui;
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
        ui->toolButton_Eject->setEnabled(false);
        return;
    }

    const QString displayName = QFileInfo(fileName).fileName();

    ui->toolButton_DiskName->setText(displayName);

    if (m_bImageInvalid)
    {
        ui->toolButton_DiskName->setStyleSheet(
            QStringLiteral("QToolButton { color: red; }"));

        ui->toolButton_DiskName->setToolTip(
            QStringLiteral("%1\n%2")
                .arg(fileName, m_errorMessage));

        ui->toolButton_Eject->setEnabled(false);
        return;
    }

    if (!m_errorMessage.isEmpty())
    {
        ui->toolButton_DiskName->setStyleSheet(
            QStringLiteral("QToolButton { color: red; }"));

        ui->toolButton_DiskName->setToolTip(
            QStringLiteral("%1\n%2")
                .arg(fileName, m_errorMessage));
    }
    else
    {
        ui->toolButton_DiskName->setStyleSheet(QString());

        ui->toolButton_DiskName->setToolTip(
            QStringLiteral("%1\n%2")
                .arg(fileName,
                     m_bImageLoaded
                         ? tr("Image loaded")
                         : tr("Image status unknown")));
    }

    ui->toolButton_Eject->setEnabled(false);
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
