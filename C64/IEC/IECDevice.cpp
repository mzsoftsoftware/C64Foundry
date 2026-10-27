#include "IECDevice.h"


IECDevice::IECDevice(IECBus* ptrBus, const quint8 deviceNumber)
    : m_deviceNumber(deviceNumber)
    , m_ptrBus(ptrBus)
{
    if (m_ptrBus != nullptr)
        m_participantId = m_ptrBus->addParticipant();
}

IECDevice::~IECDevice()
{
    if (m_ptrBus != nullptr)
        m_ptrBus->removeParticipant(m_participantId);
}

bool IECDevice::lineHigh(const IECBus::Line line) const
{
    if (m_ptrBus == nullptr)
        return true;

    return m_ptrBus->lineHigh(line);
}

void IECDevice::setLineLow(const IECBus::Line line, const bool low)
{
    if (m_ptrBus == nullptr)
        return;

    m_ptrBus->setLineLow(m_participantId, line, low);
}
