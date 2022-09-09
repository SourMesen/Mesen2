#pragma once
#include "pch.h"

struct AluState
{
	uint8_t MultOperand1 = 0;
	uint8_t MultOperand2 = 0;
	uint16_t MultOrRemainderResult = 0;

	uint16_t Dividend = 0;
	uint8_t Divisor = 0;
	uint16_t DivResult = 0;
};

struct InternalRegisterState
{
	bool EnableAutoJoypadRead = false;
	bool EnableFastRom = false;

	bool EnableNmi = false;
	bool EnableHorizontalIrq = false;
	bool EnableVerticalIrq = false;
	uint16_t HorizontalTimer = 0;
	uint16_t VerticalTimer = 0;

	uint8_t IoPortOutput = 0;

	uint16_t ControllerData[4] = {};
};
