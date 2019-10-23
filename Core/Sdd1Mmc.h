#pragma once
#include "stdafx.h"
#include "IMemoryHandler.h"
#include "Sdd1Types.h"
#include "Sdd1Decomp.h"
#include "../Utilities/ISerializable.h"

class BaseCartridge;

class Sdd1Mmc : public IMemoryHandler, public ISerializable
{
private:
	Sdd1State* _state;
	vector<unique_ptr<IMemoryHandler>> *_romHandlers;
	uint32_t _handlerMask;
	Sdd1Decomp _decompressor;

	IMemoryHandler* GetHandler(uint32_t addr);

public:
	Sdd1Mmc(Sdd1State &state, BaseCartridge *cart);

	uint8_t ReadRom(uint32_t addr);

	// Inherited via IMemoryHandler
	virtual uint8_t Read(uint32_t addr) override;
	virtual uint8_t Peek(uint32_t addr) override;
	virtual void PeekBlock(uint8_t * output) override;
	virtual void Write(uint32_t addr, uint8_t value) override;
	virtual AddressInfo GetAbsoluteAddress(uint32_t address) override;

	void Serialize(Serializer &s) override;
};