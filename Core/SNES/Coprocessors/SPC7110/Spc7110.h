#pragma once
#include "pch.h"
#include "SNES/Coprocessors/BaseCoprocessor.h"
#include "SNES/Coprocessors/SPC7110/Spc7110Decomp.h"
#include "SNES/Coprocessors/SPC7110/Rtc4513.h"

class SnesConsole;
class Emulator;
class Spc7110Decomp;
class IMemoryHandler;
class BaseCartridge;

class Spc7110 : public BaseCoprocessor
{
private:
	unique_ptr<Spc7110Decomp> _decomp;
	unique_ptr<Rtc4513> _rtc;

	IMemoryHandler* _cpuRegisterHandler = nullptr;
	Emulator* _emu = nullptr;
	SnesConsole* _console = nullptr;
	BaseCartridge* _cart = nullptr;
	bool _useRtc = false;
	uint32_t _realDataRomSize = 0;

	//Decomp
	uint32_t _directoryBase = 0;
	uint8_t _directoryIndex = 0;
	uint16_t _targetOffset = 0;
	uint16_t _dataLengthCounter = 0;
	uint8_t _skipBytes = 0;
	uint8_t _decompFlags = 0;
	uint8_t _decompMode = 0;
	uint32_t _srcAddress = 0;
	uint32_t _decompOffset = 0;
	uint8_t _decompStatus = 0;
	uint8_t _decompBuffer[32];

	//ALU
	uint32_t _dividend = 0;
	uint16_t _multiplier = 0;
	uint16_t _divisor = 0;
	uint32_t _multDivResult = 0;
	uint16_t _remainder = 0;
	uint8_t _aluState = 0;
	uint8_t _aluFlags = 0;

	//Memory mappings
	uint8_t _sramEnabled = 0;
	uint8_t _dataRomBanks[3] = { 0, 1, 2 };
	uint8_t _dataRomSize = 0;

	//Data rom
	uint32_t _readBase = 0;
	uint16_t _readOffset = 0;
	uint16_t _readStep = 0;
	uint8_t _readMode = 0;
	uint8_t _readBuffer = 0;

	void ProcessMultiplication();
	void ProcessDivision();

	void FillReadBuffer();
	void IncrementPosition();
	void IncrementPosition4810();

	void LoadEntryHeader();
	void BeginDecompression();
	uint8_t ReadDecompressedByte();

public:
	Spc7110(SnesConsole* console, bool useRtc);
	
	uint8_t ReadDataRom(uint32_t addr);

	void Serialize(Serializer& s) override;
	uint8_t Read(uint32_t addr) override;
	uint8_t Peek(uint32_t addr) override;
	void PeekBlock(uint32_t addr, uint8_t* output) override;
	void Write(uint32_t addr, uint8_t value) override;
	void UpdateMappings();
	AddressInfo GetAbsoluteAddress(uint32_t address) override;
	void Reset() override;

	void LoadBattery() override;
	void SaveBattery() override;
};