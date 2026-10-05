#include "MOS6526TestPort.h"

#include <QtTest>

#include "C64/CIA/MOS6526.h"


MOS6526TestPort::MOS6526TestPort()
{
}

MOS6526TestPort::~MOS6526TestPort()
{
}


void MOS6526TestPort::testPortARegisterWrite()
{
    MOS6526 cia;

    //
    // Configure all Port A pins as outputs.
    //
    cia.writeRegister(0x02, 0xFF);
    cia.writeRegister(0x00, 0x5A);
    QCOMPARE(cia.readRegister(0x00), quint8(0x5A));
}
void MOS6526TestPort::testDataDirectionRegisterAWrite()
{
    MOS6526 cia;
    cia.writeRegister(0x02, 0xA5);
    QCOMPARE(cia.readRegister(0x02), quint8(0xA5));
}
void MOS6526TestPort::testPortAInputs()
{
    MOS6526 cia;

    //
    // All Port A pins are inputs.
    //
    cia.writeRegister(0x02, 0x00);

    cia.setPortAInputs(0xA5);

    QCOMPARE(cia.readRegister(0x00), quint8(0xA5));
}
void MOS6526TestPort::testPortAMixedInputsOutputs()
{
    MOS6526 cia;

    //
    // Upper nibble: outputs
    // Lower nibble: inputs
    //
    cia.writeRegister(0x02, 0xF0);

    //
    // Output latch.
    //
    cia.writeRegister(0x00, 0xA5);

    //
    // External input pins.
    //
    cia.setPortAInputs(0x3C);

    //
    // Upper nibble comes from PRA:     $A0
    // Lower nibble comes from inputs:  $0C
    //
    QCOMPARE(cia.readRegister(0x00), quint8(0xAC));
}
void MOS6526TestPort::testPortAPins()
{
    MOS6526 cia;

    //
    // Upper nibble: outputs
    // Lower nibble: inputs
    //
    cia.writeRegister(0x02, 0xF0);

    //
    // Output latch.
    //
    cia.writeRegister(0x00, 0xA5);

    //
    // External input pins.
    //
    cia.setPortAInputs(0x3C);

    //
    // Upper nibble comes from PRA:     $A0
    // Lower nibble comes from inputs:  $0C
    //
    QCOMPARE(cia.portAPins(), quint8(0xAC));
}

void MOS6526TestPort::testPortBRegisterWrite()
{
    MOS6526 cia;

    //
    // Configure all Port B pins as outputs.
    //
    cia.writeRegister(0x03, 0xFF);

    cia.writeRegister(0x01, 0x5A);

    QCOMPARE(cia.readRegister(0x01), quint8(0x5A));
}
void MOS6526TestPort::testDataDirectionRegisterBWrite()
{
    MOS6526 cia;

    cia.writeRegister(0x03, 0xA5);

    QCOMPARE(cia.readRegister(0x03), quint8(0xA5));
}
void MOS6526TestPort::testPortBInputs()
{
    MOS6526 cia;

    //
    // All Port B pins are inputs.
    //
    cia.writeRegister(0x03, 0x00);

    cia.setPortBInputs(0xA5);

    QCOMPARE(cia.readRegister(0x01), quint8(0xA5));
}
void MOS6526TestPort::testPortBMixedInputsOutputs()
{
    MOS6526 cia;

    //
    // Upper nibble: outputs
    // Lower nibble: inputs
    //
    cia.writeRegister(0x03, 0xF0);

    //
    // Output latch.
    //
    cia.writeRegister(0x01, 0xA5);

    //
    // External input pins.
    //
    cia.setPortBInputs(0x3C);

    //
    // Upper nibble comes from PRB:     $A0
    // Lower nibble comes from inputs:  $0C
    //
    QCOMPARE(cia.readRegister(0x01), quint8(0xAC));
}
void MOS6526TestPort::testPortBPins()
{
    MOS6526 cia;

    //
    // Upper nibble: outputs
    // Lower nibble: inputs
    //
    cia.writeRegister(0x03, 0xF0);

    //
    // Output latch.
    //
    cia.writeRegister(0x01, 0xA5);

    //
    // External input pins.
    //
    cia.setPortBInputs(0x3C);

    //
    // Upper nibble comes from PRB:     $A0
    // Lower nibble comes from inputs:  $0C
    //
    QCOMPARE(cia.portBPins(), quint8(0xAC));
}

void MOS6526TestPort::testRegisterMirroring()
{
    MOS6526 cia;

    //
    // The MOS6526 has 16 registers.
    // Registers are mirrored every $10 bytes.
    //
    cia.writeRegister(0x12, 0xA5);
    QCOMPARE(cia.readRegister(0x02), quint8(0xA5));
    QCOMPARE(cia.readRegister(0x12), quint8(0xA5));
    QCOMPARE(cia.readRegister(0x22), quint8(0xA5));
    QCOMPARE(cia.readRegister(0xF2), quint8(0xA5));
}
