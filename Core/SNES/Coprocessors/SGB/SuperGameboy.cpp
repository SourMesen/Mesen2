#include "pch.h"
#include "SNES/Coprocessors/SGB/SuperGameboy.h"
#include "SNES/SnesConsole.h"
#include "SNES/SnesMemoryManager.h"
#include "SNES/BaseCartridge.h"
#include "SNES/Spc.h"
#include "Gameboy/Gameboy.h"
#include "Gameboy/GbControlManager.h"
#include "Gameboy/APU/GbApu.h"
#include "Gameboy/GbPpu.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/MessageManager.h"
#include "Shared/Audio/SoundMixer.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/Audio/HermiteResampler.h"

SuperGameboy::SuperGameboy(SnesConsole* console, Gameboy* gameboy)
{
	_console = console;
	_emu = console->GetEmulator();
	_memoryManager = console->GetMemoryManager();
	_cart = _console->GetCartridge();
	_spc = _console->GetSpc();
	
	_gameboy = gameboy;
	_controlManager = (GbControlManager*)gameboy->GetControlManager();
	_ppu = gameboy->GetPpu();

	_control = 0x01; //Divider = 5, gameboy = not running
	UpdateClockRatio();
	
	MemoryMappings* cpuMappings = _memoryManager->GetMemoryMappings();
	for(int i = 0; i <= 0x3F; i++) {
		cpuMappings->RegisterHandler(i, i, 0x6000, 0x7FFF, this);
		cpuMappings->RegisterHandler(i + 0x80, i + 0x80, 0x6000, 0x7FFF, this);
	}

	_emu->GetSoundMixer()->RegisterAudioProvider(this);
}

SuperGameboy::~SuperGameboy()
{
	_emu->GetSoundMixer()->UnregisterAudioProvider(this);
}

void SuperGameboy::Reset()
{
	_control = 0;
	_resetClock = 0;

	memset(_input, 0, sizeof(_input));
	_inputIndex = 0;

	_listeningForPacket = false;
	_waitForHigh = true;
	_packetReady = false;
	_inputWriteClock = 0;
	_inputValue = 0;
	memset(_packetData, 0, sizeof(_packetData));
	_packetByte = 0;
	_packetBit = 0;

	_lcdRowSelect = 0;
	_readPosition = 0;
	memset(_lcdBuffer, 0, sizeof(_lcdBuffer));
}

uint8_t SuperGameboy::Read(uint32_t addr)
{
	addr &= 0xF80F;
	
	if(addr >= 0x7000 && addr <= 0x700F) {
		_packetReady = false;
		return _packetData[addr & 0x0F];
	} else if(addr >= 0x7800 && addr <= 0x780F) {
		if(_readPosition >= 320) {
			//Return 0xFF for 320..511 and then wrap to 0
			_readPosition = (_readPosition + 1) & 0x1FF;
			return 0xFF;
		}

		uint8_t* start = _lcdBuffer[_lcdRowSelect];
		start += ((_readPosition >> 1) & 0x07) * 160;
		start += (_readPosition >> 4) * 8;

		uint8_t data = 0;
		uint8_t shift = _readPosition & 0x01;
		for(int i = 0; i < 8; i++) {
			data |= ((start[i] >> shift) & 0x01) << (7 - i);
		}
		_readPosition++;
		return data;
	} else {
		switch(addr & 0xFFFF) {
			case 0x6000: return (GetLcdRow() << 3) | GetLcdBufferRow();
			case 0x6002: return _packetReady;
			case 0x600F: return 0x21; //or 0x61
		}
	}

	return 0;
}

void SuperGameboy::Write(uint32_t addr, uint8_t value)
{
	addr &= 0xF80F;

	switch(addr & 0xFFFF) {
		case 0x6001:
			_lcdRowSelect = value & 0x03; 
			_readPosition = 0;
			break;

		case 0x6003: {
			if(!(_control & 0x80) && (value & 0x80)) {
				_clockOffset = 0;
				_resetClock = _memoryManager->GetMasterClock();
				_gameboy->PowerOn(this);
			}
			_control = value;
			SetInputIndex(_inputIndex % GetPlayerCount());

			UpdateClockRatio();
			break;
		}

		case 0x6004: SetInputValue(0, value); break;
		case 0x6005: SetInputValue(1, value); break;
		case 0x6006: SetInputValue(2, value); break;
		case 0x6007: SetInputValue(3, value); break;
	}
}

