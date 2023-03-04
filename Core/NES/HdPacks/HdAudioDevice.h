#pragma once
#include "pch.h"
#include "NES/INesMemoryHandler.h"
#include "Utilities/ISerializable.h"

struct HdPackData;
class Emulator;
class OggMixer;

class HdAudioDevice : public INesMemoryHandler, public ISerializable
{
private:
	Emulator* _emu = nullptr;
	HdPackData* _hdData = nullptr;
	uint8_t _album = 0;
	uint8_t _playbackOptions = 0;
	bool _trackError = false;
	unique_ptr<OggMixer> _oggMixer;
	int32_t _lastBgmTrack = 0;
	uint8_t _bgmVolume = 0;
	uint8_t _sfxVolume = 0;
	
	bool PlayBgmTrack(int trackId, uint32_t startOffset);
	bool PlaySfx(uint8_t sfxNumber);
	void ProcessControlFlags(uint8_t flags);

protected:
	void Serialize(Serializer& s) override;

public:
	HdAudioDevice(Emulator* emu, HdPackData *hdData);
	~HdAudioDevice();

	void GetMemoryRanges(MemoryRanges &ranges) override;
	void WriteRam(uint16_t addr, uint8_t value) override;
	uint8_t ReadRam(uint16_t addr) override;
};