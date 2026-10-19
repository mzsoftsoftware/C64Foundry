#pragma once

#include <QtGlobal>
#include <QVector>

class C64Machine;
class IECDevice;


class EmulationScheduler
{
public:
    explicit EmulationScheduler(C64Machine* ptrMachine);

    void addIECDevice(IECDevice* ptrDevice);
    void removeIECDevice(IECDevice* ptrDevice);

    void clock();
    void runCycles(quint64 cycles);

private:
    C64Machine* m_ptrMachine = nullptr;
    QVector<IECDevice*> m_iecDevices;
};
