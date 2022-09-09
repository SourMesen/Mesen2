#pragma once
#include "pch.h"
#include "Shared/Interfaces/IAudioProvider.h"
#include "Shared/Audio/PcmReader.h"
#include "Utilities/ISerializable.h"
#include "Utilities/VirtualFile.h"

class Spc;
class Emulator;

class Msu1 final : public ISerializable, public IAudioProvider
{
private:
	Spc* _spc = nullptr;
	Emulator* _emu = nullptr;
	PcmReader _pcmReader;
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