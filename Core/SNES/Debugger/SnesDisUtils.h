#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"

class SnesConsole;
class LabelManager;
class EmuSettings;
struct SnesCpuState;
enum class CpuType : uint8_t;
enum class SnesAddrMode : uint8_t;

class SnesDisUtils
{
private:
	static uint8_t OpSize[0x1F];
	static uint32_t GetOperandAddress(DisassemblyInfo &info, uint32_t memoryAddr);
	static uint8_t GetOpSize(SnesAddrMode addrMode, uint8_t flags);

	static bool HasEffectiveAddress(SnesAddrMode addrMode);

public:
	static string OpName[256];
	static SnesAddrMode OpMode[256];

	static void GetDisassembly(DisassemblyInfo &info, string &out, uint32_t memoryAddr, LabelManager* labelManager, EmuSettings* settings);
	static uint8_t GetOpSize(uint8_t opCode, uint8_t flags);
	static bool IsUnconditionalJump(uint8_t opCode);
	static bool IsConditionalJump(uint8_t opCode);
	static CdlFlags::CdlFlags GetOpFlags(uint8_t opCode, uint32_t pc, uint32_t prevPc);
	static bool IsJumpToSub(uint8_t opCode);
	static bool IsReturnInstruction(uint8_t opCode);
	static EffectiveAddressInfo GetEffectiveAddress(DisassemblyInfo& info, SnesConsole* console, SnesCpuState& state, CpuType type);

	static bool CanDisassembleNextOp(uint8_t opCode);
	static void UpdateCpuFlags(uint8_t opCode, uint8_t* byteCode, uint8_t& cpuFlags);
};

enum class SnesAddrMode : uint8_t
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