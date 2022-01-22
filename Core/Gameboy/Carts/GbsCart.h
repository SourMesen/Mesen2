#pragma once
#include "stdafx.h"
#include "Gameboy/Carts/GbCart.h"
#include "Gameboy/Gameboy.h"
#include "Gameboy/GbMemoryManager.h"
#include "Shared/Audio/AudioPlayerTypes.h"
#include "Shared/Emulator.h"
#include "Shared/SystemActionManager.h"
#include "Utilities/Serializer.h"

class GbsCart : public GbCart
{
private:
	uint8_t _prgBank = 1;
	uint8_t _currentTrack = 0;
	uint64_t _startClock = 0;
	GbsHeader _header = {};

public:
	GbsCart(GbsHeader header)
	{
		_header = header;
		_currentTrack = header.FirstTrack - 1;
	}

	void InitCart() override
	{
		_memoryManager->MapRegisters(0x2000, 0x3FFF, RegisterAccess::Write);
	}

	void InitPlayback(uint8_t selectedTrack)
	{
		_currentTrack = selectedTrack;

		uint8_t* prg = _gameboy->DebugGetMemory(MemoryType::GbPrgRom);

		//Clear high ram and cart ram
		memset(_cartRam, 0, _gameboy->DebugGetMemorySize(MemoryType::GbCartRam));
		memset(_gameboy->DebugGetMemory(MemoryType::GbHighRam), 0, _gameboy->DebugGetMemorySize(MemoryType::GbHighRam));
		memset(_gameboy->DebugGetMemory(MemoryType::GbWorkRam), 0, _gameboy->DebugGetMemorySize(MemoryType::GbWorkRam));

		//Patch ROM to call INIT and PLAY routines

		//CALL [INIT]
		prg[0] = 0xCD;
		prg[1] = _header.InitAddress[0];
		prg[2] = _header.InitAddress[1];

		//Infinite loop (JP $0003)
		prg[3] = 0xC3;
		prg[4] = 0x03;
		prg[5] = 0x00;

		//CALL [PLAY]
		prg[0x50] = 0xCD;
		prg[0x51] = _header.PlayAddress[0];
		prg[0x52] = _header.PlayAddress[1];

		//RETI
		prg[0x53] = 0xD9;

		GbCpuState& state = _gameboy->GetCpu()->GetState();
		state = {};
		state.SP = _header.StackPointer[0] | (_header.StackPointer[1] << 8);
		state.PC = 0;
		state.A = (uint8_t)selectedTrack;
		state.IME = true; //enable CPU interrupts

		//Disable boot room
		_memoryManager->WriteRegister(0xFF50, 0x01);

		//Turn on PPU for vblank interrupts
		//TODO test and support timer IRQs properly
		_memoryManager->WriteRegister(0xFF40, 0x80);

		//Enable all IRQs
		_memoryManager->WriteRegister(0xFFFF, 0xFF);

		//Enable APU
		_memoryManager->WriteRegister(0xFF26, 0x80);
		_memoryManager->WriteRegister(0xFF25, 0xFF);

		//Clear all IRQ requests (needed when switching tracks)
		_memoryManager->ClearIrqRequest(0xFF);

		_startClock = _gameboy->GetMasterClock();
		_prgBank = 1;
		RefreshMappings();
	}

	AudioTrackInfo GetAudioTrackInfo()
	{
		AudioTrackInfo info = {};
		info.GameTitle = string(_header.Title, 32);
		info.Artist = string(_header.Author, 32);
		info.Comment = string(_header.Copyright, 32);
		info.TrackNumber = _currentTrack + 1;
		info.TrackCount = _header.TrackCount;
		info.Position = (double)(_gameboy->GetMasterClock() - _startClock) / _gameboy->GetMasterClockRate();
		info.Length = 0;
		info.FadeLength = 0;
		return info;
	}

	void ProcessAudioPlayerAction(AudioPlayerActionParams p)
	{
		int selectedTrack = _currentTrack;
		switch(p.Action) {
			case AudioPlayerAction::NextTrack: selectedTrack++; break;
			case AudioPlayerAction::PrevTrack:
				if(GetAudioTrackInfo().Position < 2) {
					selectedTrack--;
				}
				break;
			case AudioPlayerAction::SelectTrack: selectedTrack = (int)p.TrackNumber; break;
		}

		if(selectedTrack < 0) {
			selectedTrack = _header.TrackCount - 1;
		} else if(selectedTrack >= _header.TrackCount) {
			selectedTrack = 0;
		}

		//Asynchronously move to the next file
		//Can't do this in the current thread in some contexts (e.g when track reaches end)
		//because this is called from the emulation thread, which may cause infinite recursion
		thread switchTrackTask([this, selectedTrack]() {
			auto lock = _gameboy->GetEmulator()->AcquireLock();
			InitPlayback(selectedTrack);
		});
		switchTrackTask.detach();
	}

	void RefreshMappings() override
	{
		constexpr int prgBankSize = 0x4000;
		Map(0x0000, 0x3FFF, GbMemoryType::PrgRom, 0, true);
		Map(0x4000, 0x7FFF, GbMemoryType::PrgRom, _prgBank * prgBankSize, true);
		Map(0xA000, 0xFFFF, GbMemoryType::CartRam, 0, false);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		_prgBank = std::max<uint8_t>(1, value);
		RefreshMappings();
	}

	void Serialize(Serializer& s) override
	{
		s.Stream(_prgBank, _currentTrack, _startClock);
	}
};