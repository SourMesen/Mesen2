#pragma once
#include "stdafx.h"
#include "NecDspTypes.h"
#include "BaseCoprocessor.h"

class Console;
class MemoryManager;
class RamHandler;
enum class CoprocessorType;

class NecDsp final : public BaseCoprocessor
{
private:
	Console* _console = nullptr;
	MemoryManager* _memoryManager = nullptr;
	NecDspState _state = {};
	unique_ptr<RamHandler> _ramHandler;
	CoprocessorType _type;

	double _frequency = 7600000;
	uint32_t _opCode = 0;
	uint32_t *_progRom = nullptr;
	uint16_t *_dataRom = nullptr;
	uint16_t *_ram = nullptr;
	uint16_t _stack[16];

	uint32_t _progSize = 0;
	uint32_t _dataSize = 0;
	uint32_t _ramSize = 0;
	uint32_t _stackSize = 0;

	uint32_t _progMask = 0;
	uint32_t _dataMask = 0;
	uint32_t _ramMask = 0;
	uint32_t _stackMask = 0;

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

	NecDsp(CoprocessorType type, Console* console, vector<uint8_t> &programRom, vector<uint8_t> &dataRom);

public:
	static NecDsp* InitCoprocessor(CoprocessorType type, Console* console, vector<uint8_t> &embeddedFirmware);

	void Reset() override;
	void Run();

	void LoadBattery() override;
	void SaveBattery() override;
	
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