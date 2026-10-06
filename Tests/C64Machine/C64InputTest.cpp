#include "C64InputTest.h"

#include <QTest>
#include <QTemporaryFile>
#include <QByteArray>

#include "C64/C64ROMSet.h"
#include "C64/C64Machine.h"
#include "C64/Input/C64Keyboard.h"


void C64InputTest::testCIA1Keyboard()
{
    C64Machine machine;

    //
    // Configure CIA1 Port A as output and
    // Port B as input.
    //
    machine.writeCIA1Register(0x02, 0xFF);
    machine.writeCIA1Register(0x03, 0x00);

    //
    // With no key pressed all input lines
    // must remain high.
    //
    QCOMPARE(
        machine.readCIA1Register(0x01),
        quint8(0xFF));

    //
    // The A key is located at matrix position
    // row 1, column 2.
    //
    machine.keyPress(C64Key::KeyA);

    //
    // Select the matrix line for A by pulling PA1 low.
    //
    machine.writeCIA1Register(0x00, 0xFD);

    //
    // The pressed A key must pull PB2 low.
    //
    QCOMPARE(
        machine.readCIA1Register(0x01),
        quint8(0xFB));

    //
    // Releasing the key must release the
    // corresponding input line again.
    //
    machine.keyRelease(C64Key::KeyA);

    QCOMPARE(
        machine.readCIA1Register(0x01),
        quint8(0xFF));
}
void C64InputTest::testCIA1KeyboardReverse()
{
    C64Machine machine;

    //
    // Configure CIA1 Port A as input and
    // Port B as output.
    //
    machine.writeCIA1Register(0x02, 0x00);
    machine.writeCIA1Register(0x03, 0xFF);

    //
    // With no key pressed all input lines
    // must remain high.
    //
    QCOMPARE(
        machine.readCIA1Register(0x00),
        quint8(0xFF));

    //
    // The A key is located at matrix position
    // row 1, column 2.
    //
    machine.keyPress(C64Key::KeyA);

    //
    // Select the matrix line for A by pulling PB2 low.
    //
    machine.writeCIA1Register(0x01, 0xFB);

    //
    // The pressed A key must pull PA1 low.
    //
    QCOMPARE(
        machine.readCIA1Register(0x00),
        quint8(0xFD));

    //
    // Releasing the key must release the
    // corresponding input line again.
    //
    machine.keyRelease(C64Key::KeyA);

    QCOMPARE(
        machine.readCIA1Register(0x00),
        quint8(0xFF));
}

void C64InputTest::testCIA1KeyboardShiftLock()
{
    C64Machine machine;

    //
    // Configure CIA1 Port A as output and
    // Port B as input.
    //
    machine.writeCIA1Register(0x02, 0xFF);
    machine.writeCIA1Register(0x03, 0x00);

    //
    // Left Shift is located at matrix position
    // row 1, column 7.
    //
    machine.writeCIA1Register(0x00, 0xFD);

    //
    // Initially the Shift matrix contact must
    // be open.
    //
    QCOMPARE(
        machine.readCIA1Register(0x01),
        quint8(0xFF));

    //
    // Shift Lock mechanically holds the
    // left Shift key down.
    //
    machine.keyPress(C64Key::ShiftLock);

    QCOMPARE(
        machine.readCIA1Register(0x01),
        quint8(0x7F));

    //
    // Pressing Left Shift while Shift Lock is
    // active keeps the same matrix contact closed.
    //
    machine.keyPress(C64Key::LeftShift);

    QCOMPARE(
        machine.readCIA1Register(0x01),
        quint8(0x7F));

    //
    // Releasing Left Shift must not release the
    // matrix contact while Shift Lock is active.
    //
    machine.keyRelease(C64Key::LeftShift);

    QCOMPARE(
        machine.readCIA1Register(0x01),
        quint8(0x7F));

    //
    // Releasing Shift Lock now releases the
    // matrix contact.
    //
    machine.keyRelease(C64Key::ShiftLock);

    QCOMPARE(
        machine.readCIA1Register(0x01),
        quint8(0xFF));

    //
    // Test the opposite order:
    // Left Shift is pressed first.
    //
    machine.keyPress(C64Key::LeftShift);

    QCOMPARE(
        machine.readCIA1Register(0x01),
        quint8(0x7F));

    //
    // Shift Lock is then pressed while
    // Left Shift is already held.
    //
    machine.keyPress(C64Key::ShiftLock);

    QCOMPARE(
        machine.readCIA1Register(0x01),
        quint8(0x7F));

    //
    // Releasing Shift Lock must not release the
    // matrix contact while Left Shift is held.
    //
    machine.keyRelease(C64Key::ShiftLock);

    QCOMPARE(
        machine.readCIA1Register(0x01),
        quint8(0x7F));

    //
    // Only releasing Left Shift opens the
    // matrix contact again.
    //
    machine.keyRelease(C64Key::LeftShift);

    QCOMPARE(
        machine.readCIA1Register(0x01),
        quint8(0xFF));
}

