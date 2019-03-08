#pragma once
#include "stdafx.h"
#include <unordered_map>

class Console;
class MemoryManager;

struct CpuState;
enum class AddrMode : uint8_t;

class DisassemblyInfo
{
public:
	static string OpName[256];
	static AddrMode OpMode[256];
	static uint8_t OpSize[256];

private:
	uint8_t _byteCode[4];
	uint8_t _opSize;
	AddrMode _addrMode;
	uint8_t _flags;

public:
	DisassemblyInfo();
	DisassemblyInfo(uint8_t *opPointer, uint8_t cpuFlags);

	void Initialize(uint8_t * opPointer, uint8_t cpuFlags);

	void GetDisassembly(string &out, uint32_t memoryAddr);
	uint32_t GetOperandAddress(uint32_t memoryAddr);
	
	uint8_t GetOperandSize();
	static uint8_t GetOperandSize(AddrMode addrMode, uint8_t flags);
	static uint8_t GetOperandSize(uint8_t opCode, uint8_t flags);

	void GetByteCode(uint8_t copyBuffer[4]);
	void GetByteCode(string &out);
	
	void GetEffectiveAddressString(string &out, CpuState &state, Console* console);
	int32_t GetEffectiveAddress(CpuState &state, Console *console);
	uint16_t GetMemoryValue(uint32_t effectiveAddress, MemoryManager *memoryManager, uint8_t &valueSize);
};

enum class AddrMode : uint8_t
{
	Sig8,
	Imm8,
	Imm16,
	ImmX,
	ImmM,
	Abs,
	AbsIdxXInd, //JMP/JSR only
	AbsIdxX,
	AbsIdxY,
	AbsInd, //JMP only
	AbsIndLng, //JML only
	AbsLngIdxX,
	AbsLng,
	AbsJmp, //JSR/JMP only
	AbsLngJmp, //JSL/JMP only
	Acc,
	BlkMov,
	DirIdxIndX,
	DirIdxX,
	DirIdxY,
	DirIndIdxY,
	DirIndLngIdxY,
	DirIndLng,
	DirInd,
	Dir,
	Imp,
	RelLng,
	Rel,
	Stk,
	StkRel,
	StkRelIndIdxY
};