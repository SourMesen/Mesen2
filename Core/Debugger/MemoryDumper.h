#pragma once
#include "pch.h"
#include <unordered_map>
#include "Debugger/DebugTypes.h"
#include "Debugger/DebugUtilities.h"
#include "Shared/MemoryType.h"
#include "Utilities/SimpleLock.h"

class SnesMemoryManager;
class NesConsole;
class BaseCartridge;
class Spc;
class Gameboy;
class PceConsole;
class SmsConsole;
class GbaConsole;
class WsConsole;
class Emulator;
class Debugger;

struct UndoEntry
{
	MemoryType MemType;
	uint32_t StartAddress;
	vector<uint8_t> OriginalData;
};

struct UndoBatch
{
	vector<UndoEntry> Entries;
};

class MemoryDumper
{
private:
	Emulator* _emu = nullptr;
	Spc* _spc = nullptr;
	Gameboy* _gameboy = nullptr;
	SnesMemoryManager* _memoryManager = nullptr;
	NesConsole* _nesConsole = nullptr;
	PceConsole* _pceConsole = nullptr;
	SmsConsole* _smsConsole = nullptr;
	GbaConsole* _gbaConsole = nullptr;
	WsConsole* _wsConsole = nullptr;
	BaseCartridge* _cartridge = nullptr;
	Debugger* _debugger = nullptr;
	bool _isMemorySupported[DebugUtilities::GetMemoryTypeCount()] = {};

	SimpleLock _undoLock;
	deque<UndoBatch> _undoHistory;

	uint8_t InternalGetMemoryValue(MemoryType memoryType, uint32_t address, bool disableSideEffects = true);
	void InternalSetMemoryValues(MemoryType memoryType, uint32_t startAddress, uint8_t* data, uint32_t length, bool disableSideEffects, bool undoAllowed);

public:
	MemoryDumper(Debugger* debugger);

	uint8_t* GetMemoryBuffer(MemoryType type);
	uint32_t GetMemorySize(MemoryType type);
	void GetMemoryState(MemoryType type, uint8_t *buffer);

	uint8_t GetMemoryValue(MemoryType memoryType, uint32_t address, bool disableSideEffects = true);
	void GetMemoryValues(MemoryType memoryType, uint32_t start, uint32_t end, uint8_t* output);
	uint16_t GetMemoryValue16(MemoryType memoryType, uint32_t address, bool disableSideEffects = true);
	uint32_t GetMemoryValue32(MemoryType memoryType, uint32_t address, bool disableSideEffects = true);
	void SetMemoryValue16(MemoryType memoryType, uint32_t address, uint16_t value, bool disableSideEffects = true);
	void SetMemoryValue32(MemoryType memoryType, uint32_t address, uint32_t value, bool disableSideEffects);
	void SetMemoryValue(MemoryType memoryType, uint32_t address, uint8_t value, bool disableSideEffects = true);
	void SetMemoryValues(MemoryType memoryType, uint32_t address, uint8_t* data, uint32_t length);
	void SetMemoryState(MemoryType type, uint8_t *buffer, uint32_t length);

	bool HasUndoHistory();
	void PerformUndo();
};