void C64InputTest::testRestore()
{
    C64Machine machine;

    //
    // The CPU NMI line must initially be inactive.
    //
    QVERIFY(!machine.cpuNmiLine());

    //
    // RESTORE is not part of the keyboard matrix.
    // Pressing it must activate the CPU NMI line.
    //
    machine.keyPress(C64Key::Restore);

    QVERIFY(machine.cpuNmiLine());

    //
    // Releasing RESTORE must release the CPU NMI line.
    //
    machine.keyRelease(C64Key::Restore);

    QVERIFY(!machine.cpuNmiLine());
}
void C64InputTest::testRestoreAndCIA2NMI()
{
    C64Machine machine;

    //
    // The CPU NMI line must initially be inactive.
    //
    QVERIFY(!machine.cpuNmiLine());

    //
    // Pressing RESTORE activates the NMI line.
    //
    machine.keyPress(C64Key::Restore);

    QVERIFY(machine.cpuNmiLine());

    //
    // Enable Timer A interrupts in CIA2.
    //
    machine.writeCIA2Register(0x0D, 0x81);

    //
    // Load Timer A with a short interval.
    //
    machine.writeCIA2Register(0x04, 0x01);
    machine.writeCIA2Register(0x05, 0x00);

    //
    // Force load and start Timer A.
    //
    machine.writeCIA2Register(0x0E, 0x11);

    //
    // Let Timer A underflow.
    //
    machine.runCycles(2);

    //
    // Releasing RESTORE must not release the CPU NMI line,
    // because CIA2 is still requesting an interrupt.
    //
    machine.keyRelease(C64Key::Restore);

    QVERIFY(machine.cpuNmiLine());

    //
    // Reading the CIA2 interrupt control register
    // clears the pending interrupt status.
    //
    const quint8 interruptStatus =
        machine.readCIA2Register(0x0D);

    QVERIFY(interruptStatus & 0x01);
    QVERIFY(interruptStatus & 0x80);

    //
    // Advance one machine cycle so the cleared CIA2
    // interrupt state is propagated to the CPU NMI line.
    //
    machine.clock();

    //
    // RESTORE is released and CIA2 no longer requests
    // an interrupt. The CPU NMI line must therefore
    // be inactive again.
    //
    QVERIFY(!machine.cpuNmiLine());
}

