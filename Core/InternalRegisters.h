#pragma once
#include "stdafx.h"
#include "../Utilities/ISerializable.h"

class Console;

class InternalRegisters : public ISerializable
{
private:
	shared_ptr<Console> _console;

	uint8_t _multOperand1 = 0;
	uint8_t _multOperand2 = 0;
	uint16_t _multOrRemainderResult = 0;

	uint16_t _dividend = 0;
	uint8_t _divisor = 0;
	uint16_t _divResult = 0;

	bool _enableAutoJoypadRead = false;
	bool _enableFastRom = false;
	
	bool _nmiFlag = false;
	bool _enableNmi = false;
	bool _enableHorizontalIrq = false;
	bool _enableVerticalIrq = false;
	uint16_t _horizontalTimer = 0x1FF;
	uint16_t _verticalTimer = 0x1FF;

	uint8_t _ioPortOutput = 0;

	uint16_t _controllerData[4];

public:
	InternalRegisters(shared_ptr<Console> console);

	void ProcessAutoJoypadRead();
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