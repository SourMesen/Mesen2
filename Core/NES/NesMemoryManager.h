#pragma once

#include "pch.h"

#include "NES/INesMemoryHandler.h"
#include "NES/OpenBusHandler.h"
#include "NES/InternalRamHandler.h"
#include "Shared/MemoryOperationType.h"
#include "Utilities/ISerializable.h"

class BaseMapper;
class CheatManager;
class Emulator;
class NesConsole;

class NesMemoryManager : public ISerializable
{
private:
	static constexpr int CpuMemorySize = 0x10000;
	static const int NesInternalRamSize = 0x800;
	static const int FamicomBoxInternalRamSize = 0x2000;

	Emulator* _emu = nullptr;
	CheatManager* _cheatManager = nullptr;
	NesConsole* _console = nullptr;
	BaseMapper* _mapper = nullptr;

	uint8_t* _internalRam = nullptr;
	uint32_t _internalRamSize = 0;

	OpenBusHandler _openBusHandler = {};
	unique_ptr<INesMemoryHandler> _internalRamHandler;
	INesMemoryHandler** _ramReadHandlers = nullptr;
	INesMemoryHandler** _ramWriteHandlers = nullptr;

	void InitializeMemoryHandlers(INesMemoryHandler** memoryHandlers, INesMemoryHandler* handler, vector<uint16_t>* addresses, bool allowOverride);

protected:
	void Serialize(Serializer& s) override;

public:
	NesMemoryManager(NesConsole* console, BaseMapper* mapper);
	virtual ~NesMemoryManager();

	void Reset(bool softReset);
	void RegisterIODevice(INesMemoryHandler* handler);
	void RegisterWriteHandler(INesMemoryHandler* handler, uint32_t start, uint32_t end);
	void RegisterReadHandler(INesMemoryHandler* handler, uint32_t start, uint32_t end);
	void UnregisterIODevice(INesMemoryHandler* handler);

	uint8_t DebugRead(uint16_t addr);
	uint16_t DebugReadWord(uint16_t addr);
	void DebugWrite(uint16_t addr, uint8_t value, bool disableSideEffects = true);

	uint8_t* GetInternalRam();

	uint8_t Read(uint16_t addr, MemoryOperationType operationType = MemoryOperationType::Read);
	void Write(uint16_t addr, uint8_t value, MemoryOperationType operationType);

	uint8_t GetOpenBus(uint8_t mask = 0xFF);
	uint8_t GetInternalOpenBus(uint8_t mask = 0xFF);
};