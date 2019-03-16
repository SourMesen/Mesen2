#pragma once
#include "stdafx.h"
#include "DebugTypes.h"
#include "../Utilities/ISerializable.h"

class IMemoryHandler;
class RegisterHandlerA;
class RegisterHandlerB;
class InternalRegisters;
class RamHandler;
class Console;
class Ppu;
enum class MemoryOperationType;

class MemoryManager : public ISerializable
{
public:
	constexpr static uint32_t WorkRamSize = 0x20000;

private:
	shared_ptr<Console> _console;

	shared_ptr<RegisterHandlerA> _registerHandlerA;
	shared_ptr<RegisterHandlerB> _registerHandlerB;

	InternalRegisters *_regs;
	shared_ptr<Ppu> _ppu;

	IMemoryHandler* _handlers[0x100 * 0x10];
	vector<unique_ptr<RamHandler>> _workRamHandlers;

	uint8_t *_workRam;
	uint8_t _openBus;

	uint64_t _masterClock;
	uint8_t _masterClockTable[2][0x10000];

	__forceinline void Exec();

public:
	void Initialize(shared_ptr<Console> console);
	~MemoryManager();

	void Reset();

	void RegisterHandler(uint32_t startAddr, uint32_t endAddr, IMemoryHandler* handler);

	void GenerateMasterClockTable();
	void IncrementMasterClock(uint32_t addr);
	template<uint16_t value>
	void IncrementMasterClockValue();
	void IncrementMasterClockValue(uint16_t value);

	uint8_t Read(uint32_t addr, MemoryOperationType type);
	uint8_t ReadDma(uint32_t addr, bool forBusA);
	uint8_t Peek(uint32_t addr);
	uint16_t PeekWord(uint32_t addr);

	void Write(uint32_t addr, uint8_t value, MemoryOperationType type);
	void WriteDma(uint32_t addr, uint8_t value, bool forBusA);

	uint8_t GetOpenBus();
	uint64_t GetMasterClock();
	uint8_t* DebugGetWorkRam();

	bool IsRegister(uint32_t cpuAddress);
	bool IsWorkRam(uint32_t cpuAddress);
	AddressInfo GetAbsoluteAddress(uint32_t addr);
	int GetRelativeAddress(AddressInfo &address, int32_t cpuAddress = -1);

	void Serialize(Serializer &s) override;
};

template<uint16_t value>
void MemoryManager::IncrementMasterClockValue()
{
	uint16_t cyclesToRun = value;
	while(cyclesToRun >= 2) {
		cyclesToRun -= 2;
		Exec();
	}
}