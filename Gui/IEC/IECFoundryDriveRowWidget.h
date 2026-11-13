#pragma once

#include "IECDeviceRowWidget.h"

namespace Ui {
class IECFoundryDriveRowWidget;
}

class IECSystemGetConfigurationsResponse;


class IECFoundryDriveRowWidget : public IECDeviceRowWidget
{
    Q_OBJECT

public:
    explicit IECFoundryDriveRowWidget(QWidget* parent = nullptr);
    virtual ~IECFoundryDriveRowWidget();

protected:
    void setOperationState(IECOperationStatusWidget::State state, const QString& message = QString()) override;
    void applyConfiguration(const IECDeviceConfiguration& configuration) override;
    void setPowerButtonChecked(bool checked) override;
    void setControlsEnabled(bool enabled) override;
    void openSettingsDialog(const IECSystemGetConfigurationsResponse* ptrResponse) override;

private slots:
    void on_toolButton_Power_toggled(bool checked);
    void on_toolButton_Settings_clicked();
    void on_toolButton_Remove_clicked();

private:
    Ui::IECFoundryDriveRowWidget* ui = nullptr;
};
