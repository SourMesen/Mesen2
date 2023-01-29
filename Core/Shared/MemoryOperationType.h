#pragma once

enum class MemoryOperationType
{
	Read = 0,
	Write = 1,
	ExecOpCode = 2,
	ExecOperand = 3,
	DmaRead = 4,
	DmaWrite = 5,
	DummyRead = 6,
	DummyWrite = 7,
	PpuRenderingRead = 8,
	Idle = 9
};

enum class MemoryAccessFlags
{
	None = 0,
	DspAccess = 1,
};