void C64InputTest::testCIA1KeyboardBusScan()
{
    QTemporaryFile basicFile;
    QTemporaryFile kernalFile;
    QTemporaryFile characterFile;

    QVERIFY(basicFile.open());
    QVERIFY(kernalFile.open());
    QVERIFY(characterFile.open());

    QByteArray kernalROM(8192, char(0x00));

    //
    // Reset vector $FFFC/$FFFD -> $0800.
    //
    kernalROM[0x1FFC] = char(0x00);
    kernalROM[0x1FFD] = char(0x08);

    QCOMPARE(
        basicFile.write(QByteArray(8192, char(0x00))),
        qsizetype(8192));

    QCOMPARE(
        kernalFile.write(kernalROM),
        qsizetype(8192));

    QCOMPARE(
        characterFile.write(QByteArray(4096, char(0x00))),
        qsizetype(4096));

    basicFile.close();
    kernalFile.close();
    characterFile.close();

    C64ROMSet romSet(
        basicFile.fileName(),
        kernalFile.fileName(),
        characterFile.fileName());

    C64Machine machine;

    QVERIFY(machine.loadROMSet(romSet));

    //
    // The A key is located at matrix position
    // row 1, column 2.
    //
    machine.keyPress(C64Key::KeyA);

    QByteArray program;

    //
    // Configure CIA1 Port A as output.
    //
    program.append(char(0xA9));     // LDA #$FF
    program.append(char(0xFF));
    program.append(char(0x8D));     // STA $DC02
    program.append(char(0x02));
    program.append(char(0xDC));

    //
    // Configure CIA1 Port B as input.
    //
    program.append(char(0xA9));     // LDA #$00
    program.append(char(0x00));
    program.append(char(0x8D));     // STA $DC03
    program.append(char(0x03));
    program.append(char(0xDC));

    //
    // Select the matrix line for A by pulling PA1 low.
    //
    program.append(char(0xA9));     // LDA #$FD
    program.append(char(0xFD));
    program.append(char(0x8D));     // STA $DC00
    program.append(char(0x00));
    program.append(char(0xDC));

    //
    // Read CIA1 Port B.
    //
    program.append(char(0xAD));     // LDA $DC01
    program.append(char(0x01));
    program.append(char(0xDC));

    //
    // Store the result in RAM.
    //
    program.append(char(0x85));     // STA $02
    program.append(char(0x02));

    //
    // Stop execution.
    //
    program.append(char(0x02));     // KIL

    QVERIFY(machine.loadProgram(0x0800, program));

    machine.powerOn();
    machine.runCycles(100);

    //
    // The pressed A key must pull PB2 low.
    //
    QCOMPARE(
        machine.readRAM(0x0002),
        quint8(0xFB));
}

void C64InputTest::testCIA1KeyboardMultipleKeys()
{
    C64Machine machine;

    machine.writeCIA1Register(0x02, 0xFF);
    machine.writeCIA1Register(0x03, 0x00);

    //
    // A and S are both located on PA1.
    // A pulls PB2 low, S pulls PB5 low.
    //
    machine.keyPress(C64Key::KeyA);
    machine.keyPress(C64Key::KeyS);

    machine.writeCIA1Register(0x00, 0xFD);

    QCOMPARE(
        machine.readCIA1Register(0x01),
        quint8(0xDB));

    //
    // Releasing A must leave S active.
    //
    machine.keyRelease(C64Key::KeyA);

    QCOMPARE(
        machine.readCIA1Register(0x01),
        quint8(0xDF));

    //
    // Releasing S must clear the matrix again.
    //
    machine.keyRelease(C64Key::KeyS);

    QCOMPARE(
        machine.readCIA1Register(0x01),
        quint8(0xFF));
}

void C64InputTest::testCIA1KeyboardHeldKey()
{
    C64Machine machine;

    //
    // Configure CIA1 Port A as output and
    // Port B as input.
    //
    machine.writeCIA1Register(0x02, 0xFF);
    machine.writeCIA1Register(0x03, 0x00);

    //
    // A is located at matrix position
    // row 1, column 2.
    //
    machine.writeCIA1Register(0x00, 0xFD);

    //
    // Press A.
    //
    machine.keyPress(C64Key::KeyA);

    QCOMPARE(
        machine.readCIA1Register(0x01),
        quint8(0xFB));

    //
    // Keep the key pressed while the machine
    // continues running for many cycles.
    //
    machine.runCycles(100000);

    //
    // The matrix contact must still be closed.
    //
    QCOMPARE(
        machine.readCIA1Register(0x01),
        quint8(0xFB));

    //
    // Releasing A must open the matrix contact.
    //
    machine.keyRelease(C64Key::KeyA);

    QCOMPARE(
        machine.readCIA1Register(0x01),
        quint8(0xFF));
}
