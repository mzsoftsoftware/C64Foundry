#pragma once

enum class MOS6510MicroOperation
{
    ReadImmediateToAccumulator,
    ReadImmediateToXRegister,
    ReadImmediateToYRegister,

    ReadZeroPageAddress,
    ReadZeroPageToAccumulator,
    ReadZeroPageToXRegister,
    ReadZeroPageToYRegister
};

/*
enum class MOS6510MicroOperation
{
    FetchOpcode,

    ReadImmediateToAccumulator,
    ReadImmediateToX,
    ReadImmediateToY,

    ReadZeroPageAddress,
    DummyReadZeroPageIndexed,

    ReadMemoryToAccumulator,
    ReadMemoryToX,
    ReadMemoryToY,

    ReadMemoryAndAddToAccumulator,
    ReadMemoryAndCompareWithAccumulator,

    ReadAddressLow,
    ReadAddressHigh,
    ReadAddressHighAndAddX,
    ReadAddressHighAndAddY,

    ReadZeroPagePointer,
    ReadPointerLow,
    ReadPointerHigh,
    ReadPointerHighAndAddY,

    DummyRead,

    WriteAccumulator
};*/
