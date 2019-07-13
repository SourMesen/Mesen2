#pragma once
#include "stdafx.h"
#include "AluMulDiv.h"
#include "../Utilities/ISerializable.h"

class Console;
class Ppu;
class MemoryManager;

class InternalRegisters final : public ISerializable
{
private:
	Console* _console;
	Ppu* _ppu;
	MemoryManager* _memoryManager;

	AluMulDiv _aluMulDiv;

	bool _enableAutoJoypadRead = false;
	bool _enableFastRom = false;
	
	bool _nmiFlag = false;
	bool _enableNmi = false;
	
	bool _enableHorizontalIrq = false;
	bool _enableVerticalIrq = false;
	uint16_t _horizontalTimer = 0x1FF;
	uint16_t _verticalTimer = 0x1FF;
	bool _irqLevel = false;
	bool _needIrq = false;

	uint8_t _ioPortOutput = 0;

	uint16_t _controllerData[4];

public:
	InternalRegisters(Console* console);

	void Reset();

	void ProcessAutoJoypadRead();
	void ProcessIrqCounters();
	uint8_t GetIoPortOutput();
	void SetNmiFlag(bool nmiFlag);

	bool IsVerticalIrqEnabled() { return _enableVerticalIrq; }
	bool IsHorizontalIrqEnabled() { return _enableHorizontalIrq; }
	bool IsNmiEnabled() { return _enableNmi; }
	bool IsFastRomEnabled() { return _enableFastRom; }
	uint16_t GetHorizontalTimer() { return _horizontalTimer; }
	uint16_t GetVerticalTimer() { return _verticalTimer; }
	
	uint8_t Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t value);

	void Serialize(Serializer &s) override;
};