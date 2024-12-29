#pragma once
#include "pch.h"
#include "SNES/Coprocessors/BaseCoprocessor.h"
#include "SNES/MemoryMappings.h"
#include "SNES/Coprocessors/SA1/Sa1Types.h"

class SnesConsole;
class Emulator;
class SnesCpu;
class Sa1Cpu;
class SnesMemoryManager;
class BaseCartridge;

enum class MemoryOperationType;

//TODO: Implement write protection flags
//TODO: Timers

class Sa1 : public BaseCoprocessor
{
private:
	static constexpr int InternalRamSize = 0x800;

	unique_ptr<Sa1Cpu> _cpu;
	SnesConsole* _console;
	Emulator* _emu;
	SnesMemoryManager* _memoryManager;
	BaseCartridge* _cart;
	SnesCpu* _snesCpu;

	Sa1State _state = {};
	uint8_t* _iRam;
	
	MemoryType _lastAccessMemType;
	uint8_t _openBus;

	unique_ptr<IMemoryHandler> _iRamHandler;
	unique_ptr<IMemoryHandler> _bwRamHandler;
	unique_ptr<IMemoryHandler> _cpuVectorHandler;
	
	vector<unique_ptr<IMemoryHandler>> _cpuBwRamHandlers;

	MemoryMappings _mappings;
	
	void UpdateBank(uint8_t index, uint8_t value);
	void UpdatePrgRomMappings();
	void UpdateVectorMappings();
	void UpdateSaveRamMappings();

	void IncVarLenPosition();
	void CalculateMathOpResult();
	void RunCharConvertType2();
	
	void ProcessInterrupts();
	void RunDma();

	void Sa1RegisterWrite(uint16_t addr, uint8_t value);
	uint8_t Sa1RegisterRead(uint16_t addr);

	void WriteSharedRegisters(uint16_t addr, uint8_t value);

	void WriteInternalRam(uint32_t addr, uint8_t value);
	void WriteBwRam(uint32_t addr, uint8_t value);

public:
	Sa1(SnesConsole* console);
	virtual ~Sa1();
	
	void WriteSa1(uint32_t addr, uint8_t value, MemoryOperationType type);
	uint8_t ReadSa1(uint32_t addr, MemoryOperationType type = MemoryOperationType::Read);

	void CpuRegisterWrite(uint16_t addr, uint8_t value);
	uint8_t CpuRegisterRead(uint16_t addr);

	uint8_t ReadCharConvertType1(uint32_t addr);

	// Inherited via BaseCoprocessor
	void Serialize(Serializer & s) override;
	uint8_t Read(uint32_t addr) override;
	uint8_t Peek(uint32_t addr) override;
	void PeekBlock(uint32_t addr, uint8_t *output) override;
	void Write(uint32_t addr, uint8_t value) override;
	AddressInfo GetAbsoluteAddress(uint32_t address) override;
	
	void Run() override;
	void Reset() override;

	MemoryType GetSa1MemoryType();
	bool IsSnesCpuFastRomSpeed();
	MemoryType GetSnesCpuMemoryType();

	uint8_t* DebugGetInternalRam();
	uint32_t DebugGetInternalRamSize();

	Sa1State& GetState();
	SnesCpuState& GetCpuState();

	uint16_t ReadVector(uint16_t vector);
	MemoryMappings* GetMemoryMappings();
	void LoadBattery() override;
	void SaveBattery() override;
};
