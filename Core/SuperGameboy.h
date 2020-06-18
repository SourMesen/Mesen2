#pragma once
#include "stdafx.h"
#include "BaseCoprocessor.h"
#include "../Utilities/HermiteResampler.h"

class Console;
class MemoryManager;
class BaseCartridge;
class Gameboy;
class GbPpu;

class SuperGameboy : public BaseCoprocessor
{
private:
	Console* _console = nullptr;
	MemoryManager* _memoryManager = nullptr;
	BaseCartridge* _cart = nullptr;
	Gameboy* _gameboy = nullptr;
	GbPpu* _ppu = nullptr;

	uint8_t _control = 0;
	uint64_t _resetClock = 0;
	
	uint8_t _input[4] = {};
	uint8_t _inputIndex = 0;

	bool _listeningForPacket = false;
	bool _waitForHigh = true;
	bool _packetReady = false;
	uint64_t _inputWriteClock = 0;
	uint8_t _inputValue = 0;	
	uint8_t _packetData[16] = {};
	uint8_t _packetByte = 0;
	uint8_t _packetBit = 0;

	uint8_t _lcdRowSelect = 0;
	uint16_t _readPosition = 0;
	uint8_t _lcdBuffer[4][1280] = {};
	
	HermiteResampler _resampler;
	int16_t* _mixBuffer = nullptr;
	uint32_t _mixSampleCount = 0;

	uint8_t GetLcdRow();
	uint8_t GetLcdBufferRow();
	uint8_t GetPlayerCount();

public:
	SuperGameboy(Console* console);
	~SuperGameboy();

	void Reset() override;
	
	uint8_t Read(uint32_t addr) override;
	void Write(uint32_t addr, uint8_t value) override;

	void Run() override;

	void ProcessInputPortWrite(uint8_t value);

	void WriteLcdColor(uint8_t scanline, uint8_t pixel, uint8_t color);

	void MixAudio(uint32_t targetRate, int16_t* soundSamples, uint32_t sampleCount);

	uint8_t GetControl();
	uint64_t GetResetClock();
	uint8_t GetInputIndex();
	uint8_t GetInput();

	uint8_t Peek(uint32_t addr) override;
	void PeekBlock(uint32_t addr, uint8_t* output) override;
	AddressInfo GetAbsoluteAddress(uint32_t address) override;
	void Serialize(Serializer& s) override;
};