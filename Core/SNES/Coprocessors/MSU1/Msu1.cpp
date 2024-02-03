#include "pch.h"
#include "SNES/Coprocessors/MSU1/Msu1.h"
#include "SNES/Spc.h"
#include "Shared/Emulator.h"
#include "Shared/Audio/SoundMixer.h"
#include "Utilities/Serializer.h"
#include "Utilities/FolderUtilities.h"

Msu1* Msu1::Init(Emulator* emu, VirtualFile& romFile, Spc* spc)
{
	string romFolder = romFile.GetFolderPath();
	string romName = FolderUtilities::GetFilename(romFile.GetFileName(), false);
	if(ifstream(FolderUtilities::CombinePath(romFolder, romName + ".msu"))) {
		return new Msu1(emu, romFile, spc);
	} else if(ifstream(FolderUtilities::CombinePath(romFolder, "msu1.rom"))) {
		return new Msu1(emu, romFile, spc);
	} else {
		return nullptr;
	}
}

Msu1::Msu1(Emulator* emu, VirtualFile& romFile, Spc* spc)
{
	_emu = emu;
	_spc = spc;
	_romFolder = romFile.GetFolderPath();
	_romName = FolderUtilities::GetFilename(romFile.GetFileName(), false);
	_dataFile.open(FolderUtilities::CombinePath(_romFolder, _romName) + ".msu", ios::binary);
	if(_dataFile) {
		_trackPath = FolderUtilities::CombinePath(_romFolder, _romName);
	} else {
		_dataFile.open(FolderUtilities::CombinePath(_romFolder, "msu1.rom"), ios::binary);
		_trackPath = FolderUtilities::CombinePath(_romFolder, "track");
	}

	if(_dataFile) {
		_dataFile.seekg(0, ios::end);
		_dataSize = (uint32_t)_dataFile.tellg();
	} else {
		_dataSize = 0;
	}

	_emu->GetSoundMixer()->RegisterAudioProvider(this);
}

Msu1::~Msu1()
{
	_emu->GetSoundMixer()->UnregisterAudioProvider(this);
}

void Msu1::Write(uint16_t addr, uint8_t value)
{
	switch(addr) {
		case 0x2000: _tmpDataPointer = (_tmpDataPointer & 0xFFFFFF00) | value; break;
		case 0x2001: _tmpDataPointer = (_tmpDataPointer & 0xFFFF00FF) | (value << 8); break;
		case 0x2002: _tmpDataPointer = (_tmpDataPointer & 0xFF00FFFF) | (value << 16); break;
		case 0x2003:
			_tmpDataPointer = (_tmpDataPointer & 0x00FFFFFF) | (value << 24);
			_dataPointer = _tmpDataPointer;
			_dataFile.seekg(_dataPointer, ios::beg);
			break;

		case 0x2004: _trackSelect = (_trackSelect & 0xFF00) | value; break;
		case 0x2005:
			_trackSelect = (_trackSelect & 0xFF) | (value << 8);
			LoadTrack();
			break;

		case 0x2006: _volume = value; break;
		case 0x2007:
			if(!_audioBusy) {
				_repeat = (value & 0x02) != 0;
				_paused = (value & 0x01) == 0;
				_pcmReader.SetLoopFlag(_repeat);
			}
			break;
	}
}

uint8_t Msu1::Read(uint16_t addr)
{
	switch(addr) {
		case 0x2000:
			//status
			return (_dataBusy << 7) | (_audioBusy << 6) | (_repeat << 5) | ((!_paused) << 4) | (_trackMissing << 3) | 0x01;

		case 0x2001:
			//data
			if(!_dataBusy && _dataPointer < _dataSize) {
				_dataPointer++;
				return (uint8_t)_dataFile.get();
			}
			return 0;

		case 0x2002: return 'S';
		case 0x2003: return '-';
		case 0x2004: return 'M';
		case 0x2005: return 'S';
		case 0x2006: return 'U';
		case 0x2007: return '1';
	}

	return 0;
}

void Msu1::MixAudio(int16_t* buffer, uint32_t sampleCount, uint32_t sampleRate)
{
	if(!_paused) {
		_pcmReader.SetSampleRate(sampleRate);
		_pcmReader.ApplySamples(buffer, (size_t)sampleCount, _spc->IsMuted() ? 0 : _volume);

		_paused |= _pcmReader.IsPlaybackOver();
	}
}

void Msu1::LoadTrack(uint32_t startOffset)
{
	_trackMissing = !_pcmReader.Init(_trackPath + "-" + std::to_string(_trackSelect) + ".pcm", _repeat, startOffset);
}

void Msu1::Serialize(Serializer &s)
{
	uint32_t offset = _pcmReader.GetOffset();
	SV(_trackSelect); SV(_tmpDataPointer); SV(_dataPointer); SV(_repeat); SV(_paused); SV(_volume); SV(_trackMissing); SV(_audioBusy); SV(_dataBusy); SV(offset);
	if(!s.IsSaving()) {
		_dataFile.seekg(_dataPointer, ios::beg);
		LoadTrack(offset);
	}
}
