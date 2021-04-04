#pragma once
#include "stdafx.h"
#include "IMemoryHandler.h"
#include "Utilities/ISerializable.h"

class Console;
class Emulator;
class Ppu;
class Spc;
class Sa1;
class Msu1;
class CheatManager;

class RegisterHandlerB : public IMemoryHandler, public ISerializable
{
private:
	Emulator* _emu;
	Console *_console;
	CheatManager *_cheatManager;
	Ppu *_ppu;
	Spc *_spc;
	Sa1 *_sa1;
	Msu1 *_msu1;

	uint8_t *_workRam;
	uint32_t _wramPosition;

public:
	RegisterHandlerB(Console *console, Ppu *ppu, Spc *spc, uint8_t *workRam);

	uint8_t Read(uint32_t addr) override;
	uint8_t Peek(uint32_t addr) override;
	void PeekBlock(uint32_t addr, uint8_t *output) override;
	void Write(uint32_t addr, uint8_t value) override;

	AddressInfo GetAbsoluteAddress(uint32_t address) override;

	void Serialize(Serializer &s) override;
};