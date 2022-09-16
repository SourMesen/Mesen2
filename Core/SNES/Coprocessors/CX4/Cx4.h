#pragma once
#include "pch.h"
#include "SNES/Coprocessors/BaseCoprocessor.h"
#include "SNES/Coprocessors/CX4/Cx4Types.h"
#include "SNES/MemoryMappings.h"

class SnesConsole;
class Emulator;
class SnesMemoryManager;
class SnesCpu;

class Cx4 : public BaseCoprocessor
{
private:
	static constexpr int DataRamSize = 0xC00;

	Emulator *_emu;
	SnesConsole *_console;
	SnesMemoryManager *_memoryManager;
	SnesCpu *_cpu;
	MemoryMappings _mappings;
	double _clockRatio;

	Cx4State _state;
	uint16_t _prgRam[2][256];
	uint8_t _dataRam[Cx4::DataRamSize];

	void Exec(uint16_t opCode);
	void SwitchCachePage();
	bool ProcessCache(uint64_t targetCycle);
	void ProcessDma(uint64_t targetCycle);

	void Step(uint64_t cycles);
	bool IsRunning();
	bool IsBusy();

	uint8_t GetAccessDelay(uint32_t addr);
	uint8_t ReadCx4(uint32_t addr);
	void WriteCx4(uint32_t addr, uint8_t value);

	uint32_t GetSourceValue(uint8_t src);
	void WriteRegister(uint8_t reg, uint32_t value);
	void SetA(uint32_t value);

	void NOP();

	void WAIT();

	void Branch(bool branch, uint8_t, uint8_t dest);
	void Skip(uint8_t flagToCheck, uint8_t skipIfSet);
	void JSR(bool branch, uint8_t, uint8_t dest);
	void RTS();

	void PushPC();
	void PullPC();

	void SetZeroNegativeFlags();

	uint32_t AddValues(uint32_t a, uint32_t b);
	uint32_t Subtract(uint32_t a, uint32_t b);

	void CMPR(uint8_t shift, uint8_t src);
	void CMPR_Imm(uint8_t shift, uint8_t imm);
	void CMP(uint8_t shift, uint8_t src);
	void CMP_Imm(uint8_t shift, uint8_t imm);

	void SignExtend(uint8_t mode);

	void Load(uint8_t dest, uint8_t src);
	void Load_Imm(uint8_t dest, uint8_t imm);

	void ADD(uint8_t shift, uint8_t src);
	void ADD_Imm(uint8_t shift, uint8_t imm);
	void SUB(uint8_t shift, uint8_t src);
	void SUB_Imm(uint8_t shift, uint8_t imm);
	void SUBR(uint8_t shift, uint8_t src);
	void SUBR_Imm(uint8_t shift, uint8_t imm);

	void SMUL(uint8_t src);
	void SMUL_Imm(uint8_t imm);

	void AND(uint8_t shift, uint8_t src);
	void AND_Imm(uint8_t shift, uint8_t imm);
	void OR(uint8_t shift, uint8_t src);
	void OR_Imm(uint8_t shift, uint8_t imm);
	void XOR(uint8_t shift, uint8_t src);
	void XOR_Imm(uint8_t shift, uint8_t imm);
	void XNOR(uint8_t shift, uint8_t src);
	void XNOR_Imm(uint8_t shift, uint8_t imm);

	void SHR(uint8_t src);
	void SHR_Imm(uint8_t imm);
	void ASR(uint8_t src);
	void ASR_Imm(uint8_t imm);
	void SHL(uint8_t src);
	void SHL_Imm(uint8_t imm);
	void ROR(uint8_t src);
	void ROR_Imm(uint8_t imm);

	void ReadRom();
	void ReadRom_Imm(uint16_t imm);
	void ReadRam(uint8_t byteIndex);
	void ReadRam_Imm(uint8_t byteIndex, uint8_t imm);
	void WriteRam(uint8_t byteIndex);
	void WriteRam_Imm(uint8_t byteIndex, uint8_t imm);

	void LoadP(uint8_t byteIndex, uint8_t imm);

	void Swap(uint8_t reg);
	void IncMar();
	void Store(uint8_t src, uint8_t dst);
	void Stop();

public:
	Cx4(SnesConsole* console);

	void Reset() override;

	void Run() override;

	uint8_t Read(uint32_t addr) override;
	void Write(uint32_t addr, uint8_t value) override;

	void Serialize(Serializer &s) override;

	uint8_t Peek(uint32_t addr) override;
	void PeekBlock(uint32_t addr, uint8_t* output) override;
	AddressInfo GetAbsoluteAddress(uint32_t address) override;

	MemoryMappings* GetMemoryMappings();
	Cx4State& GetState();
};