#pragma once
#include "stdafx.h"
#include "GbTypes.h"
#include "../Utilities/ISerializable.h"

class Console;
class Gameboy;
class GbMemoryManager;

class GbPpu : public ISerializable
{
private:
	Console* _console = nullptr;
	Gameboy* _gameboy = nullptr;
	GbPpuState _state = {};
	GbMemoryManager* _memoryManager = nullptr;
	uint16_t* _outputBuffers[2] = {};
	uint16_t* _currentBuffer;
	uint8_t* _vram = nullptr;
	uint8_t* _oam = nullptr;
	
	uint64_t _lastFrameTime = 0;
	uint16_t _drawModeLength = 140;

	void ExecCycle();
	void RenderScanline();
	
	template<bool isCgb>
	void RenderScanline();

	void WriteCgbPalette(uint8_t& pos, uint16_t* pal, bool autoInc, uint8_t value);

public:
	virtual ~GbPpu();

	void Init(Console* console, Gameboy* gameboy, GbMemoryManager* memoryManager, uint8_t* vram, uint8_t* oam);

	GbPpuState GetState();
	void GetPalette(uint16_t out[4], uint8_t palCfg);

	void Exec();

	void SendFrame();

	uint8_t Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t value);

	uint8_t ReadVram(uint16_t addr);
	void WriteVram(uint16_t addr, uint8_t value);

	uint8_t ReadOam(uint8_t addr);
	void WriteOam(uint8_t addr, uint8_t value);

	uint8_t ReadCgbRegister(uint16_t addr);
	void WriteCgbRegister(uint16_t addr, uint8_t value);
	
	void Serialize(Serializer& s) override;
};
