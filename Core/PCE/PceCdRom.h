#pragma once
#include "stdafx.h"
#include "PCE/PceScsiBus.h"
#include "PCE/PceAdpcm.h"
#include "PCE/PceCdAudioPlayer.h"
#include "PCE/PceTypes.h"
#include "Shared/CdReader.h"
#include "Utilities/ISerializable.h"

class Emulator;
class PceConsole;
class PceMemoryManager;

class PceCdRom : public ISerializable
{
private:
	Emulator* _emu = nullptr;
	PceConsole* _console = nullptr;

	DiscInfo _disc;
	PceScsiBus _scsi;
	PceAdpcm _adpcm;
	PceCdAudioPlayer _audioPlayer;
	PceCdRomState _state = {};
	
	void UpdateIrqState();

public:
	PceCdRom(Emulator* emu, PceConsole* console, DiscInfo& disc);
	~PceCdRom();

	PceCdRomState& GetState() { return _state; }
	PceScsiBusState& GetScsiState() { return _scsi.GetState(); }
	PceAdpcmState& GetAdpcmState() { return _adpcm.GetState(); }
	PceCdAudioPlayerState& GetCdPlayerState() { return _audioPlayer.GetState(); }

	void Exec();

	PceCdAudioPlayer& GetAudioPlayer() { return _audioPlayer; }

	void SetIrqSource(PceCdRomIrqSource src);
	void ClearIrqSource(PceCdRomIrqSource src);

	void Write(uint16_t addr, uint8_t value);
	uint8_t Read(uint16_t addr);

	void Serialize(Serializer& s) override;
};