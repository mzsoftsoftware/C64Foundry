#pragma once

#include "IECDeviceRowWidget.h"

namespace Ui {
class IECFoundryDiskRowWidget;
}


class IECFoundryDiskRowWidget : public IECDeviceRowWidget
{
    Q_OBJECT

public:
    explicit IECFoundryDiskRowWidget(QWidget* parent = nullptr);
    ~IECFoundryDiskRowWidget() override;

protected:
    void onControllerSet() override;
    void setOperationState(IECOperationStatusWidget::State state, const QString& message = QString()) override;
    void applyConfiguration(const IECDeviceConfiguration& configuration) override;
    void setPowerButtonChecked(bool checked) override;
    void setControlsEnabled(bool enabled) override;
    void openSettingsDialog(const EmulatorResponseIEC* ptrResponse) override;

private slots:
    void on_toolButton_Power_toggled(bool checked);
    void on_toolButton_Disk_clicked();
    void on_toolButton_Eject_clicked();
    void on_toolButton_DiskName_clicked();
    void on_toolButton_Settings_clicked();
    void on_toolButton_Remove_clicked();

private:
    void showSettingsDialog(const EmulatorResponseIEC* ptrResponse);

    Ui::IECFoundryDiskRowWidget* ui = nullptr;
};
