#pragma once

enum class MOS6510MicroOperation
{
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
    ReadIndirectIndexedToAccumulator


};
