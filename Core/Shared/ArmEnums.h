#pragma once
#include "pch.h"

enum class ArmOpCategory
{
	BranchExchangeRegister,
	Branch,
	Msr,
	Mrs,
	DataProcessing,
	Multiply,
	MultiplyLong,
	SingleDataTransfer,
	SignedHalfDataTransfer,
	BlockDataTransfer,
	SingleDataSwap,
	SoftwareInterrupt,
	InvalidOp,
};

enum class ArmAluOperation : uint8_t
{
	And,
	Eor,
	Sub,
	Rsb,

	Add,
	Adc,
	Sbc,
	Rsc,

	Tst,
	Teq,
	Cmp,
	Cmn,

	Orr,
	Mov,
	Bic,
	Mvn
};
