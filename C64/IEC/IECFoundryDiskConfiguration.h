#pragma once

#include <QString>


class IECFoundryDiskConfiguration
{
public:
    IECFoundryDiskConfiguration() = default;

    int slotIndex() const;
    bool enabled() const;
    QString imageFileName() const;
    bool saveCurrentDiskOnExit() const;

    void setSlotIndex(int slotIndex);
    void setEnabled(bool enabled);
    void setImageFileName(const QString& fileName);
    void setSaveCurrentDiskOnExit(bool enabled);

    bool operator==(const IECFoundryDiskConfiguration& other) const;
    bool operator!=(const IECFoundryDiskConfiguration& other) const;

private:
    int m_slotIndex = 0;
    bool m_enabled = false;
    QString m_imageFileName;
    bool m_saveCurrentDiskOnExit = false;
};
