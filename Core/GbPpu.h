#pragma once
#include "stdafx.h"
#include "GbTypes.h"
#include "../Utilities/ISerializable.h"

class Console;
class Gameboy;
class GbMemoryManager;

struct FifoEntry
{
	uint8_t Color;
	uint8_t Attributes;
};

class GbPpu : public ISerializable
{
private:
	Console* _console = nullptr;
	Gameboy* _gameboy = nullptr;
	GbPpuState _state = {};
	GbMemoryManager* _memoryManager = nullptr;
	uint16_t* _outputBuffers[2] = {};
	uint16_t* _currentBuffer = nullptr;

	uint16_t* _eventViewerBuffers[2] = {};
	uint16_t* _currentEventViewerBuffer = nullptr;

	uint8_t* _vram = nullptr;
	uint8_t* _oam = nullptr;
	
	uint64_t _lastFrameTime = 0;

	uint8_t _fifoPosition = 0;
	uint8_t _fifoSize = 0;
	FifoEntry _fifoContent[16];
	uint8_t _shiftedPixels = 0;
	uint8_t _drawnPixels = 0;

	uint8_t _fetcherAttributes = 0;
	uint8_t _fetcherStep = 0;
	uint8_t _fetchColumn = 0;
	uint16_t _fetcherTileAddr = 0;
	uint8_t _fetcherTileLowByte = 0;
	uint8_t _fetcherTileHighByte = 0;
	bool _fetchWindow = false;

	int16_t _fetchSprite = -1;
	int16_t _fetchSpriteOffset = -1;
	uint8_t _spriteCount = 0;
	uint8_t _spriteX[10] = {};
	uint8_t _spriteIndexes[10] = {};

	void ExecCycle();
	void RunSpriteEvaluation();
	void ResetTileFetcher();
	void ClockTileFetcher();
	void PushSpriteToPixelFifo();
	void PushTileToPixelFifo();

	void WriteCgbPalette(uint8_t& pos, uint16_t* pal, bool autoInc, uint8_t value);

public:
	virtual ~GbPpu();

	void Init(Console* console, Gameboy* gameboy, GbMemoryManager* memoryManager, uint8_t* vram, uint8_t* oam);

	GbPpuState GetState();
	uint16_t* GetEventViewerBuffer();
	uint16_t* GetPreviousEventViewerBuffer();
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
