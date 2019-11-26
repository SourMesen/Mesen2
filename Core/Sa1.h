#pragma once
#include "stdafx.h"
#include "BaseCoprocessor.h"
#include "MemoryMappings.h"
#include "Sa1Types.h"

class Console;
class Cpu;
class Sa1Cpu;
class MemoryManager;
class BaseCartridge;

//TODO: Implement write protection flags
//TODO: Bitmap projection at $6000
//TODO: Timers

class Sa1 : public BaseCoprocessor
{
private:
	static constexpr int InternalRamSize = 0x800;

	unique_ptr<Sa1Cpu> _cpu;
	Console* _console;
	MemoryManager* _memoryManager;
	BaseCartridge* _cart;
	Cpu* _snesCpu;

	Sa1State _state = {};
	uint8_t* _iRam;
	
	SnesMemoryType _lastAccessMemType;
	uint8_t _openBus;

	unique_ptr<IMemoryHandler> _iRamHandler;
	unique_ptr<IMemoryHandler> _sa1VectorHandler;
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
	Sa1(Console* console);
	
	void WriteSa1(uint32_t addr, uint8_t value, MemoryOperationType type);
	uint8_t ReadSa1(uint32_t addr, MemoryOperationType type = MemoryOperationType::Read);

	void CpuRegisterWrite(uint16_t addr, uint8_t value);
	uint8_t CpuRegisterRead(uint16_t addr);

	uint8_t ReadCharConvertType1(uint32_t addr);

	// Inherited via BaseCoprocessor
	void Serialize(Serializer & s) override;
	uint8_t Read(uint32_t addr) override;
	uint8_t Peek(uint32_t addr) override;
	void PeekBlock(uint8_t * output) override;
	void Write(uint32_t addr, uint8_t value) override;
	AddressInfo GetAbsoluteAddress(uint32_t address) override;
	
	void Run();
	void Reset() override;

	SnesMemoryType GetSa1MemoryType();
	bool IsSnesCpuFastRomSpeed();
	SnesMemoryType GetSnesCpuMemoryType();

	uint8_t* DebugGetInternalRam();
	uint32_t DebugGetInternalRamSize();

	CpuState GetCpuState();
	MemoryMappings* GetMemoryMappings();
};
