#pragma once
#include "pch.h"
#include "SNES/Coprocessors/BaseCoprocessor.h"
#include "SNES/Coprocessors/SDD1/Sdd1Types.h"

class SnesConsole;
class Sdd1Mmc;

class Sdd1 : public BaseCoprocessor
{
private:
	Sdd1State _state;
	unique_ptr<Sdd1Mmc> _sdd1Mmc;
	IMemoryHandler* _cpuRegisterHandler;

public:
	Sdd1(SnesConsole *console);

	void Serialize(Serializer &s) override;
	uint8_t Read(uint32_t addr) override;
	uint8_t Peek(uint32_t addr) override;
	void PeekBlock(uint32_t addr, uint8_t* output) override;
	void Write(uint32_t addr, uint8_t value) override;
	AddressInfo GetAbsoluteAddress(uint32_t address) override;
	void Reset() override;
};