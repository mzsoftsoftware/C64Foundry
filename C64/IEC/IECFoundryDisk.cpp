#include "IECFoundryDisk.h"

#include "IECFoundryDiskConfiguration.h"
#include "IECFoundryDrive.h"


QList<quint8> IECFoundryDisk::supportedDeviceNumbers()
{
    return { 8, 9, 10, 11 };
}

IECFoundryDisk::IECFoundryDisk(IECBus* ptrBus, IECFoundryDrive* ptrDrive, quint8 deviceNumber)
    : IECDevice(ptrBus, deviceNumber)
    , m_ptrDrive(ptrDrive)
    , m_protocol(iecBus(), deviceNumber, participantId())
{
    Q_ASSERT(m_ptrDrive);
    m_ptrDrive->registerDisk(this);
}
IECFoundryDisk::~IECFoundryDisk()
{
    m_ptrDrive->unregisterDisk(this);
}

bool IECFoundryDisk::setConfiguration(const IECFoundryDiskConfiguration& configuration, QString* errorMessage)
{
    Q_ASSERT(m_ptrDrive);

    if (configuration.type() != IECDeviceType::FoundryDisk)
        return false;

    if (configuration.deviceNumber() != deviceNumber())
        return false;

    const bool success = m_ptrDrive->insertImage(deviceNumber(), configuration.imageFileName());

    if (!success && errorMessage != nullptr)
        *errorMessage = m_ptrDrive->lastError();

    return success;
}

bool IECFoundryDisk::hasImage() const
{
    return m_ptrDrive != nullptr && m_ptrDrive->hasImage(deviceNumber());
}

void IECFoundryDisk::reset()
{
    m_protocol.reset();
    m_listenFileName.clear();
    m_requestedFileName.clear();
    m_pendingTalk = false;
}

void IECFoundryDisk::clock()
{
    const IECProtocol::Mode previousMode = m_protocol.mode();
    m_protocol.clock();
    const IECProtocol::Mode currentMode = m_protocol.mode();

    // Bei einem neuen LISTEN-Vorgang beginnt ein neuer Dateiname.
    if (currentMode == IECProtocol::Mode::Listen && previousMode != IECProtocol::Mode::Listen)
    {
        m_listenFileName.clear();
    }
    // TALK wird zunächst unter ATN angekündigt. Die Übertragung
    // beginnt erst, wenn ATN wieder High ist und die Adresse steht.
    if (currentMode == IECProtocol::Mode::Talk && previousMode != IECProtocol::Mode::Talk)
    {
        m_pendingTalk = true;
    }

    processReceivedBytes();
    if (m_protocol.takeListenEnded())
        finishListen();
    if (m_pendingTalk && lineHigh(IECBus::Line::ATN))
        startRequestedTransfer();
}

void IECFoundryDisk::processReceivedBytes()
{
    while (m_protocol.hasReceivedByte())
    {
        const IECProtocol::ReceivedByte receivedByte = m_protocol.takeReceivedByte();

        // Sekundäradresse 0 ist der Datenkanal für LOAD.
        // Der Kommando-/Statuskanal 15 wird später ergänzt.
        if (receivedByte.secondaryAddress == 0)
            m_listenFileName.append(static_cast<char>(receivedByte.value));
    }
}

void IECFoundryDisk::finishListen()
{
    m_requestedFileName = m_listenFileName;
    m_listenFileName.clear();
}

void IECFoundryDisk::startRequestedTransfer()
{
    m_pendingTalk = false;

    if (m_ptrDrive == nullptr || !m_ptrDrive->hasImage(deviceNumber()))
        return;

    QByteArray transferData;

    // LOAD "$",8 fordert das Directory als BASIC-Programm an.
    if (m_requestedFileName == QByteArrayLiteral("$"))
    {
        if (!m_ptrDrive->createDirectoryProgram(deviceNumber(), transferData))
            return;
    }
    else
    {
        if (!m_ptrDrive->readFile(deviceNumber(), m_requestedFileName, transferData))
            return;
    }

    if (transferData.isEmpty())
        return;

    if (!m_protocol.startSending(transferData))
    {
        // Falls das Protokoll in diesem Takt noch nicht senden kann,
        // wird der Versuch im nächsten Takt wiederholt.
        m_pendingTalk = true;
        return;
    }

    m_requestedFileName.clear();
}
