#pragma once
#include "stdafx.h"
#include "PpuTypes.h"

class Console;

class Ppu
{
public:
	constexpr static uint32_t SpriteRamSize = 544;
	constexpr static uint32_t CgRamSize = 512;
	constexpr static uint32_t VideoRamSize = 0x10000;

private:
	shared_ptr<Console> _console;

	uint16_t _cycle = 0;
	uint16_t _scanline = 0;
	uint32_t _frameCount = 0;
	
	bool _nmiFlag = false;
	bool _enableNmi = false;

	bool _irqFlag = false;
	bool _enableHorizontalIrq = false;
	bool _enableVerticalIrq = false;
	uint16_t _horizontalTimer = 0x1FF;
	uint16_t _verticalTimer = 0x1FF;
	
	uint8_t _bgMode = 0;
	LayerConfig _layerConfig[4];
	
	uint8_t *_vram;
	uint16_t _vramAddress;
	uint8_t _vramIncrementValue;
	uint8_t _vramAddressRemapping;
	bool _vramAddrIncrementOnSecondReg;

	uint8_t _multOperand1 = 0;
	uint8_t _multOperand2 = 0;
	uint16_t _multResult = 0;
	
	uint16_t _cgramAddress;
	uint8_t _cgram[Ppu::CgRamSize];

	uint8_t _spriteRam[Ppu::SpriteRamSize];

	uint16_t *_outputBuffers[2];
	uint16_t *_currentBuffer;

	void RenderTilemap(LayerConfig & config, uint8_t bpp);

public:
	Ppu(shared_ptr<Console> console);
	~Ppu();

	PpuState GetState();

	void Exec();

	void SendFrame();

	uint8_t* GetVideoRam();
	uint8_t* GetCgRam();
	uint8_t* GetSpriteRam();

	uint8_t Read(uint16_t addr);
	void Write(uint32_t addr, uint8_t value);
};