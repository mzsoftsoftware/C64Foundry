#pragma once

#include "IECDeviceConfiguration.h"

#include <QList>
#include <QSharedPointer>
#include <QStringList>


class IECConfigurationValidator
{
public:
    static bool validate(
        const QList<QSharedPointer<IECDeviceConfiguration>>& configurations,
        QStringList* errors = nullptr);
};
