#pragma once

#include <QQueue>
#include <QTimer>
#include <QWidget>

#include "C64/IEC/IECDeviceEvent.h"
#include "C64/IEC/IECFoundryDiskConfiguration.h"

namespace Ui {
class IECFoundryDiskRowWidget;
}


class IECFoundryDiskRowWidget : public QWidget
{
    Q_OBJECT

public:
    explicit IECFoundryDiskRowWidget(QWidget* parent = nullptr);
    ~IECFoundryDiskRowWidget() override;

    void setSlotConfiguration(IECDeviceConfiguration::DeviceId driveId,
                              const IECFoundryDiskConfiguration& configuration);

    void processDeviceEvent(const IECDeviceEvent& event);

private slots:
    void processNextSectorAccessEvent();

private:
    void updateImageDisplay();

private:
    Ui::IECFoundryDiskRowWidget* ui = nullptr;

    IECDeviceConfiguration::DeviceId m_driveId = 0;
    IECFoundryDiskConfiguration m_configuration;

    bool m_bImageLoaded = false;
    bool m_bImageInvalid = false;
    QString m_errorMessage;

    QQueue<IECDriveAccessEvent> m_sectorAccessQueue;
    QTimer m_activityTimer;
};
