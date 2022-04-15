#pragma once
#include "stdafx.h"
#include "PCE/PceScsiBus.h"
#include "PCE/PceAdpcm.h"

class PceConsole;
class PceMemoryManager;

enum class PceCdRomIrqSource
{
	Adpcm = 0x04,
	Stop = 0x08,
	SubChannel = 0x10,
	DataTransferDone = 0x20,
	DataTransferReady = 0x40
};

struct PceCdRomState
{
	uint8_t ActiveIrqs = 0;
	uint8_t EnabledIrqs = 0;
	bool ReadRightChannel = false;
	bool BramLocked = false;
};

class PceCdRom
{
private:
	PceConsole* _console = nullptr;
	PceScsiBus _scsi;
	PceAdpcm _adpcm;
	PceCdRomState _state = {};
	
	void UpdateIrqState();

public:
	PceCdRom(PceConsole* console, DiscInfo& disc);

	void Exec();

	void SetIrqSource(PceCdRomIrqSource src);
	void ClearIrqSource(PceCdRomIrqSource src);

	void Write(uint16_t addr, uint8_t value);
	uint8_t Read(uint16_t addr);
};