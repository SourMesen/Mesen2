#pragma once
#include "pch.h"
#include "Shared/BaseState.h"

struct Cx4Dma
{
	uint32_t Source;
	uint32_t Dest;
	uint16_t Length;
	uint32_t Pos;
	bool Enabled;
};

struct Cx4Suspend
{
	uint32_t Duration;
	bool Enabled;
};

struct Cx4Cache
{
	bool Enabled;
	uint8_t Page;
	bool Lock[2];
	uint32_t Address[2];
	uint32_t Base;
	uint16_t ProgramBank;
	uint8_t ProgramCounter;
	uint16_t Pos;
};

struct Cx4Bus
{
	bool Enabled;
	bool Reading;
	bool Writing;
	uint8_t DelayCycles;
	uint32_t Address;
};

struct Cx4State : BaseState
{
	uint64_t CycleCount;

	//Program bank
	uint16_t PB;

	//Program counter
	uint8_t PC;

	//Accumulator
	uint32_t A; 

	//Page register
	uint16_t P;

	uint8_t SP;
	uint32_t Stack[8];

	//Multiplier
	uint64_t Mult;

	uint32_t RomBuffer;
	uint8_t RamBuffer[3];

	uint32_t MemoryDataReg;
	uint32_t MemoryAddressReg;
	uint32_t DataPointerReg;
	uint32_t Regs[16];

	bool Negative;
	bool Zero;
	bool Carry;
	bool Overflow;
	bool IrqFlag;

	bool Stopped;
	bool Locked;
	bool IrqDisabled;
	
	bool SingleRom;

	uint8_t RomAccessDelay;
	uint8_t RamAccessDelay;

	Cx4Bus Bus;
	Cx4Dma Dma;
	Cx4Cache Cache;
	Cx4Suspend Suspend;
	uint8_t Vectors[0x20];
};