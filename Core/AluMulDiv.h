#pragma once
#include "stdafx.h"
#include "../Utilities/ISerializable.h"

class Cpu;

class AluMulDiv final : public ISerializable
{
private:
	Cpu *_cpu;

	uint64_t _prevCpuCycle = 0;

	uint8_t _multOperand1 = 0xFF;
	uint8_t _multOperand2 = 0;
	uint16_t _multOrRemainderResult = 0;

	uint16_t _dividend = 0xFFFF;
	uint8_t _divisor = 0;
	uint16_t _divResult = 0;

	uint32_t _shift = 0;
	uint8_t _multCounter = 0;
	uint8_t _divCounter = 0;
	
public:
	AluMulDiv(Cpu *cpu);
	
	void Run(bool isRead);

	uint8_t Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t value);

	void Serialize(Serializer &s) override;
};