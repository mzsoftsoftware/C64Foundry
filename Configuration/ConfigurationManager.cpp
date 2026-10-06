#include "ConfigurationManager.h"

#include "C64Configuration.h"
#include "ConfigurationStorage.h"
#include "ROMSetDetector.h"


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
