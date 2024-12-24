#pragma once
#include "pch.h"
#include "SNES/MemoryMappings.h"
#include "Debugger/DebugTypes.h"
#include "Utilities/ISerializable.h"
#include "Shared/MemoryType.h"

class IMemoryHandler;
class RegisterHandlerA;
class RegisterHandlerB;
class InternalRegisters;
class RamHandler;
class BaseCartridge;
class Emulator;
class SnesConsole;
class SnesPpu;
class SnesCpu;
class CheatManager;
enum class MemoryOperationType;

enum class SnesEventType : uint8_t
{
	HdmaInit,
	DramRefresh,
	HdmaStart,
	EndOfScanline
};

class SnesMemoryManager : public ISerializable
{
public:
	constexpr static uint32_t WorkRamSize = 0x20000;

private:
	SnesConsole* _console = nullptr;
	Emulator* _emu = nullptr;

	unique_ptr<RegisterHandlerA> _registerHandlerA;
	unique_ptr<RegisterHandlerB> _registerHandlerB;

	InternalRegisters *_regs = nullptr;
	SnesPpu* _ppu = nullptr;
	SnesCpu* _cpu = nullptr;
	BaseCartridge* _cart = nullptr;
	CheatManager* _cheatManager = nullptr;
	uint8_t *_workRam = nullptr;

	uint64_t _masterClock = 0;
	uint16_t _hClock = 0;
	uint16_t _nextEventClock = 0;
	uint16_t _dramRefreshPosition = 0;
	SnesEventType _nextEvent = SnesEventType::DramRefresh;
	MemoryType _memTypeBusA = MemoryType::SnesPrgRom;

	uint8_t _cpuSpeed = 8;
	uint8_t _openBus = 0;

	MemoryMappings _mappings = {};
	vector<unique_ptr<IMemoryHandler>> _workRamHandlers;
	uint8_t _masterClockTable[0x800] = {};

	typedef void(SnesMemoryManager::*Func)();
	Func _execRead = nullptr;
	Func _execWrite = nullptr;
	
	template<uint8_t clocks> void IncMasterClock();
	void UpdateExecCallbacks();

	__forceinline void Exec();

	void ProcessEvent();

public:
	void Initialize(SnesConsole* console);
	virtual ~SnesMemoryManager();

	void Reset();

	void GenerateMasterClockTable();

	void IncMasterClock4();
	void IncMasterClock6();
	void IncMasterClock8();
	void IncMasterClock40();
	void IncMasterClockStartup();
	void IncrementMasterClockValue(uint16_t value);

	uint8_t Read(uint32_t addr, MemoryOperationType type);
	uint8_t ReadDma(uint32_t addr, bool forBusA);

	uint8_t Peek(uint32_t addr);
	uint16_t PeekWord(uint32_t addr);
	void PeekBlock(uint32_t addr, uint8_t * dest);

	void Write(uint32_t addr, uint8_t value, MemoryOperationType type);
	void WriteDma(uint32_t addr, uint8_t value, bool forBusA);

	uint8_t GetOpenBus();
	uint64_t GetMasterClock();
	uint16_t GetHClock();
	uint8_t* DebugGetWorkRam();

	MemoryMappings* GetMemoryMappings();

	uint8_t GetCpuSpeed(uint32_t addr);
	uint8_t GetCpuSpeed();
	void SetCpuSpeed(uint8_t speed);
	MemoryType GetMemoryTypeBusA();

	bool IsRegister(uint32_t cpuAddress);
	bool IsWorkRam(uint32_t cpuAddress);

	uint32_t GetWramPosition();

	void Serialize(Serializer &s) override;
};
