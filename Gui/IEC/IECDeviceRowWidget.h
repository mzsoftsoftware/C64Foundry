class IECDeviceRowWidget : public QWidget
{
    Q_OBJECT

public:
    explicit IECDeviceRowWidget(QWidget* parent = nullptr);
    virtual ~IECDeviceRowWidget();

    virtual void setConfiguration(const IECDeviceConfiguration& configuration) = 0;
    virtual void setOperationState(IECOperationStatusWidget::State state, const QString& message = QString()) = 0;
    virtual void setBusy(bool busy) = 0;

    virtual IECDeviceConfiguration::DeviceId deviceId() const = 0;

signals:
    void enabledChanged(IECDeviceConfiguration::DeviceId deviceId, bool enabled);
    void settingsRequested(IECDeviceConfiguration::DeviceId deviceId);
    void removeRequested(IECDeviceConfiguration::DeviceId deviceId);
};
