#include "ConfigurationManager.h"

#include "C64Configuration.h"
#include "ConfigurationStorage.h"
#include "ROMSetDetector.h"

#include "Emulator/EmulatorController.h"
#include "C64/IEC/IECConfigurationValidator.h"
#include "C64/IEC/IECSystemRequest.h"
#include "C64/IEC/IECSystemResponse.h"

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
    {
        qWarning() << "ConfigurationManager: no active configuration available";

        if (!m_bSaveError)
        {
            m_bSaveError = true;
            emit saveErrorChanged(true);
        }

        return false;
    }

    ConfigurationStorage storage;

    const bool bSuccess = storage.save(m_qmapConfigurations, m_ptrActiveConfiguration->name);

    if (!bSuccess)
        qWarning() << "ConfigurationManager: could not save configurations";

    if (m_bSaveError != !bSuccess)
    {
        m_bSaveError = !bSuccess;
        emit saveErrorChanged(m_bSaveError);
    }

    return bSuccess;
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

    //
    // Validate the complete copied configuration before replacing
    // the currently active configuration.
    //
    QStringList errors;

    if (!IECConfigurationValidator::validate(copiedConfigurations, &errors))
    {
        for (const QString& error : errors)
            qWarning() << "ConfigurationManager: invalid IEC configuration:" << error;

        return false;
    }

    //
    // Replace the active IEC configuration only after successful
    // cloning and validation.
    //
    m_ptrActiveConfiguration->iecDevices.swap(copiedConfigurations);

    return true;
}

void ConfigurationManager::startIECSynchronization(
    EmulatorController* ptrController)
{
    if (ptrController == nullptr || m_ptrEmulatorController != nullptr)
        return;

    m_ptrEmulatorController = ptrController;

    connect(m_ptrEmulatorController,
            &EmulatorController::iecSystemEventsChanged,
            this,
            &ConfigurationManager::onIECSystemEventsChanged);

    connect(m_ptrEmulatorController,
            &EmulatorController::iecRequestDone,
            this,
            &ConfigurationManager::onIECRequestDone);

    qDebug() << "ConfigurationManager: IEC synchronization started";

    requestIECConfigurations();
}
void ConfigurationManager::onIECSystemEventsChanged(
    IECSystemEventList events)
{
    for (const IECSystemEventPtr& ptrEvent : events)
    {
        if (ptrEvent.isNull())
            continue;

        switch (ptrEvent->type)
        {
        case IECSystemEvent::Type::DeviceAdded:
        case IECSystemEvent::Type::DeviceRemoved:
        case IECSystemEvent::Type::ConfigurationChanged:
        case IECSystemEvent::Type::ConfigurationsRestored:
            requestIECConfigurations();
            return;

        case IECSystemEvent::Type::Reset:
            break;
        }
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

    m_iecConfigurationRequestId =
        m_ptrEmulatorController->request(
            new IECSystemGetConfigurationsRequest());

    if (m_iecConfigurationRequestId == 0)
    {
        qWarning() << "ConfigurationManager: could not request IEC configurations";
        m_bIECRefreshPending = true;
    }
}
void ConfigurationManager::onIECRequestDone(
    QSharedPointer<IECResponse> response)
{
    if (response.isNull())
        return;

    if (m_iecConfigurationRequestId == 0 ||
        response->requestId() != m_iecConfigurationRequestId)
    {
        return;
    }

    m_iecConfigurationRequestId = 0;

    const IECSystemGetConfigurationsResponse* ptrIECResponse =
        dynamic_cast<const IECSystemGetConfigurationsResponse*>(response.data());

    if (!response->success() || ptrIECResponse == nullptr)
    {
        qWarning() << "ConfigurationManager: could not retrieve IEC configurations";
        m_bIECRefreshPending = true;
    }
    else if (!setIECConfigurations(ptrIECResponse->configurations()))
    {
        qWarning() << "ConfigurationManager: could not update IEC configurations";
        m_bIECRefreshPending = true;
    }
    else
    {
        if (!save())
            qWarning() << "ConfigurationManager: IEC configurations could not be persisted";
    }

    if (m_bIECRefreshPending)
    {
        m_bIECRefreshPending = false;
        requestIECConfigurations();
    }
}
