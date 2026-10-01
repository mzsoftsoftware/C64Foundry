#pragma once

#include <QtGlobal>


enum class MOS6510MicroOperation : quint16
{
    //
    // Read cycles.
    //

    // Generic.
    NoOperation = 0x0000,

    // Immediate.
    ReadImmediate,
    ReadImmediateANC,
    ReadImmediateARR,
    ReadImmediateASR,
    ReadImmediateAXS,
    ReadImmediateXAA,
    ReadImmediateLAXUnstable,
    ReadImmediateToAccumulator,
    ReadImmediateToXRegister,
    ReadImmediateToYRegister,
    ReadImmediateAndAccumulator,
    ReadImmediateOrAccumulator,
    ReadImmediateExclusiveOrAccumulator,
    ReadImmediateAddToAccumulator,
    ReadImmediateSubtractFromAccumulator,
    ReadImmediateCompareAccumulator,
    ReadImmediateCompareXRegister,
    ReadImmediateCompareYRegister,

    // Zero page.
    ReadZeroPage,
    ReadZeroPageAddress,
    ReadZeroPageToAccumulator,
    ReadZeroPageToXRegister,
    ReadZeroPageToYRegister,
    ReadZeroPageAndAccumulator,
    ReadZeroPageOrAccumulator,
    ReadZeroPageExclusiveOrAccumulator,
    ReadZeroPageAddToAccumulator,
    ReadZeroPageSubtractFromAccumulator,
    ReadZeroPageCompareAccumulator,
    ReadZeroPageCompareXRegister,
    ReadZeroPageCompareYRegister,
    ReadZeroPageBitTest,

    // Zero page indexed.
    ReadZeroPageIndexed,
    ReadZeroPageIndexedAddress,
    ReadZeroPageIndexedToAccumulator,
    ReadZeroPageIndexedToXRegister,
    ReadZeroPageIndexedToYRegister,
    ReadZeroPageIndexedAndAccumulator,
    ReadZeroPageIndexedOrAccumulator,
    ReadZeroPageIndexedExclusiveOrAccumulator,
    ReadZeroPageIndexedAddToAccumulator,
    ReadZeroPageIndexedSubtractFromAccumulator,
    ReadZeroPageIndexedCompareAccumulator,

    // Absolute.
    ReadAbsolute,
    ReadAbsoluteAddressLow,
    ReadAbsoluteAddressHigh,
    ReadAbsoluteXAddress,
    ReadAbsoluteYAddress,
    ReadAbsoluteBitTest,

    // Absolute indexed.
    ReadAbsoluteIndexedDummy,
    ReadAbsoluteToAccumulator,
    ReadAbsoluteToXRegister,
    ReadAbsoluteToYRegister,
    ReadAbsoluteAndAccumulator,
    ReadAbsoluteOrAccumulator,
    ReadAbsoluteExclusiveOrAccumulator,
    ReadAbsoluteAddToAccumulator,
    ReadAbsoluteSubtractFromAccumulator,
    ReadAbsoluteCompareAccumulator,
    ReadAbsoluteCompareXRegister,
    ReadAbsoluteCompareYRegister,
    ReadAbsoluteIndexed,
    ReadAbsoluteIndexedToAccumulator,
    ReadAbsoluteIndexedToXRegister,
    ReadAbsoluteIndexedToYRegister,
    ReadAbsoluteIndexedAndAccumulator,
    ReadAbsoluteIndexedOrAccumulator,
    ReadAbsoluteIndexedExclusiveOrAccumulator,
    ReadAbsoluteIndexedAddToAccumulator,
    ReadAbsoluteIndexedSubtractFromAccumulator,
    ReadAbsoluteIndexedCompareAccumulator,

    // Indirect.
    ReadIndirectAddressLow,
    ReadIndirectAddressHigh,
    ReadIndirectAddressHighIndexed,
    ReadIndirectToAccumulator,
    ReadIndirectAddToAccumulator,
    ReadIndirectSubtractFromAccumulator,
    ReadIndirectCompareAccumulator,
    ReadIndirectAndAccumulator,
    ReadIndirectOrAccumulator,
    ReadIndirectExclusiveOrAccumulator,
    ReadIndirectIndexedToAccumulator,
    ReadIndirectIndexedAddToAccumulator,
    ReadIndirectIndexedSubtractFromAccumulator,
    ReadIndirectIndexedCompareAccumulator,
    ReadIndirectIndexedAndAccumulator,
    ReadIndirectIndexedOrAccumulator,
    ReadIndirectIndexedExclusiveOrAccumulator,

