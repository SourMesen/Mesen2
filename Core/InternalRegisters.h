#pragma once
#include "stdafx.h"

class Console;

class InternalRegisters
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
	
	bool _nmiFlag = false;
	bool _enableNmi = false;
	bool _enableHorizontalIrq = false;
	bool _enableVerticalIrq = false;
	uint16_t _horizontalTimer = 0x1FF;
	uint16_t _verticalTimer = 0x1FF;

	uint16_t _controllerData[4];

public:
	InternalRegisters(shared_ptr<Console> console);

	void ProcessAutoJoypadRead();
	void SetNmiFlag(bool nmiFlag);

	bool IsVerticalIrqEnabled() { return _enableVerticalIrq; }
	bool IsHorizontalIrqEnabled() { return _enableHorizontalIrq; }
	bool IsNmiEnabled() { return _enableNmi; }
	uint16_t GetHorizontalTimer() { return _horizontalTimer; }
	uint16_t GetVerticalTimer() { return _verticalTimer; }
	
	uint8_t Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t value);
};