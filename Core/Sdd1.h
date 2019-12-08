#pragma once
#include "stdafx.h"
#include "BaseCoprocessor.h"
#include "Sdd1Types.h"

class Console;
class Sdd1Mmc;

class Sdd1 : public BaseCoprocessor
{
private:
	Sdd1State _state;
	unique_ptr<Sdd1Mmc> _sdd1Mmc;
	IMemoryHandler* _cpuRegisterHandler;

public:
	Sdd1(Console *console);

	void Serialize(Serializer &s) override;
	uint8_t Read(uint32_t addr) override;
	uint8_t Peek(uint32_t addr) override;
	void PeekBlock(uint32_t addr, uint8_t* output) override;
	void Write(uint32_t addr, uint8_t value) override;
	AddressInfo GetAbsoluteAddress(uint32_t address) override;
	void Reset() override;
};