    // Memory.
    ReadMemoryToData,
    ReadMemoryToAccumulatorAndXRegister,
    ReadMemoryToAccumulatorXRegisterAndStackPointer,

    // Register operations.
    IncrementXRegister,
    IncrementYRegister,
    DecrementXRegister,
    DecrementYRegister,

    // Accumulator operations.
    ShiftLeftAccumulator,
    ShiftRightAccumulator,
    RotateLeftAccumulator,
    RotateRightAccumulator,

    // Transfers.
    TransferAccumulatorToXRegister,
    TransferAccumulatorToYRegister,
    TransferXRegisterToAccumulator,
    TransferYRegisterToAccumulator,
    TransferStackPointerToXRegister,
    TransferXRegisterToStackPointer,

    // Stack.
    ReadStackToAccumulator,
    ReadStackToStatus,
    ReadStackDummy,

    // Flags.
    SetDecimalFlag,
    ClearDecimalFlag,
    SetCarryFlag,
    ClearCarryFlag,
    SetInterruptDisableFlag,
    ClearInterruptDisableFlag,
    ClearOverflowFlag,

    // Branches.
    ReadRelativeBranchCarryClear,
    ReadRelativeBranchCarrySet,
    ReadRelativeBranchEqual,
    ReadRelativeBranchNotEqual,
    ReadRelativeBranchMinus,
    ReadRelativeBranchPlus,
    ReadRelativeBranchOverflowClear,
    ReadRelativeBranchOverflowSet,
    Branch,
    BranchPageCrossing,

    // JMP.
    ReadAbsoluteAddressHighAndJump,
    ReadIndirectJumpAddressLow,
    ReadIndirectJumpAddressHigh,

    // JSR.
    ReadJsrAddressLow,
    ReadJsrStackDummy,
    ReadJsrAddressHighAndJump,

    // RTS.
    ReadRtsProgramCounterDummy,
    ReadRtsStackDummy,
    ReadRtsReturnAddressLow,
    ReadRtsReturnAddressHigh,
    RtsIncrementProgramCounter,

    // RTI.
    ReadRtiProgramCounterDummy,
    ReadRtiStackDummy,
    ReadRtiStatus,
    ReadRtiProgramCounterLow,
    ReadRtiProgramCounterHigh,

    // BRK.
    ReadBrkPadding,
    ReadBrkVectorLow,
    ReadBrkVectorHigh,

    //
    // Write cycles.
    //

    // Stores.
    WriteAccumulator,
    WriteXRegister,
    WriteYRegister,
    WriteSHYAbsoluteX,
    WriteSHXAbsoluteY,
    WriteAHX,
    WriteTASAbsoluteY,

    // Memory / read-modify-write.
    WriteDataToMemory,
    WriteAccumulatorAndXRegisterToMemory,
    IncrementDataAndWriteToMemory,
    DecrementDataAndWriteToMemory,
    DecrementDataCompareAccumulatorAndWriteToMemory,
    IncrementDataSubtractFromAccumulatorAndWriteToMemory,
    ShiftLeftDataAndWriteToMemory,
    ShiftRightDataAndWriteToMemory,
    RotateLeftDataAndWriteToMemory,
    RotateRightDataAndWriteToMemory,
    RotateDataLeftAndAccumulatorAndWriteToMemory,
    RotateDataRightAndAddToAccumulatorAndWriteToMemory,
    ShiftDataLeftAndOrAccumulatorAndWriteToMemory,
    ShiftDataRightAndExclusiveOrAccumulatorAndWriteToMemory,

    // Stack.
    WriteAccumulatorToStack,
    WriteStatusToStack,

    // JSR.
    WriteJsrReturnAddressHigh,
    WriteJsrReturnAddressLow,

    // BRK.
    WriteBrkProgramCounterHigh,
    WriteBrkProgramCounterLow,
    WriteBrkStatus
};


//
// Returns true if the micro-operation performs a CPU write cycle.
//
constexpr bool mos6510MicroOperationIsWrite(const MOS6510MicroOperation operation)
{
    return operation >= MOS6510MicroOperation::WriteAccumulator;
}
