#pragma once
#include "pch.h"
#include <algorithm>
#include "Utilities/ISerializable.h"
#include "NES/Mappers/FDS/ModChannel.h"
#include "NES/Mappers/FDS/BaseFdsChannel.h"
#include "NES/APU/BaseExpansionAudio.h"

class NesConsole;
struct MapperStateEntry;

class FdsAudio : public BaseExpansionAudio
{
private:
	const uint32_t WaveVolumeTable[4] = { 36, 24, 17, 14 };

	//Register values
	uint8_t _waveTable[64] = {};
	bool _waveWriteEnabled = false;

	BaseFdsChannel _volume;
	ModChannel _mod;

	bool _disableEnvelopes = false;
	bool _haltWaveform = false;

	uint8_t _masterVolume = 0;

	//Internal values
	uint16_t _waveOverflowCounter = 0;
	int32_t _wavePitch = 0;
	uint8_t _wavePosition = 0;
	
	uint8_t _lastOutput = 0;

protected:
	void Serialize(Serializer& s) override;

	void ClockAudio() override;
	void UpdateOutput();

public:
	FdsAudio(NesConsole* console);

	uint8_t ReadRegister(uint16_t addr);
	void WriteRegister(uint16_t addr, uint8_t value);
	
	void GetMapperStateEntries(vector<MapperStateEntry>& entries);
};