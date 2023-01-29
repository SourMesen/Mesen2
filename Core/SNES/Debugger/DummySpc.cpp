#include "pch.h"
#include "SNES/Debugger/DummySpc.h"

#define DUMMYSPC
#define Spc DummySpc
#include "SNES/Spc.cpp"
#include "SNES/Spc.Instructions.cpp"
#undef Spc
#undef DUMMYSPC

DummySpc::DummySpc(uint8_t* spcRam)
{
	_ram = spcRam;

	_opCode = 0;
	_opStep = SpcOpStep::ReadOpCode;
	_opSubStep = 0;
	_tmp1 = 0;
	_tmp2 = 0;
	_tmp3 = 0;
	_operandA = 0;
	_operandB = 0;
}

DummySpc::~DummySpc()
{
	_ram = nullptr;
}

void DummySpc::SetDummyState(SpcState& state)
{
	_state = state;
	_state.StopState = SnesCpuStopState::Running;
	_memOpCounter = 0;
}

void DummySpc::Step()
{
	do {
		ProcessCycle();
	} while(_opStep != SpcOpStep::ReadOpCode);
}

uint32_t DummySpc::GetOperationCount()
{
	return _memOpCounter;
}

void DummySpc::LogMemoryOperation(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	_memOperations[_memOpCounter] = {
		addr,
		(int32_t)value,
		type,
		MemoryType::SpcMemory
	};
	_memOpCounter++;
}

MemoryOperationInfo DummySpc::GetOperationInfo(uint32_t index)
{
	return _memOperations[index];
}