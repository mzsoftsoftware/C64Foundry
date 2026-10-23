#include "ConfigurationManager.h"

#include "C64Configuration.h"
#include "ConfigurationStorage.h"
#include "ROMSetDetector.h"

#include "Emulator/EmulatorController.h"

#include <QDebug>


ConfigurationManager::ConfigurationManager(QObject* parent)
    : QObject(parent)
{
}
ConfigurationManager::~ConfigurationManager()
{
    qDeleteAll(m_qmapConfigurations);
    m_qmapConfigurations.clear();
    m_ptrActiveConfiguration = nullptr;
}

bool ConfigurationManager::initialize()
{
    ConfigurationStorage storage;
    QMap<QString, C64Configuration*> configurations;
    QString activeConfigurationName;

    //
    // Load all stored configurations.
    //
    if (!storage.load(configurations, activeConfigurationName))
    {
        qDeleteAll(configurations);
        return false;
    }

    //
    // Create and store a default configuration if no configurations
    // exist yet.
    //
    if (configurations.isEmpty())
    {
        C64Configuration* ptrConfiguration = new C64Configuration();
        ptrConfiguration->name = QStringLiteral("Default");

        ptrConfiguration->romSet.basicROMFileName = QStringLiteral(":/ROMs/OpenROMs/basic.rom");
        ptrConfiguration->romSet.kernalROMFileName = QStringLiteral(":/ROMs/OpenROMs/kernal.rom");
        ptrConfiguration->romSet.characterROMFileName = QStringLiteral(":/ROMs/OpenROMs/chargen.rom");

        configurations.insert(ptrConfiguration->name, ptrConfiguration);
        activeConfigurationName = ptrConfiguration->name;

        //
        // Detect an installed VICE C64 ROM set.
        //
        ROMSetDetector romSetDetector;
        const C64ROMSet viceROMSet = romSetDetector.detectVICE();
        if (!viceROMSet.basicROMFileName.isEmpty())
        {
            C64Configuration* ptrVICEConfiguration = new C64Configuration();
            ptrVICEConfiguration->name = QStringLiteral("VICE C64");
            ptrVICEConfiguration->romSet = viceROMSet;

            configurations.insert(
                ptrVICEConfiguration->name,
                ptrVICEConfiguration);
        }

        if (!storage.save(configurations, activeConfigurationName))
        {
            qDeleteAll(configurations);
            return false;
        }
    }

    //
    // The stored active configuration must exist.
    //
    C64Configuration* ptrActiveConfiguration = configurations.value(activeConfigurationName, nullptr);
    if (ptrActiveConfiguration == nullptr)
    {
        qDeleteAll(configurations);
        return false;
    }

    //
    // Replace the current configuration state only after initialization
    // completed successfully.
    //
    qDeleteAll(m_qmapConfigurations);
    m_qmapConfigurations = configurations;
    m_ptrActiveConfiguration = ptrActiveConfiguration;

    return true;
}

bool ConfigurationManager::save()
{
    if (m_ptrActiveConfiguration == nullptr)
        return false;

    ConfigurationStorage storage;

    if (!storage.save(m_qmapConfigurations, m_ptrActiveConfiguration->name))
    {
        qWarning() << "ConfigurationManager: could not save configurations";
        return false;
    }

    return true;
}

bool ConfigurationManager::setIECConfigurations(const QList<QSharedPointer<IECDeviceConfiguration>>& configurations)
{
    if (m_ptrActiveConfiguration == nullptr)
        return false;

    QList<QSharedPointer<IECDeviceConfiguration>> copiedConfigurations;

    for (const QSharedPointer<IECDeviceConfiguration>& ptrConfiguration : configurations)
    {
        if (ptrConfiguration.isNull())
            return false;

        QSharedPointer<IECDeviceConfiguration> ptrCopy(ptrConfiguration->clone());

        if (ptrCopy.isNull())
            return false;

        copiedConfigurations.append(ptrCopy);
    }

    m_ptrActiveConfiguration->iecDevices.swap(copiedConfigurations);

    return true;
}

void ConfigurationManager::startIECSynchronization(EmulatorController* ptrController)
{
    if (ptrController == nullptr || m_ptrEmulatorController != nullptr)
        return;

    m_ptrEmulatorController = ptrController;

    connect(m_ptrEmulatorController, &EmulatorController::iecDeviceChanged, this, &ConfigurationManager::onIECDeviceChanged);
    connect(m_ptrEmulatorController, &EmulatorController::requestDone, this, &ConfigurationManager::onIECRequestDone);

    qDebug() << "ConfigurationManager: IEC synchronization started";
}
void ConfigurationManager::onIECDeviceChanged(EmulatorIECDeviceEvent::Type type, IECDeviceConfiguration::DeviceId deviceId)
{
    Q_UNUSED(deviceId)

    switch (type)
    {
    case EmulatorIECDeviceEvent::Type::Added:
    case EmulatorIECDeviceEvent::Type::Removed:
    case EmulatorIECDeviceEvent::Type::ConfigurationChanged:
        requestIECConfigurations();
        break;

    case EmulatorIECDeviceEvent::Type::Reset:
    case EmulatorIECDeviceEvent::Type::Restored:
        break;
    }
}
void ConfigurationManager::requestIECConfigurations()
{
    if (m_ptrEmulatorController == nullptr)
        return;

    if (m_iecConfigurationRequestId != 0)
    {
        m_bIECRefreshPending = true;
        return;
    }

    m_iecConfigurationRequestId = m_ptrEmulatorController->request(new EmulatorRequestGetIECConfigurations());

    if (m_iecConfigurationRequestId == 0)
    {
        qWarning() << "ConfigurationManager: could not request IEC configurations";
        m_bIECRefreshPending = true;
    }
}
void ConfigurationManager::onIECRequestDone(QSharedPointer<EmulatorResponse> response)
{
    if (response.isNull())
        return;

    if (m_iecConfigurationRequestId == 0 || response->id() != m_iecConfigurationRequestId)
        return;

    m_iecConfigurationRequestId = 0;

    if (response->type() != EmulatorRequest::Type::GetIECConfigurations || !response->success())
    {
        qWarning() << "ConfigurationManager: could not retrieve IEC configurations";
        m_bIECRefreshPending = true;
        return;
    }

    const EmulatorResponseIEC* ptrIECResponse = dynamic_cast<const EmulatorResponseIEC*>(response.data());

    if (ptrIECResponse == nullptr)
    {
        qWarning() << "ConfigurationManager: invalid IEC configuration response";
        m_bIECRefreshPending = true;
        return;
    }

    if (!setIECConfigurations(ptrIECResponse->configurations()))
    {
        qWarning() << "ConfigurationManager: could not update IEC configurations";
        m_bIECRefreshPending = true;
        return;
    }

    if (!save())
        qWarning() << "ConfigurationManager: IEC configurations could not be persisted";

    if (m_bIECRefreshPending)
    {
        m_bIECRefreshPending = false;
        requestIECConfigurations();
    }
}
