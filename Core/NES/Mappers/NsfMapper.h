#pragma once
#include "NES/BaseMapper.h"

enum class NsfIrqType
{
	Init = 0,
	Stop = 1,
	Play = 2,
	None = 0xFF
};

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
	
	bool _needInit = false;
	bool _irqEnabled = false;
	uint32_t _irqReloadValue = 0;
	uint32_t _irqCounter = 0;
	NsfIrqType _irqStatus = NsfIrqType::None;
	NsfIrqType _debugIrqStatus = NsfIrqType::None;
	uint8_t _mmc5MultiplierValues[2];

	int32_t _trackEndCounter;
	int32_t _trackFadeCounter;
	int32_t _fadeLength;
	uint32_t _silenceDetectDelay;
	bool _trackEnded;
	bool _allowSilenceDetection;

	bool _hasBankSwitching;

	uint16_t _ntscSpeed;
	uint16_t _palSpeed;
	uint16_t _dendySpeed;

	uint8_t _songNumber = 0;
	
	/*****************************************
	* NSF BIOS by Quietust, all credits to him!
	* Taken from the Nintendulator source code:
	* http://www.qmtpro.com/~nes/nintendulator/
	* See below for assembly source code
	******************************************/
	uint8_t _nsfBios[256] = {
		0xFF,0xFF,0xFF,0x78,0xA2,0xFF,0x8E,0x17,0x40,0xE8,0x20,0x30,0x3F,0x8E,0x00,0x20,
		0x8E,0x01,0x20,0x8E,0x12,0x3E,0x58,0x4C,0x17,0x3F,0x48,0x8A,0x48,0x98,0x48,0xAE,
		0x12,0x3E,0xF0,0x59,0xCA,0xF0,0xDC,0x20,0xF9,0x3F,0x68,0xA8,0x68,0xAA,0x68,0x40,
		0x8E,0x15,0x40,0xAD,0x13,0x3E,0x4A,0x90,0x09,0x8E,0x02,0x90,0x8E,0x02,0xA0,0x8E,
		0x02,0xB0,0x4A,0x90,0x0D,0xA0,0x20,0x8C,0x10,0x90,0x8E,0x30,0x90,0xC8,0xC0,0x26,
		0xD0,0xF5,0x4A,0x90,0x0B,0xA0,0x80,0x8C,0x83,0x40,0x8C,0x87,0x40,0x8C,0x89,0x40,
		0x4A,0x90,0x03,0x8E,0x15,0x50,0x4A,0x90,0x08,0xCA,0x8E,0x00,0xF8,0xE8,0x8E,0x00,
		0x48,0x4A,0x90,0x08,0xA0,0x07,0x8C,0x00,0xC0,0x8C,0x00,0xE0,0x60,0x20,0x30,0x3F,
		0x8A,0xCA,0x9A,0x8E,0xF7,0x5F,0xCA,0x8E,0xF6,0x5F,0xA2,0x7F,0x85,0x00,0x86,0x01,
		0xA8,0xA2,0x27,0x91,0x00,0xC8,0xD0,0xFB,0xCA,0x30,0x0A,0xC6,0x01,0xE0,0x07,0xD0,
		0xF2,0x86,0x01,0xF0,0xEE,0xA2,0x14,0xCA,0x9D,0x00,0x40,0xD0,0xFA,0xA2,0x07,0xBD,
		0x08,0x3E,0x9D,0xF8,0x5F,0xCA,0x10,0xF7,0xA0,0x0F,0x8C,0x15,0x40,0xAD,0x13,0x3E,
		0x29,0x04,0xF0,0x10,0xAD,0x0E,0x3E,0xEA,0xEA,0x8D,0xF6,0x5F,0xAD,0x0F,0x3E,0xEA,
		0xEA,0x8D,0xF7,0x5F,0xAE,0x11,0x3E,0xBD,0x04,0x3E,0x8D,0x10,0x3E,0xBD,0x06,0x3E,
		0x8D,0x11,0x3E,0x8C,0x12,0x3E,0xAD,0x12,0x3E,0x58,0xAD,0x10,0x3E,0x20,0xF6,0x3F,
		0x8D,0x13,0x3E,0x4C,0x17,0x3F,0x6C,0x00,0x3E,0x6C,0x02,0x3E,0x03,0x3F,0x1A,0x3F
	};

private:
	void TriggerIrq(NsfIrqType type);
	void ClearIrq();

	bool HasBankSwitching();
	uint32_t GetClockRate();

	void InternalSelectTrack(uint8_t trackNumber, bool requestReset = true);
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

