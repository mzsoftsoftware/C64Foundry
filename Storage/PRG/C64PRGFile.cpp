#include "C64PRGFile.h"

#include <QFile>


bool C64PRGFile::load(const QString& fileName)
{
    //
    // Reset previous state.
    //
    m_loadAddress = 0;
    m_data.clear();
    m_bBasicProgram = false;
    m_basicEndAddress = 0;
    m_errorString.clear();

    //
    // Open PRG file.
    //
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        m_errorString = file.errorString();
        return false;
    }

    //
    // A PRG file contains a two-byte load address followed
    // by the program data.
    //
    const QByteArray fileData = file.readAll();
    if (file.error() != QFileDevice::NoError)
    {
        m_errorString = file.errorString();
        return false;
    }

    if (fileData.size() < 3)
    {
        m_errorString = QStringLiteral("Invalid PRG file: missing load address or program data.");
        return false;
    }

    //
    // Extract little-endian load address.
    //
    const quint16 address = static_cast<quint8>(fileData.at(0)) | (static_cast<quint16>(static_cast<quint8>(fileData.at(1))) << 8);

    //
    // Validate available C64 address space.
    //
    const qsizetype dataSize = fileData.size() - 2;
    if (dataSize > (0x10000 - address))
    {
        m_errorString = QStringLiteral("Invalid PRG file: program exceeds C64 address space.");
        return false;
    }

    //
    // Store validated program.
    //
    m_loadAddress = address;
    m_data = fileData.mid(2);

    analyzeBasicProgram();

    return true;
}


bool C64PRGFile::analyzeBasicProgram()
{
    m_bBasicProgram = false;
    m_basicEndAddress = 0;

    if (m_data.size() < 4)
        return false;

    const quint32 programStart = m_loadAddress;
    const quint32 programEnd = programStart + static_cast<quint32>(m_data.size());

    quint32 currentAddress = programStart;

    while (true)
    {
        const qsizetype offset = static_cast<qsizetype>(currentAddress - programStart);

        if (offset + 2 > m_data.size())
            return false;

        const quint16 nextAddress = static_cast<quint8>(m_data.at(offset)) | (static_cast<quint16>(static_cast<quint8>(m_data.at(offset + 1))) << 8);

        if (nextAddress == 0)
        {
            if (currentAddress == programStart)
                return false;

            // Address immediately after the BASIC end marker.
            const quint32 endAddress = currentAddress + 2;

            if (endAddress > 0xFFFF)
                return false;

            m_basicEndAddress = static_cast<quint16>(endAddress);
            m_bBasicProgram = true;
            return true;
        }

        if (nextAddress <= currentAddress + 4)
            return false;

        if (nextAddress >= programEnd)
            return false;

        const qsizetype nextOffset = static_cast<qsizetype>(nextAddress - programStart);

        // A BASIC line consists of a link, a line number,
        // tokenized content and a terminating zero byte.
        if (nextOffset < offset + 5)
            return false;

        if (static_cast<quint8>(m_data.at(nextOffset - 1)) != 0)
            return false;

        currentAddress = nextAddress;
    }
}
