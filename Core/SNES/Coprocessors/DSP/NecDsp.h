#pragma once
#include "pch.h"
#include "SNES/Coprocessors/DSP/NecDspTypes.h"
#include "SNES/Coprocessors/BaseCoprocessor.h"

class SnesConsole;
class Emulator;
class SnesMemoryManager;
class RamHandler;
enum class CoprocessorType;

class NecDsp final : public BaseCoprocessor
{
public:
	static constexpr uint32_t DataRomReadFlag = 0x80000000;

private:
	SnesConsole* _console = nullptr;
	Emulator* _emu = nullptr;
	SnesMemoryManager* _memoryManager = nullptr;
	NecDspState _state = {};
	unique_ptr<RamHandler> _ramHandler;
	CoprocessorType _type;

	double _frequency = 7600000;
	uint32_t _opCode = 0;
	uint8_t *_progRom = nullptr;
	uint32_t *_prgCache = nullptr;
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

	uint16_t _registerMask = 0;
	bool _inRqmLoop = false;

	void ReadOpCode();

	void RunApuOp(uint8_t aluOperation, uint16_t source);

	void UpdateDataPointer();
	void ExecOp();
	void ExecAndReturn();

	void Jump();
	void Load(uint8_t dest, uint16_t value);
	uint16_t GetSourceValue(uint8_t source);

	uint16_t ReadRom(uint32_t addr);
	
	uint16_t ReadRam(uint32_t addr);
	void WriteRam(uint32_t addr, uint16_t value);

	NecDsp(CoprocessorType type, SnesConsole* console, vector<uint8_t> &programRom, vector<uint8_t> &dataRom);

public:
	virtual ~NecDsp();

	static NecDsp* InitCoprocessor(CoprocessorType type, SnesConsole* console, vector<uint8_t> &embeddedFirmware);

	void Reset() override;
	void Run() override;

	void LoadBattery() override;
	void SaveBattery() override;

	void BuildProgramCache();
	
	uint8_t Read(uint32_t addr) override;
	void Write(uint32_t addr, uint8_t value) override;

	uint32_t GetOpCode(uint32_t addr);
	
	uint8_t Peek(uint32_t addr) override;
	void PeekBlock(uint32_t addr, uint8_t * output) override;
	AddressInfo GetAbsoluteAddress(uint32_t address) override;

	uint8_t* DebugGetProgramRom();
	uint8_t* DebugGetDataRom();
	uint8_t* DebugGetDataRam();
	uint32_t DebugGetProgramRomSize();
	uint32_t DebugGetDataRomSize();
	uint32_t DebugGetDataRamSize();
	NecDspState& GetState();

	void Serialize(Serializer &s) override;
};