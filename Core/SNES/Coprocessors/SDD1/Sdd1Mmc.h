#pragma once
#include "pch.h"
#include "SNES/IMemoryHandler.h"
#include "SNES/Coprocessors/SDD1/Sdd1Types.h"
#include "SNES/Coprocessors/SDD1/Sdd1Decomp.h"
#include "Utilities/ISerializable.h"

class BaseCartridge;

class Sdd1Mmc : public IMemoryHandler, public ISerializable
{
private:
	Sdd1State* _state;
	vector<unique_ptr<IMemoryHandler>> *_romHandlers;
	uint32_t _handlerMask;
	Sdd1Decomp _decompressor;

	IMemoryHandler* GetHandler(uint32_t addr);
	uint16_t ProcessRomMirroring(uint32_t addr);

public:
	Sdd1Mmc(Sdd1State &state, BaseCartridge *cart);

	uint8_t ReadRom(uint32_t addr);

	// Inherited via IMemoryHandler
	virtual uint8_t Read(uint32_t addr) override;
	virtual uint8_t Peek(uint32_t addr) override;
	virtual void PeekBlock(uint32_t addr, uint8_t * output) override;
	virtual void Write(uint32_t addr, uint8_t value) override;
	virtual AddressInfo GetAbsoluteAddress(uint32_t address) override;

	void Serialize(Serializer &s) override;
};