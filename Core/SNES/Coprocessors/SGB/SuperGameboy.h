#pragma once
#include "pch.h"
#include "SNES/Coprocessors/BaseCoprocessor.h"
#include "Shared/Interfaces/IAudioProvider.h"
#include "Utilities/Audio/HermiteResampler.h"

class SnesConsole;
class Emulator;
class SnesMemoryManager;
class BaseCartridge;
class GbControlManager;
class Spc;
class Gameboy;
class GbPpu;

class SuperGameboy : public BaseCoprocessor, public IAudioProvider
{
private:
	SnesConsole* _console = nullptr;
	Emulator* _emu = nullptr;
	SnesMemoryManager* _memoryManager = nullptr;
	GbControlManager* _controlManager = nullptr;
	BaseCartridge* _cart = nullptr;
	Spc* _spc = nullptr;
	Gameboy* _gameboy = nullptr;
	GbPpu* _ppu = nullptr;

	uint8_t _control = 0;
	uint64_t _resetClock = 0;
	double _clockRatio = 0;
	double _effectiveClockRate = 0;
	uint64_t _clockOffset = 0;
	
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

	uint8_t GetLcdRow();
	uint8_t GetLcdBufferRow();
	uint8_t GetPlayerCount();

	void SetInputIndex(uint8_t index);
	void SetInputValue(uint8_t index, uint8_t value);

public:
	SuperGameboy(SnesConsole* console, Gameboy* gameboy);
	~SuperGameboy();

	void Reset() override;
	
	uint8_t Read(uint32_t addr) override;
	void Write(uint32_t addr, uint8_t value) override;

	void Run() override;

	void ProcessInputPortWrite(uint8_t value);

	void LogPacket();

	void WriteLcdColor(uint8_t scanline, uint8_t pixel, uint8_t color);

	void MixAudio(int16_t* out, uint32_t sampleCount, uint32_t sampleRate) override;

	void UpdateClockRatio();
	uint32_t GetClockRate();

	uint8_t GetInputIndex();
	uint8_t GetInput();

	uint8_t Peek(uint32_t addr) override;
	void PeekBlock(uint32_t addr, uint8_t* output) override;
	AddressInfo GetAbsoluteAddress(uint32_t address) override;
	void Serialize(Serializer& s) override;
};