void SuperGameboy::ProcessInputPortWrite(uint8_t value)
{
	if(_inputValue == value) {
		return;
	}

	if(value == 0x00) {
		//Reset pulse
		_waitForHigh = true;
		_packetByte = 0;
		_packetBit = 0;
	} else if(_waitForHigh) {
		if(value == 0x10 || value == 0x20) {
			//Invalid sequence (should be 0x00 -> 0x30 -> 0x10/0x20 -> 0x30 -> 0x10/0x20, etc.)
			_waitForHigh = false;
			_listeningForPacket = false;
		} else if(value == 0x30) {
			_waitForHigh = false;
			_listeningForPacket = true;
		}
	} else if(_listeningForPacket) {
		if(value == 0x20) {
			//0 bit
			if(_packetByte >= 16 && _packetBit == 0) {
				_packetReady = true;
				_listeningForPacket = false;

				if(_emu->IsDebugging()) {
					LogPacket();
				}
			} else {
				_packetData[_packetByte] &= ~(1 << _packetBit);
			}
			_packetBit++;
			if(_packetBit == 8) {
				_packetBit = 0;
				_packetByte++;
			}
		} else if(value == 0x10) {
			//1 bit
			if(_packetByte >= 16) {
				//Invalid bit
				_listeningForPacket = false;
			} else {
				_packetData[_packetByte] |= (1 << _packetBit);
				_packetBit++;
				if(_packetBit == 8) {
					_packetBit = 0;
					_packetByte++;
				}
			}
		}
		_waitForHigh = _listeningForPacket;
	} else if(!(_inputValue & 0x20) && (value & 0x20)) {
		SetInputIndex((_inputIndex + 1) % GetPlayerCount());
	}

	_inputValue = value;
	_inputWriteClock = _memoryManager->GetMasterClock();
}

void SuperGameboy::LogPacket()
{
	uint8_t commandId = _packetData[0] >> 3;
	string name;
	switch(commandId) {
		case 0: name = "PAL01"; break; //Set SGB Palette 0, 1 Data
		case 1: name = "PAL23"; break; //Set SGB Palette 2, 3 Data
		case 2: name = "PAL03"; break; //Set SGB Palette 0, 3 Data
		case 3: name = "PAL12"; break; //Set SGB Palette 1, 2 Data
		case 4: name = "ATTR_BLK"; break; //"Block" Area Designation Mode
		case 5: name = "ATTR_LIN"; break; //"Line" Area Designation Mode
		case 6: name = "ATTR_DIV"; break; //"Divide" Area Designation Mode
		case 7: name = "ATTR_CHR"; break; //"1CHR" Area Designation Mode
		case 8: name = "SOUND"; break; //Sound On / Off
		case 9: name = "SOU_TRN"; break; //Transfer Sound PRG / DATA
		case 0xA: name = "PAL_SET"; break; //Set SGB Palette Indirect
		case 0xB: name = "PAL_TRN"; break; //Set System Color Palette Data
		case 0xC: name = "ATRC_EN"; break; //Enable / disable Attraction Mode
		case 0xD: name = "TEST_EN"; break; //Speed Function
		case 0xE: name = "ICON_EN"; break; //SGB Function
		case 0xF: name = "DATA_SND"; break; //SUPER NES WRAM Transfer 1
		case 0x10: name = "DATA_TRN"; break; //SUPER NES WRAM Transfer 2
		case 0x11: name = "MLT_REG"; break; //Controller 2 Request
		case 0x12: name = "JUMP"; break; //Set SNES Program Counter
		case 0x13: name = "CHR_TRN"; break; //Transfer Character Font Data
		case 0x14: name = "PCT_TRN"; break; //Set Screen Data Color Data
		case 0x15: name = "ATTR_TRN"; break; //Set Attribute from ATF
		case 0x16: name = "ATTR_SET"; break; //Set Data to ATF
		case 0x17: name = "MASK_EN"; break; //Game Boy Window Mask
		case 0x18: name = "OBJ_TRN"; break; //Super NES OBJ Mode
		
		case 0x1E: name = "Header Data"; break;
		case 0x1F: name = "Header Data"; break;

		default: name = "Unknown"; break;
	}

	string log = "SGB Command: " + HexUtilities::ToHex(commandId) + " - " + name + " (Len: " + std::to_string(_packetData[0] & 0x07) + ") - ";
	for(int i = 0; i < 16; i++) {
		log += HexUtilities::ToHex(_packetData[i]) + " ";
	}
	_emu->DebugLog(log);
}

