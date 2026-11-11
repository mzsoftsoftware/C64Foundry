#pragma once

#include <QtGlobal>
#include <QString>


class IECFoundryDiskConfiguration
{
public:
    using DiskId = quint64;

    IECFoundryDiskConfiguration() = default;

    DiskId diskId() const;
    int slotIndex() const;
    bool enabled() const;
    QString imageFileName() const;
    bool saveCurrentDiskOnExit() const;

    void setDiskId(DiskId diskId);
    void setSlotIndex(int slotIndex);
    void setEnabled(bool enabled);
    void setImageFileName(const QString& fileName);
    void setSaveCurrentDiskOnExit(bool enabled);

    bool operator==(const IECFoundryDiskConfiguration& other) const;
    bool operator!=(const IECFoundryDiskConfiguration& other) const;

private:
    DiskId m_diskId = 0;
    int m_slotIndex = 0;
    bool m_enabled = false;
    QString m_imageFileName;
    bool m_saveCurrentDiskOnExit = false;
};
