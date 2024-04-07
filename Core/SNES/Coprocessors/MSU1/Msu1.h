#pragma once
#include "pch.h"
#include "Shared/Interfaces/IAudioProvider.h"

#include "Shared/Audio/PcmReader.h"
#include "Shared/Audio/OggReader.h"
#include "Utilities/ISerializable.h"
#include "Utilities/VirtualFile.h"

class Spc;
class Emulator;

class PcmOrOggReader final {
private:
	PcmReader _pcmReader;
	OggReader _oggReader;
	bool _usingOgg = false;

public:
	void SetLoopFlag(bool loop) {
		if (_usingOgg) { _oggReader.SetLoopFlag(loop); }
		else { _pcmReader.SetLoopFlag(loop); }
	}
	void SetSampleRate(uint32_t sampleRate) {
		if (_usingOgg) { _oggReader.SetSampleRate(sampleRate); }
		else { _pcmReader.SetSampleRate(sampleRate); }
	}
	void ApplySamples(int16_t* buffer, size_t sampleCount, uint8_t volume) {
		if (_usingOgg) { _oggReader.ApplySamples(buffer, sampleCount, volume); } 
		else { _pcmReader.ApplySamples(buffer, sampleCount, volume); }
	}
	bool IsPlaybackOver() {
		return _usingOgg ? _oggReader.IsPlaybackOver() : _pcmReader.IsPlaybackOver();
	}
	uint32_t GetOffset() {
		return _usingOgg ? _oggReader.GetOffset() * 4 : _pcmReader.GetOffset();
	}
	bool Init(string filenameNoExt, bool loop, uint32_t startOffset) {
		if (_pcmReader.Init(filenameNoExt + ".pcm", loop, startOffset)) {
			_usingOgg = false;
			return true;
		}
		if (_oggReader.Init(filenameNoExt + ".ogg", loop, 44100, startOffset / 4)) {
			_usingOgg = true;
			return true;
		}
		_usingOgg = false;
		return false;
	}
};

class Msu1 final : public ISerializable, public IAudioProvider
{
private:
	Spc* _spc = nullptr;
	Emulator* _emu = nullptr;
	PcmOrOggReader _soundReader;
	uint8_t _volume = 100;
	uint16_t _trackSelect = 0;
	uint32_t _tmpDataPointer = 0;
	uint32_t _dataPointer = 0;
	string _romName;
	string _romFolder;
	string _trackPath;

	bool _repeat = false;
	bool _paused = false;
	bool _audioBusy = false; //Always false
	bool _dataBusy = false; //Always false
	bool _trackMissing = false;

	ifstream _dataFile;
	uint32_t _dataSize;
	
	void LoadTrack(uint32_t startOffset = 8);

public:
	Msu1(Emulator* emu, VirtualFile& romFile, Spc* spc);
	~Msu1();

	static Msu1* Init(Emulator* emu, VirtualFile& romFile, Spc* spc);

	void Write(uint16_t addr, uint8_t value);
	uint8_t Read(uint16_t addr);
	
	void MixAudio(int16_t* buffer, uint32_t sampleCount, uint32_t sampleRate) override;
	
	void Serialize(Serializer &s) override;
};
