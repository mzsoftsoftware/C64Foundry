#include "C64KeyboardConfiguration.h"

#include "C64KeyboardDefaults.h"


QList<C64KeyboardMapping> C64KeyboardConfiguration::mappings(const HostPlatform::Type platform) const
{
    QList<C64KeyboardMapping> mappings = C64KeyboardDefaults::mappings(platform);
    for (const C64KeyboardMapping& overrideMapping : m_overrides)
    {
        int mappingIndex = -1;
        for (int index = 0; index < mappings.size(); ++index)
        {
            if (mappings[index].nativeScanCode == overrideMapping.nativeScanCode)
            {
                mappingIndex = index;
                break;
            }
        }

        if (overrideMapping.keys.isEmpty())
        {
            if (mappingIndex >= 0)
                mappings.removeAt(mappingIndex);

            continue;
        }

        if (mappingIndex >= 0)
            mappings[mappingIndex] = overrideMapping;
        else
            mappings.append(overrideMapping);
    }

    return mappings;
}

void C64KeyboardConfiguration::setOverride(const C64KeyboardMapping& mapping)
{
    for (int index = 0; index < m_overrides.size(); ++index)
    {
        if (m_overrides[index].nativeScanCode == mapping.nativeScanCode)
        {
            m_overrides[index] = mapping;
            return;
        }
    }

    m_overrides.append(mapping);
}

void C64KeyboardConfiguration::removeOverride(const quint32 nativeScanCode)
{
    for (int index = 0; index < m_overrides.size(); ++index)
    {
        if (m_overrides[index].nativeScanCode == nativeScanCode)
        {
            m_overrides.removeAt(index);
            return;
        }
    }
}

void C64KeyboardConfiguration::clearOverrides()
{
    m_overrides.clear();
}
