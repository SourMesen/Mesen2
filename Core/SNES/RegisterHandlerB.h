#pragma once
#include "pch.h"
#include "SNES/IMemoryHandler.h"
#include "Utilities/ISerializable.h"

class SnesConsole;
class Emulator;
class SnesPpu;
class Spc;
class Sa1;
class Msu1;
class CheatManager;

class RegisterHandlerB : public IMemoryHandler, public ISerializable
{
private:
	Emulator* _emu;
	SnesConsole *_console;
	CheatManager *_cheatManager;
	SnesPpu *_ppu;
	Spc *_spc;
	Sa1 *_sa1;
	Msu1 *_msu1;

	uint8_t *_workRam;
	uint32_t _wramPosition;

public:
	RegisterHandlerB(SnesConsole *console, SnesPpu *ppu, Spc *spc, uint8_t *workRam);

	uint8_t Read(uint32_t addr) override;
	uint8_t Peek(uint32_t addr) override;
	void PeekBlock(uint32_t addr, uint8_t *output) override;
	void Write(uint32_t addr, uint8_t value) override;

	AddressInfo GetAbsoluteAddress(uint32_t address) override;
	
	uint32_t GetWramPosition();

	void Serialize(Serializer &s) override;
};