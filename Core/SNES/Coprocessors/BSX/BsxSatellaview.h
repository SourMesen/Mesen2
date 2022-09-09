#pragma once
#include "pch.h"
#include "SNES/IMemoryHandler.h"
#include "SNES/Coprocessors/BSX/BsxStream.h"
#include "Utilities/ISerializable.h"

class SnesConsole;
class SnesMemoryManager;

class BsxSatellaview : public IMemoryHandler, public ISerializable
{
private:
	IMemoryHandler* _bBusHandler;
	SnesConsole* _console;
	SnesMemoryManager* _memoryManager;

	BsxStream _stream[2];
	uint8_t _streamReg;
	uint8_t _extOutput;
	int64_t _customDate;

	uint64_t _prevMasterClock;

	void ProcessClocks();

public:
	BsxSatellaview(SnesConsole* console, IMemoryHandler *bBusHandler);

	void Reset();

	uint8_t Read(uint32_t addr) override;
	uint8_t Peek(uint32_t addr) override;
	void PeekBlock(uint32_t addr, uint8_t* output) override;
	void Write(uint32_t addr, uint8_t value) override;
	AddressInfo GetAbsoluteAddress(uint32_t address) override;

	void Serialize(Serializer& s) override;
};