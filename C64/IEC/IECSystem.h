#pragma once

#include <QHash>
#include <QList>
#include <QQueue>
#include <QSharedPointer>
#include <QString>
#include <QtGlobal>

#include "IECDeviceConfiguration.h"
#include "IECDeviceEvent.h"


class C64Machine;
class IECBus;
class IECProtocol;
class IECDevice;
class IECDeviceManager;
class IECDeviceFactory;


class IECSystem
{
public:
    using DeviceId = IECDeviceConfiguration::DeviceId;
    using ConfigurationPtr = QSharedPointer<IECDeviceConfiguration>;

    struct PrinterOutputChange
    {
        DeviceId deviceId = 0;
        QQueue<IECPrinterOutputEvent> events;
    };

    struct DriveAccessChange
    {
        DeviceId deviceId = 0;
        quint8 deviceNumber = 0;
        QQueue<IECDriveAccessEvent> events;
    };

    explicit IECSystem(C64Machine* ptrMachine, IECDeviceFactory* ptrDeviceFactory);
    ~IECSystem();

    IECSystem(const IECSystem&) = delete;
    IECSystem& operator=(const IECSystem&) = delete;

    // Lifecycle and clocking
    void reset();
    void clock();

    // Device types and supported addresses
    QList<IECDeviceType> availableDeviceTypes() const;
    QList<quint8> supportedDeviceNumbers(IECDeviceType type) const;

    // Configurations
    QList<ConfigurationPtr> configurations() const;
    ConfigurationPtr configuration(DeviceId id) const;
    bool restoreConfigurations(const QList<ConfigurationPtr>& configurations);

    // Device management
    QList<IECDeviceType> addableDeviceTypes() const;
    bool canAddDevice(IECDeviceType type) const;
    DeviceId addDevice(IECDeviceType type);
    bool removeDevice(DeviceId id);
    bool setConfiguration(const IECDeviceConfiguration& configuration, QString* operationMessage = nullptr);

    // Device-specific operations
    bool isDiskImageLoaded(DeviceId id) const;
    bool tearOffPrinter(DeviceId id);

    // Events
    QList<PrinterOutputChange> takePrinterOutputChanges();
    QList<DriveAccessChange> takeDriveAccessChanges();

private:
    struct DeviceEntry
    {
        ConfigurationPtr configuration;
        IECDevice* ptrDevice = nullptr;
    };

    DeviceEntry* findEntry(DeviceId id);
    const DeviceEntry* findEntry(DeviceId id) const;

    bool isDeviceNumberReserved(quint8 deviceNumber, DeviceId excludedId = 0) const;

    void registerDevice(DeviceEntry& entry);
    void unregisterDevice(DeviceEntry& entry);
    void destroyDevice(DeviceEntry& entry);

    void collectPrinterOutputChanges();
    void collectDriveAccessChanges();

private:
    // Non-owning references
    C64Machine* m_ptrMachine = nullptr;
    IECDeviceFactory* m_ptrDeviceFactory = nullptr;

    // Owned central IEC infrastructure
    IECBus* m_ptrBus = nullptr;
    IECProtocol* m_ptrProtocol = nullptr;
    IECDeviceManager* m_ptrDeviceManager = nullptr;

    // Configuration and device registry.
    // The actual device instances are owned by IECDeviceFactory.
    QList<DeviceEntry> m_devices;
    DeviceId m_nextDeviceId = 1;

    // Last forwarded printer output revision per device.
    QHash<DeviceId, quint64> m_printerOutputRevisions;

    // Pending events collected during clocking or device operations.
    QList<PrinterOutputChange> m_printerOutputChanges;
    QList<DriveAccessChange> m_driveAccessChanges;
};
