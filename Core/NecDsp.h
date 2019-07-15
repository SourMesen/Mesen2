#pragma once
#include "stdafx.h"
#include "NecDspTypes.h"
#include "BaseCoprocessor.h"

class Console;
class MemoryManager;
enum class CoprocessorType;

class NecDsp final : public BaseCoprocessor
{
private:
	static constexpr int DspClockRate = 7600000;

	Console* _console = nullptr;
	MemoryManager* _memoryManager = nullptr;
	NecDspState _state = {};

	uint32_t _opCode = 0;
	uint32_t _progRom[2048] = {};
	uint16_t _dataRom[1024] = {};
	uint16_t _ram[256] = {};
	uint16_t _stack[4] = {};
	uint64_t _cycleCount = 0;
	uint16_t _registerMask = 0;
	bool _inRqmLoop = false;

	void RunApuOp(uint8_t aluOperation, uint16_t source);

	void UpdateDataPointer();
	void ExecOp();
	void ExecAndReturn();

	void Jump();
	void Load(uint8_t dest, uint16_t value);
	uint16_t GetSourceValue(uint8_t source);

	NecDsp(Console* console, vector<uint8_t> &biosRom, uint16_t registerMask);

	static bool LoadBios(string combinedFilename, string splitFilenameProgram, string splitFilenameData, vector<uint8_t> &biosData);

public:
	static NecDsp* InitCoprocessor(CoprocessorType type, Console* console);

	void Reset();
	void Run();
	
	uint8_t Read(uint32_t addr) override;
	void Write(uint32_t addr, uint8_t value) override;
	
	uint8_t Peek(uint32_t addr) override;
	void PeekBlock(uint8_t * output) override;
	AddressInfo GetAbsoluteAddress(uint32_t address) override;

	uint8_t* DebugGetProgramRom();
	uint8_t* DebugGetDataRom();
	uint8_t* DebugGetDataRam();
	uint32_t DebugGetProgramRomSize();
	uint32_t DebugGetDataRomSize();
	uint32_t DebugGetDataRamSize();
	NecDspState GetState();

	void Serialize(Serializer &s) override;
};