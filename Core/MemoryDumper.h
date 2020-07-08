#pragma once
#include "stdafx.h"
#include <unordered_map>
#include "DebugTypes.h"

class MemoryManager;
class BaseCartridge;
class Ppu;
class Spc;
class Debugger;
class Disassembler;
enum class SnesMemoryType;

class MemoryDumper
{
private:
	Ppu* _ppu;
	Spc* _spc;
	MemoryManager* _memoryManager;
	BaseCartridge* _cartridge;
	Debugger* _debugger;
	Disassembler* _disassembler;

public:
	MemoryDumper(Debugger* debugger);

	uint8_t* GetMemoryBuffer(SnesMemoryType type);
	uint32_t GetMemorySize(SnesMemoryType type);
	void GetMemoryState(SnesMemoryType type, uint8_t *buffer);

	uint8_t GetMemoryValue(SnesMemoryType memoryType, uint32_t address, bool disableSideEffects = true);
	uint16_t GetMemoryValueWord(SnesMemoryType memoryType, uint32_t address, bool disableSideEffects = true);
	void SetMemoryValueWord(SnesMemoryType memoryType, uint32_t address, uint16_t value, bool disableSideEffects = true);
	void SetMemoryValue(SnesMemoryType memoryType, uint32_t address, uint8_t value, bool disableSideEffects = true);
	void SetMemoryValues(SnesMemoryType memoryType, uint32_t address, uint8_t* data, uint32_t length);
	void SetMemoryState(SnesMemoryType type, uint8_t *buffer, uint32_t length);
};