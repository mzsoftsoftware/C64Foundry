#include "EmulationScheduler.h"

#include "C64/C64Machine.h"
#include "C64/IEC/IECDevice.h"


EmulationScheduler::EmulationScheduler(C64Machine* ptrMachine)
    : m_ptrMachine(ptrMachine)
{
}

void EmulationScheduler::addIECDevice(IECDevice* ptrDevice)
{
    if (ptrDevice == nullptr || m_iecDevices.contains(ptrDevice))
        return;

    m_iecDevices.append(ptrDevice);
}

void EmulationScheduler::removeIECDevice(IECDevice* ptrDevice)
{
    m_iecDevices.removeAll(ptrDevice);
}

void EmulationScheduler::clock()
{
    m_ptrMachine->clock();

    for (IECDevice* ptrDevice : m_iecDevices)
        ptrDevice->clock();
}

void EmulationScheduler::runCycles(const quint64 cycles)
{
    for (quint64 cycle = 0; cycle < cycles; ++cycle)
        clock();
}
