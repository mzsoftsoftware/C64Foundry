#include "IECFoundryDisk.h"

#include "IECFoundryDiskConfiguration.h"
#include "IECFoundryDrive.h"

#include <QDebug>


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
    qDebug() << "IEC disk UNLISTEN"
             << "device:" << deviceNumber()
             << "filename bytes:" << m_requestedFileName.toHex()
             << "filename:" << m_requestedFileName;

    m_listenFileName.clear();
}

void IECFoundryDisk::startRequestedTransfer()
{
    m_pendingTalk = false;

    qDebug() << "IEC disk TALK"
             << "device:" << deviceNumber()
             << "filename:" << m_requestedFileName.toHex()
             << "has image:" << (m_ptrDrive != nullptr && m_ptrDrive->hasImage(deviceNumber()))
             << "mode:" << static_cast<int>(m_protocol.mode())
             << "ATN high:" << lineHigh(IECBus::Line::ATN);

    if (m_ptrDrive == nullptr || !m_ptrDrive->hasImage(deviceNumber()))
    {
        qDebug() << "IEC disk: no image available";
        return;
    }

    QByteArray transferData;

    if (m_requestedFileName == QByteArrayLiteral("$"))
    {
        if (!m_ptrDrive->createDirectoryProgram(deviceNumber(), transferData))
        {
            qDebug() << "IEC disk: directory creation failed:" << m_ptrDrive->lastError();
            return;
        }
    }
    else
    {
        if (!m_ptrDrive->readFile(deviceNumber(), m_requestedFileName, transferData))
        {
            qDebug() << "IEC disk: file read failed:" << m_ptrDrive->lastError();
            return;
        }
    }

    qDebug() << "IEC disk: transfer data size:" << transferData.size();

    if (transferData.isEmpty())
    {
        qDebug() << "IEC disk: transfer data is empty";
        return;
    }

    if (!m_protocol.startSending(transferData))
    {
        qDebug() << "IEC disk: startSending failed"
                 << "mode:" << static_cast<int>(m_protocol.mode());

        m_pendingTalk = true;
        return;
    }

    qDebug() << "IEC disk: startSending accepted";
    m_requestedFileName.clear();
}
