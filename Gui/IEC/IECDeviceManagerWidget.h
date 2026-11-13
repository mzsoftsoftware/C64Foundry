#pragma once

#include <QHash>
#include <QList>
#include <QWidget>

#include "C64/IEC/IECRequest.h"
#include "C64/IEC/IECSystemEvent.h"
#include "C64/IEC/IECDeviceConfiguration.h"

class QAction;
class QTreeWidgetItem;
class EmulatorController;
class IECDeviceRowWidget;
class IECResponse;
class IECSystemGetConfigurationsResponse;

namespace Ui {
class IECDeviceManagerWidget;
}


class IECDeviceManagerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit IECDeviceManagerWidget(QWidget* parent = nullptr);
    virtual ~IECDeviceManagerWidget();

    void setController(EmulatorController* ptrController);

private slots:
    void onAddMenuTriggered(QAction* ptrAction);
    void onIECRequestDone(QSharedPointer<IECResponse> response);
    void onIECSystemEventsChanged(IECSystemEventList events);
    void on_toolButton_Refresh_clicked();

private:
    void setupAddMenu();
    void updateAddMenu();
    void refreshConfigurations();
    void setConfigurations(const IECSystemGetConfigurationsResponse* ptrResponse);
    void clearDeviceRows();
    void clearDeviceList();
    IECDeviceRowWidget* createDeviceRow(IECDeviceType type);
    QString deviceTypeName(IECDeviceType type) const;
    quint8 deviceStartNumber(const IECDeviceConfiguration& configuration) const;
    void updateControls();

private:
    Ui::IECDeviceManagerWidget* ui = nullptr;
    EmulatorController* m_ptrController = nullptr;

    QHash<IECDeviceConfiguration::DeviceId, IECDeviceRowWidget*> m_deviceRows;
    QList<IECDeviceType> m_availableDeviceTypes;
    QList<IECDeviceType> m_addableDeviceTypes;

    IECRequest::RequestId m_addRequestId = 0;
    IECRequest::RequestId m_configurationRequestId = 0;

    bool m_bRefreshPending = false;
};
