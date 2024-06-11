#pragma once
#include "pch.h"
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

		MessageManager::Log("[GBS] Load: $" + HexUtilities::ToHex(header.LoadAddress[0] | (header.LoadAddress[1] << 8)));
		MessageManager::Log("[GBS] Init: $" + HexUtilities::ToHex(header.InitAddress[0] | (header.InitAddress[1] << 8)));
		MessageManager::Log("[GBS] Play: $" + HexUtilities::ToHex(header.PlayAddress[0] | (header.PlayAddress[1] << 8)));
		MessageManager::Log("[GBS] Timer Control: $" + HexUtilities::ToHex(header.TimerControl));
		MessageManager::Log("[GBS] Timer Modulo: $" + HexUtilities::ToHex(header.TimerModulo));
		MessageManager::Log("-----------------------------");
		
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

		uint16_t loadAddress = _header.LoadAddress[0] | (_header.LoadAddress[1] << 8);

		//Make RST xx calls jump to [LOAD+0/$8/$10/etc]
		for(int i = 0; i <= 0x38; i += 8) {
			//JP LoadAddress+i
			prg[i+0] = 0xC3;
			prg[i+1] = (loadAddress + i) & 0xFF;
			prg[i+2] = ((loadAddress + i) >> 8) & 0xFF;
		}

		//Vertical Blank IRQ: CALL [PLAY]
		prg[0x40] = 0xCD;
		prg[0x41] = _header.PlayAddress[0];
		prg[0x42] = _header.PlayAddress[1];
		prg[0x43] = 0xD9; //RETI

		//Stat IRQ: CALL [PLAY]
		prg[0x48] = 0xCD;
		prg[0x49] = _header.PlayAddress[0];
		prg[0x4A] = _header.PlayAddress[1];
		prg[0x4B] = 0xD9; //RETI

		//Timer IRQ: CALL [PLAY]
		prg[0x50] = 0xCD;
		prg[0x51] = _header.PlayAddress[0];
		prg[0x52] = _header.PlayAddress[1];
		prg[0x53] = 0xD9; //RETI

		//Joypad/Serial IRQ - RETI
		prg[0x58] = 0xD9;
		prg[0x60] = 0xD9;

		//Init (PC starts at 0x70), CALL [INIT]
		prg[0x70] = 0xCD;
		prg[0x71] = _header.InitAddress[0];
		prg[0x72] = _header.InitAddress[1];
		//Infinite loop
		prg[0x73] = 0xC3;
		prg[0x74] = 0x73;
		prg[0x75] = 0x00;

		//Disable boot room
		_memoryManager->WriteRegister(0xFF50, 0x01);

		//Enable timer, vblank and scanline IRQs
		//(enabling joypad irq seems to break some (most?) GBS files because no joypad irq handler is defined)
		_memoryManager->WriteRegister(0xFFFF, GbIrqSource::Timer | GbIrqSource::LcdStat | GbIrqSource::VerticalBlank);

		//Enable APU
		_memoryManager->WriteRegister(0xFF26, 0x80);
		_memoryManager->WriteRegister(0xFF25, 0xFF);

		//Clear all IRQ requests (needed when switching tracks)
		_memoryManager->ClearIrqRequest(0xFF);

		if((_header.TimerControl & 0x04) == 0) {
			//Turn off PPU to restart it at the top of the frame
			_memoryManager->WriteRegister(0xFF40, 0x00);

			//Turn on PPU for vblank interrupts, since timer is disabled
			_memoryManager->WriteRegister(0xFF40, 0x80);
		} else {
			//Turn off PPU (timer is used instead of vblank irqs)
			_memoryManager->WriteRegister(0xFF40, 0x00);

			//Use timer for IRQs
			if((_header.TimerControl & 0x80) && !_memoryManager->IsHighSpeed()) {
				//2x clock rate (CGB)
				_memoryManager->ToggleSpeed();
			}

			_memoryManager->WriteRegister(0xFF06, _header.TimerModulo);
			_memoryManager->WriteRegister(0xFF07, _header.TimerControl & 0x07);
		}

		//Clear any pending IRQ processing
		_gameboy->GetCpu()->PowerOn();

		GbCpuState& state = _gameboy->GetCpu()->GetState();
		state = {};
		state.SP = _header.StackPointer[0] | (_header.StackPointer[1] << 8);
		state.PC = 0x70;
		state.A = (uint8_t)selectedTrack;
		state.IME = true; //enable CPU interrupts

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
		Emulator* emu = _gameboy->GetEmulator();
		thread switchTrackTask([emu, selectedTrack]() {
			auto lock = emu->AcquireLock(false);
			((Gameboy*)emu->GetConsole().get())->InitGbsPlayback((uint8_t)selectedTrack);
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
		SV(_prgBank); SV(_currentTrack); SV(_startClock);
	}
};