void SuperGameboy::WriteLcdColor(uint8_t scanline, uint8_t pixel, uint8_t color)
{
	_lcdBuffer[GetLcdBufferRow()][(scanline & 0x07) * 160 + pixel] = color;
}

uint8_t SuperGameboy::GetLcdRow()
{
	uint8_t scanline = _ppu->GetScanline();
	uint8_t row = scanline / 8;
	if(row >= 18) {
		row = 0;
	}
	return row;
}

uint8_t SuperGameboy::GetLcdBufferRow()
{
	return (_ppu->GetFrameCount() * 18 + GetLcdRow()) & 0x03;
}

uint8_t SuperGameboy::GetPlayerCount()
{
	uint8_t playerCount = ((_control >> 4) & 0x03) + 1;
	if(playerCount >= 3) {
		//Unknown: 2 and 3 both mean 4 players?
		return 4;
	}
	return playerCount;
}

void SuperGameboy::MixAudio(int16_t* out, uint32_t sampleCount, uint32_t sampleRate)
{
	int16_t* gbSamples = nullptr;
	uint32_t gbSampleCount = 0;
	_gameboy->GetSoundSamples(gbSamples, gbSampleCount);
	
	if(!_spc->IsMuted()) {
		_resampler.SetSampleRates(GbApu::SampleRate * _effectiveClockRate / _gameboy->GetMasterClockRate(), sampleRate);
		_resampler.Resample<true>(gbSamples, gbSampleCount, out, sampleCount, true);
	}
}

void SuperGameboy::Run()
{
	if(!(_control & 0x80)) {
		return;
	}

	_gameboy->Run(_clockOffset + (uint64_t)((_memoryManager->GetMasterClock() - _resetClock) * _clockRatio));
}

void SuperGameboy::UpdateClockRatio()
{
	bool isSgb2 = _emu->GetSettings()->GetGameboyConfig().UseSgb2;
	uint32_t masterRate = isSgb2 ? 20971520 : _console->GetMasterClockRate();
	uint8_t divider = 5;

	switch(_control & 0x03) {
		case 0: divider = 4; break;
		case 1: divider = 5; break;
		case 2: divider = 7; break;
		case 3: divider = 9; break;
	}

	double effectiveRate = (double)masterRate / divider;
	if(effectiveRate != _effectiveClockRate) {
		_effectiveClockRate = effectiveRate;

		double clockRatio = _effectiveClockRate / _console->GetMasterClockRate();
		Run();
		_clockOffset = _gameboy->GetCycleCount();
		_resetClock = _memoryManager->GetMasterClock();
		_clockRatio = clockRatio;
	}
}

uint32_t SuperGameboy::GetClockRate()
{
	return (uint32_t)(_console->GetMasterClockRate() * _clockRatio);
}

uint8_t SuperGameboy::GetInputIndex()
{
	return 0xF - _inputIndex;
}

uint8_t SuperGameboy::GetInput()
{
	return _input[_inputIndex];
}

void SuperGameboy::SetInputIndex(uint8_t index)
{
	_controlManager->ProcessInputChange([=]() { _inputIndex = index; });
}

void SuperGameboy::SetInputValue(uint8_t index, uint8_t value)
{
	if(_inputIndex == index) {
		_controlManager->ProcessInputChange([=]() { _input[index] = value; });
	} else {
		_input[index] = value;
	}
}

uint8_t SuperGameboy::Peek(uint32_t addr)
{
	return 0;
}

void SuperGameboy::PeekBlock(uint32_t addr, uint8_t* output)
{
	memset(output, 0, 0x1000);
}

AddressInfo SuperGameboy::GetAbsoluteAddress(uint32_t address)
{
	return { -1, MemoryType::None };
}

void SuperGameboy::Serialize(Serializer& s)
{
	SV(_control); SV(_resetClock); SV(_input[0]); SV(_input[1]); SV(_input[2]); SV(_input[3]); SV(_inputIndex); SV(_listeningForPacket); SV(_packetReady);
	SV(_inputWriteClock); SV(_inputValue); SV(_packetByte); SV(_packetBit); SV(_lcdRowSelect); SV(_readPosition); SV(_waitForHigh); SV(_clockRatio);

	SVArray(_packetData, 16);
	SVArray(_lcdBuffer[0], 1280);
	SVArray(_lcdBuffer[1], 1280);
	SVArray(_lcdBuffer[2], 1280);
	SVArray(_lcdBuffer[3], 1280);
}
