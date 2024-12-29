#pragma once
#include "pch.h"
#include "SNES/Coprocessors/ST018/St018Types.h"
#include "SNES/Coprocessors/ST018/ArmV3Types.h"
#include "SNES/Coprocessors/ST018/ArmV3Cpu.h"
#include "SNES/Coprocessors/BaseCoprocessor.h"

class SnesConsole;
class Emulator;
class ArmV3Cpu;
class SnesMemoryManager;

class St018 final : public BaseCoprocessor
{
private:
	static constexpr int PrgRomSize = 0x20000;
	static constexpr int DataRomSize = 0x8000;
	static constexpr int WorkRamSize = 0x4000;

	SnesConsole* _console = nullptr;
	Emulator* _emu = nullptr;
	SnesMemoryManager* _memoryManager = nullptr;
	St018State _state = {};
	unique_ptr<ArmV3Cpu> _cpu;

	uint8_t* _prgRom = nullptr;
	uint8_t* _dataRom = nullptr;
	uint8_t* _workRam = nullptr;

	__forceinline uint8_t ReadCpuByte(uint32_t addr);
	__forceinline void WriteCpuByte(uint32_t addr, uint8_t value);

	uint8_t GetStatus();

public:
	St018(SnesConsole* console);
	virtual ~St018();

	void Reset() override;
	void Run() override;

	void ProcessEndOfFrame() override;

	void LoadBattery() override;
	void SaveBattery() override;

	ArmV3Cpu* GetCpu() { return _cpu.get(); }

	St018State& GetState();

	uint32_t ReadCpu(ArmV3AccessModeVal mode, uint32_t addr);
	uint32_t DebugCpuRead(ArmV3AccessModeVal mode, uint32_t addr);
	void WriteCpu(ArmV3AccessModeVal mode, uint32_t addr, uint32_t value);
	void ProcessIdleCycle();

	uint8_t DebugRead(uint32_t addr);
	void DebugWrite(uint32_t addr, uint8_t value);

	uint8_t Read(uint32_t addr) override;
	void Write(uint32_t addr, uint8_t value) override;

	uint8_t Peek(uint32_t addr) override;
	void PeekBlock(uint32_t addr, uint8_t* output) override;

	AddressInfo GetAbsoluteAddress(uint32_t address) override;
	
	AddressInfo GetArmAbsoluteAddress(uint32_t address);
	int GetArmRelativeAddress(AddressInfo& absoluteAddr);

	void Serialize(Serializer& s) override;
};