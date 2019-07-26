#pragma once
#include "stdafx.h"
#include "IMemoryHandler.h"
#include "../Utilities/ISerializable.h"

class Console;
class Ppu;
class Spc;
class Sa1;

class RegisterHandlerB : public IMemoryHandler, public ISerializable
{
private:
	Console *_console;
	Ppu *_ppu;
	Spc *_spc;
	Sa1 *_sa1;

	uint8_t *_workRam;
	uint32_t _wramPosition;

public:
	RegisterHandlerB(Console *console, Ppu *ppu, Spc *spc, uint8_t *workRam);

	uint8_t Read(uint32_t addr) override;
	uint8_t Peek(uint32_t addr) override;
	void PeekBlock(uint8_t *output) override;
	void Write(uint32_t addr, uint8_t value) override;

	AddressInfo GetAbsoluteAddress(uint32_t address) override;

	void Serialize(Serializer &s) override;
};