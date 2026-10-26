#pragma once

#include <QWidget>

#include "C64/IEC/IECDeviceConfiguration.h"

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
    void setOutput(const QByteArray& output);

private slots:
    void on_toolButton_Save_clicked();
    void on_toolButton_TearOff_clicked();

    void onPrinterOutputChanged(IECDeviceConfiguration::DeviceId deviceId, quint64 revision, QByteArray output);

private:
    Ui::IECAsciiPrinterOutputWidget *ui;

    EmulatorController* m_ptrController = nullptr;
    IECDeviceConfiguration::DeviceId m_deviceId = 0;
    quint64 m_outputRevision = 0;
};
