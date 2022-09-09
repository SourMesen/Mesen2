#pragma once
#include "pch.h"
#include "SNES/Coprocessors/BaseCoprocessor.h"

class SnesConsole;
class SnesMemoryManager;
class BsxMemoryPack;
class BsxSatellaview;

class BsxCart : public BaseCoprocessor
{
private:
	SnesConsole* _console;
	SnesMemoryManager* _memoryManager;
	BsxMemoryPack* _memPack;
	unique_ptr<BsxSatellaview> _satellaview;

	uint8_t* _psRam = nullptr;
	uint32_t _psRamSize = 0;
	vector<unique_ptr<IMemoryHandler>> _psRamHandlers;

	uint8_t _regs[0x10] = {};
	uint8_t _dirtyRegs[0x10] = {};
	bool _dirty = false;
	
	void UpdateMemoryMappings();

public:
	BsxCart(SnesConsole* console, BsxMemoryPack* memPack);
	virtual ~BsxCart();

	uint8_t Read(uint32_t addr) override;
	void Write(uint32_t addr, uint8_t value) override;

	void Reset() override;
	void Serialize(Serializer& s) override;

	uint8_t Peek(uint32_t addr) override;
	void PeekBlock(uint32_t addr, uint8_t* output) override;
	AddressInfo GetAbsoluteAddress(uint32_t address) override;

	uint8_t* DebugGetPsRam();
	uint32_t DebugGetPsRamSize();
};