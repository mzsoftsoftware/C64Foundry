#pragma once

#include <QWidget>

#include "C64/IEC/IECDeviceConfiguration.h"
#include "C64/IEC/IECDeviceEvent.h"

class EmulatorController;

namespace Ui {
class IECAsciiPrinterOutputWidget;
}


class IECAsciiPrinterOutputWidget : public QWidget
{
    Q_OBJECT

public:
    explicit IECAsciiPrinterOutputWidget(QWidget *parent = nullptr);
    virtual ~IECAsciiPrinterOutputWidget();

    void setController(EmulatorController* ptrController, IECDeviceConfiguration::DeviceId deviceId);
    void setPrinterName(const QString& name);

private slots:
    void on_toolButton_Save_clicked();
    void on_toolButton_TearOff_clicked();

    void onPrinterOutputChanged(IECDeviceConfiguration::DeviceId deviceId, QQueue<IECPrinterOutputEvent> events);

private:
    Ui::IECAsciiPrinterOutputWidget *ui;

    EmulatorController* m_ptrController = nullptr;
    IECDeviceConfiguration::DeviceId m_deviceId = 0;
};
