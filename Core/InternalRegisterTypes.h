#pragma once
#include "stdafx.h"

struct AluState
{
	uint8_t MultOperand1;
	uint8_t MultOperand2;
	uint16_t MultOrRemainderResult;

	uint16_t Dividend;
	uint8_t Divisor;
	uint16_t DivResult;
};

struct InternalRegisterState
{
	bool EnableAutoJoypadRead;
	bool EnableFastRom;

	bool EnableNmi;
	bool EnableHorizontalIrq;
	bool EnableVerticalIrq;
	uint16_t HorizontalTimer;
	uint16_t VerticalTimer;

	uint8_t IoPortOutput;

	uint16_t ControllerData[4];
};
