#pragma once
#include "NES/BaseMapper.h"

class Mmc5Audio;
class Vrc6Audio;
class Vrc7Audio;
class FdsAudio;
class Mmc5Audio;
class Namco163Audio;
class Sunsoft5bAudio;

class NsfMapper : public BaseMapper
{
private:
	enum NsfSoundChips
	{
		VRC6 = 0x01,
		VRC7 = 0x02,
		FDS = 0x04,
		MMC5 = 0x08,
		Namco = 0x10,
		Sunsoft = 0x20
	};

	EmuSettings* _settings;

	NsfHeader _nsfHeader;
	unique_ptr<Mmc5Audio> _mmc5Audio;
	unique_ptr<Vrc6Audio> _vrc6Audio;
	unique_ptr<Vrc7Audio> _vrc7Audio;
	unique_ptr<FdsAudio> _fdsAudio;
	unique_ptr<Namco163Audio> _namcoAudio;
	unique_ptr<Sunsoft5bAudio> _sunsoftAudio;
	
	uint8_t _mmc5MultiplierValues[2];

	uint32_t _irqCounter = 0;
	int32_t _trackEndCounter;
	int32_t _trackFadeCounter;
	int32_t _fadeLength;
	uint32_t _silenceDetectDelay;
	bool _trackEnded;
	bool _allowSilenceDetection;

	bool _hasBankSwitching;

	uint8_t _songNumber = 0;
	
	/*
	
	.org $3F00	
	reset:
		CLI
		JSR [INIT] ; set at load time
		JSR [PLAY] ; set at load time
	 loop:
		JMP loop    ;loop forever

	.org $3F10
	 irq:
		STA $3F00   ;clear irq
		JSR [PLAY]  ;set at load time
		RTI
	*/

	uint8_t _nsfBios[0x20] = {
		0x58,0x20,0x00,0x00,0x20,0x00,0x00,0x4C,0x07,0x3F,0x00,0x00,0x00,0x00,0x00,0x00,
		0x8D,0x00,0x3F,0x20,0x00,0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
	};

private:
	void TriggerIrq();
	void ClearIrq();

	bool HasBankSwitching();

	void InternalSelectTrack(uint8_t trackNumber);
	void ClockLengthAndFadeCounters();
	void SelectNextTrack();

protected:
	uint16_t GetPRGPageSize() override { return 0x1000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }
	uint32_t GetWorkRamSize() override { return 0x4000; }
	uint32_t GetWorkRamPageSize() override { return 0x1000; }
	bool AllowRegisterRead() override { return true; }

	void Serialize(Serializer& s) override;

	void InitMapper() override;
	void InitMapper(RomData& romData) override;
	void Reset(bool softReset) override;
	void GetMemoryRanges(MemoryRanges &ranges) override;

	uint32_t GetIrqReloadValue();
	
	void ProcessCpuClock() override;
	uint8_t ReadRegister(uint16_t addr) override;
	void WriteRegister(uint16_t addr, uint8_t value) override;

public:
	NsfMapper();
	~NsfMapper();
	
	ConsoleFeatures GetAvailableFeatures() override;

	AudioTrackInfo GetAudioTrackInfo();
	void ProcessAudioPlayerAction(AudioPlayerActionParams p);

	void SelectTrack(uint8_t trackNumber);
	uint8_t GetCurrentTrack();
	NsfHeader GetNsfHeader();
};
