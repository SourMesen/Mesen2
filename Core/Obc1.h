#pragma once
#include "stdafx.h"
#include "BaseCoprocessor.h"

class Console;

class Obc1 : public BaseCoprocessor
{
private:
	uint8_t *_ram;
	uint32_t _mask;

	uint16_t GetBaseAddress();
	uint16_t GetLowAddress();
	uint16_t GetHighAddress();

	uint8_t ReadRam(uint16_t addr);
	void WriteRam(uint16_t addr, uint8_t value);

public:
	Obc1(Console* console, uint8_t* saveRam, uint32_t saveRamSize);

	void Reset() override;

	uint8_t Read(uint32_t addr) override;
	void Write(uint32_t addr, uint8_t value) override;

	void Serialize(Serializer & s) override;
	
	uint8_t Peek(uint32_t addr) override;
	void PeekBlock(uint8_t * output) override;
	AddressInfo GetAbsoluteAddress(uint32_t address) override;
};