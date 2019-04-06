#pragma once
#include "stdafx.h"

class DisassemblyInfo;
class Console;
struct CpuState;
enum class AddrMode : uint8_t;

class CpuDisUtils
{
private:
	static string OpName[256];
	static AddrMode OpMode[256];
	static uint32_t GetOperandAddress(DisassemblyInfo &info, uint32_t memoryAddr);
	static uint8_t GetOpSize(AddrMode addrMode, uint8_t flags);

public:
	static void GetDisassembly(DisassemblyInfo &info, string &out, uint32_t memoryAddr);
	static uint8_t GetOpSize(uint8_t opCode, uint8_t flags);
	static int32_t GetEffectiveAddress(DisassemblyInfo &info, Console* console, CpuState &state);
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