#pragma once

#include <QQueue>
#include <QTimer>
#include <QWidget>

#include "C64/IEC/IECDeviceEvent.h"
#include "C64/IEC/IECFoundryDiskConfiguration.h"
#include "C64/IEC/IECRequest.h"

class EmulatorController;
class IECResponse;

namespace Ui {
class IECFoundryDiskRowWidget;
}


class IECFoundryDiskRowWidget : public QWidget
{
    Q_OBJECT

public:
    explicit IECFoundryDiskRowWidget(QWidget* parent = nullptr);
    ~IECFoundryDiskRowWidget() override;

    void setController(EmulatorController* ptrController);

    void setSlotConfiguration(IECDeviceConfiguration::DeviceId driveId,
                              const IECFoundryDiskConfiguration& configuration);

    void processDeviceEvent(const IECDeviceEvent& event);

private slots:
    void onPowerClicked(bool enabled);
    void onDiskClicked();
    void onEjectClicked();
    void onIECRequestDone(QSharedPointer<IECResponse> response);
    void processNextSectorAccessEvent();

private:
    enum class RequestPurpose
    {
        None,
        SetEnabled,
        InsertImage,
        EjectImage
    };

    bool submitRequest(IECRequest* ptrRequest, RequestPurpose purpose);
    void finishRequest();
    void updateImageDisplay();
    void updateControls();

private:
    Ui::IECFoundryDiskRowWidget* ui = nullptr;

    EmulatorController* m_ptrController = nullptr;

    IECDeviceConfiguration::DeviceId m_driveId = 0;
    IECFoundryDiskConfiguration m_configuration;

    IECRequest::RequestId m_requestId = 0;
    RequestPurpose m_requestPurpose = RequestPurpose::None;
    bool m_pendingEnabled = false;

    bool m_bImageLoaded = false;
    bool m_bImageInvalid = false;
    QString m_errorMessage;

    QQueue<IECDriveAccessEvent> m_sectorAccessQueue;
    QTimer m_activityTimer;
};