/*
;BIOS Source Code
;NSF BIOS by Quietust, all credits to him!
;Taken from the Nintendulator source code:
;http://www.qmtpro.com/~nes/nintendulator/

;NSF Mapper
;R $3E00-$3E01 - INIT address
;R $3E02-$3E03 - PLAY address
;R $3E04/$3E06 - NTSC speed value
;R $3E05/$3E07 - PAL speed value
;R $3E08-$3E0F - Initial bankswitch values
;R $3E10 - Song Number (start at 0)
;R $3E11 - NTSC/PAL setting (0 for NTSC, 1 for PAL)
;W $3E10-$3E11 - IRQ counter (for PLAY)
;R $3E12 - IRQ status (0=INIT, 1=STOP, 2+=PLAY)
;W $3E12 - IRQ enable (for PLAY)
;R $3E13 - Sound chip info
;W $3E13 - clear watchdog timer

.org $3F00
.db $FF,$FF,$FF			;pad to 256 bytes
reset:SEI
		LDX #$FF
		STX $4017	;kill frame IRQs
		INX
		JSR silence	;silence all sound channels
		STX $2000	;we don't rely on the presence of a PPU
		STX $2001	;but we don't want it getting in the way
		STX $3E12	;kill PLAY timer
		CLI
loop:	JMP loop

irq:	PHA
		TXA
		PHA
		TYA
		PHA
		LDX $3E12		;check IRQ status
		BEQ init
		DEX
		BEQ reset		;"Proper" way to play a tune
		JSR song_play	;1) Call the play address of the music at periodic intervals determined by the speed words.
		PLA
		TAY
		PLA
		TAX
		PLA
		RTI

.module	silence
silence:				;X=0 coming in, must also be coming out
		STX $4015	;silence all sound channels
		LDA $3E13
		LSR A
		BCC _1
		STX $9002
		STX $A002
		STX $B002	;stop VRC6
_1:	LSR A
		BCC _2
		LDY #$20
__2:	STY $9010
		STX $9030	;stop VRC7
		INY
		CPY #$26
		BNE __2
_2:	LSR A
		BCC _3
		LDY #$80
		STY $4083
		STY $4087
		STY $4089	;stop FDS
_3:	LSR A
		BCC _4
		STX $5015	;stop MMC5
_4:	LSR A
		BCC _5
		DEX
		STX $F800
		INX
		STX $4800	;stop Namco-163
_5:	LSR A
		BCC _6
		LDY #$07
		STY $C000
		STY $E000	;stop SUN5
_6:		RTS

.module	init		;"Proper" way to init a tune
init:	JSR silence
		TXA			;(X=0)
		DEX
		TXS			;clear the stack
		STX $5FF7	;1.5) Map RAM to $6000-$7FFF
		DEX
		STX $5FF6	;(banks FE/FF are treated as RAM)

		LDX #$7F
		STA $00
		STX $01
		TAY
		LDX #$27
_1:	STA ($00),Y	;1) Clear all RAM at $0000-$07FF.
		INY			;2) Clear all RAM at $6000-$7FFF.
		BNE _1
		DEX
		BMI _2
		DEC $01
		CPX #$07
		BNE _1
		STX $01
		BEQ _1

_2:	LDX #$14
_3:	DEX
		STA $4000,X		;3) Init the sound registers by writing $00 to $4000-$4013
		BNE _3

		LDX #$07
_4:	LDA $3E08,X		;5) If this is a banked tune, load the bank values from the header into $5FF8-$5FFF.
		STA $5FF8,X		;For this player, *all* tunes are considered banked (the loader will fill in appropriate values)
		DEX
		BPL _4

		LDY #$0F			;4) Set volume register $4015 to $0F.
		STY $4015

		LDA $3E13		;For FDS games, banks E/F also get mapped to 6/7
		AND #$04			;For non-FDS games, we don't want to do this
		BEQ _6
		LDA $3E0E		;if these are zero, let's leave them as plain RAM (EDIT: Always write the value, even if it's 0)
		;BEQ _5        
		NOP
		NOP
		STA $5FF6
_5:	LDA $3E0F		;TODO - need to allow FDS NSF data to be writeable
		;BEQ _6        ;EDIT: Always write the value, even if it's 0
		NOP
		NOP
		STA $5FF7

_6:	LDX $3E11		;4.5) Set up the PLAY timer. Which word to use is determined by which mode you are in - PAL or NTSC.
		LDA $3E04,X		;X is 0 for NTSC and 1 for PAL, as needed for #6 below
		STA $3E10
		LDA $3E06,X
		STA $3E11
		STY $3E12
		LDA $3E12		;if we have a pending IRQ here, we need to cancel it NOW
		CLI				;re-enable interrupts now - this way, we can allow the init routine to not return (ex: for raw PCM)

		LDA $3E10		;6) Set the accumulator and X registers for the desired song.
		JSR song_init	;7) Call the music init routine.
		STA $3E13		;kill the watchdog timer (and fire a 'Play' interrupt after 1 frame; otherwise, it'll wait 4 frames)
		JMP loop			;we don't actually RTI from here; this way, the init code is faster

.module	song
song_init:	JMP ($3E00)
song_play:	JMP ($3E02)
.dw reset,irq	;no NMI vector
.end
*/
