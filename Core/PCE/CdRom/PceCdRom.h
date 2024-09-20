#pragma once
#include "pch.h"
#include "PCE/CdRom/PceScsiBus.h"
#include "PCE/CdRom/PceAdpcm.h"
#include "PCE/CdRom/PceCdAudioPlayer.h"
#include "PCE/CdRom/PceAudioFader.h"
#include "PCE/PceTypes.h"
#include "Shared/MemoryType.h"
#include "Shared/CdReader.h"
#include "Utilities/ISerializable.h"

class Emulator;
class PceConsole;
class PceMemoryManager;

class PceCdRom final : public ISerializable
{
private:
	Emulator* _emu = nullptr;
	PceConsole* _console = nullptr;

	DiscInfo _disc;
	PceScsiBus _scsi;
	PceAdpcm _adpcm;
	PceAudioFader _audioFader;
	PceCdAudioPlayer _audioPlayer;
	PceCdRomState _state = {};
	uint64_t _latchChannelStamp = 0;

	uint8_t* _cdromRam = nullptr;
	uint32_t _cdromRamSize = 0;
	uint8_t* _saveRam = nullptr;
	uint32_t _saveRamSize = 0;
	uint8_t* _orgSaveRam = nullptr;

	void UpdateIrqState();

public:
	PceCdRom(Emulator* emu, PceConsole* console, DiscInfo& disc);
	~PceCdRom();

	void SaveBattery();
	void InitMemoryBanks(uint8_t* readBanks[0x100], uint8_t* writeBanks[0x100], MemoryType bankMemType[0x100], uint8_t* unmappedBank);

	PceCdRomState& GetState() { return _state; }
	PceScsiBusState& GetScsiState() { return _scsi.GetState(); }
	PceAdpcmState& GetAdpcmState() { return _adpcm.GetState(); }
	PceAudioFaderState& GetAudioFaderState() { return _audioFader.GetState(); }
	PceCdAudioPlayerState& GetCdPlayerState() { return _audioPlayer.GetState(); }

	__forceinline void Exec()
	{
		if(_scsi.NeedExec()) {
			_scsi.Exec();
		}
		if(_adpcm.NeedExec()) {
			_adpcm.Exec();
		}
		_audioPlayer.Exec();
	}

	PceCdAudioPlayer& GetAudioPlayer() { return _audioPlayer; }
	PceAudioFader& GetAudioFader() { return _audioFader; }
	
	uint32_t GetCurrentSector();

	void ProcessAudioPlaybackStart();

	void SetIrqSource(PceCdRomIrqSource src);
	void ClearIrqSource(PceCdRomIrqSource src);

	void Write(uint16_t addr, uint8_t value);
	uint8_t Read(uint16_t addr);

	void Serialize(Serializer& s) override;
};