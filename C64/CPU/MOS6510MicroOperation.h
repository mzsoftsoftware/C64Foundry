#pragma once

enum class MOS6510MicroOperation
{
    NoOperation,

    ReadImmediateToAccumulator,
    ReadImmediateToXRegister,
    ReadImmediateToYRegister,
    ReadImmediateAndAccumulator,
    ReadImmediateOrAccumulator,
    ReadImmediateExclusiveOrAccumulator,

    ReadZeroPageAddress,
    ReadZeroPageToAccumulator,
    ReadZeroPageToXRegister,
    ReadZeroPageToYRegister,
    ReadZeroPageAndAccumulator,
    ReadZeroPageOrAccumulator,
    ReadZeroPageExclusiveOrAccumulator,

    ReadZeroPageIndexedAddress,
    ReadZeroPageIndexedToAccumulator,
    ReadZeroPageIndexedToXRegister,
    ReadZeroPageIndexedToYRegister,
    ReadZeroPageIndexedAndAccumulator,
    ReadZeroPageIndexedOrAccumulator,
    ReadZeroPageIndexedExclusiveOrAccumulator,

    ReadAbsoluteAddressLow,
    ReadAbsoluteAddressHigh,
    ReadAbsoluteXAddress,
    ReadAbsoluteYAddress,
    ReadAbsoluteToAccumulator,
    ReadAbsoluteToXRegister,
    ReadAbsoluteToYRegister,
    ReadAbsoluteAndAccumulator,
    ReadAbsoluteOrAccumulator,
    ReadAbsoluteExclusiveOrAccumulator,

    ReadAbsoluteIndexedToAccumulator,
    ReadAbsoluteIndexedToXRegister,
    ReadAbsoluteIndexedToYRegister,
    ReadAbsoluteIndexedAndAccumulator,
    ReadAbsoluteIndexedOrAccumulator,
    ReadAbsoluteIndexedExclusiveOrAccumulator,

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
