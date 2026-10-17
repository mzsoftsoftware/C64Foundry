
#pragma once

#include <QByteArray>
#include <QString>
#include <QtGlobal>


class C64PRGFile
{
public:
    C64PRGFile() = default;

    // Operations
    bool load(const QString& fileName);

    // Getter
    quint16 loadAddress() const             { return m_loadAddress; }
    const QByteArray& data() const          { return m_data; }
    bool isBasicProgram() const             { return m_bBasicProgram; }
    quint16 basicEndAddress() const         { return m_basicEndAddress; }

    const QString& errorString() const      { return m_errorString; }

private:
    bool analyzeBasicProgram();

private:
    quint16 m_loadAddress = 0;
    QByteArray m_data;
    bool m_bBasicProgram = false;
    quint16 m_basicEndAddress = 0;

    QString m_errorString;
};
