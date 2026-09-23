#pragma once

enum class MOS6510MicroOperation
{
    NoOperation,

    ReadImmediateToAccumulator,
    ReadImmediateToXRegister,
    ReadImmediateToYRegister,

    ReadZeroPageAddress,
    ReadZeroPageToAccumulator,
    ReadZeroPageToXRegister,
    ReadZeroPageToYRegister,

    ReadZeroPageIndexedAddress,
    ReadZeroPageIndexedToAccumulator,
    ReadZeroPageIndexedToXRegister,
    ReadZeroPageIndexedToYRegister,

    ReadAbsoluteAddressLow,
    ReadAbsoluteAddressHigh,
    ReadAbsoluteAddressHighIndexed,

    ReadAbsoluteToAccumulator,
    ReadAbsoluteToXRegister,
    ReadAbsoluteToYRegister,

    ReadAbsoluteIndexedToAccumulator,
    ReadAbsoluteIndexedToXRegister,
    ReadAbsoluteIndexedToYRegister,

    ReadIndirectAddressLow,
    ReadIndirectAddressHigh,
    ReadIndirectAddressHighIndexed,
    ReadIndirectToAccumulator,
    ReadIndirectIndexedToAccumulator,

    WriteAccumulator,
    WriteXRegister,
    WriteYRegister,

    TransferAccumulatorToXRegister,
    TransferAccumulatorToYRegister,
    TransferXRegisterToAccumulator,
    TransferYRegisterToAccumulator,
    TransferStackPointerToXRegister,
    TransferXRegisterToStackPointer,

    WriteAccumulatorToStack,
    ReadStackToAccumulator,
    WriteStatusToStack,
    ReadStackToStatus
};
