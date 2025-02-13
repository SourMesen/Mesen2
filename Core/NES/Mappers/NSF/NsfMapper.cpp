#include "pch.h"
#include <random>
#include "NES/Mappers/NSF/NsfMapper.h"
#include "NES/NesConstants.h"
#include "NES/NesConsole.h"
#include "NES/NesCpu.h"
#include "NES/NesMemoryManager.h"
#include "NES/Mappers/Audio/Mmc5Audio.h"
#include "NES/Mappers/Audio/Vrc6Audio.h"
#include "NES/Mappers/Audio/Vrc7Audio.h"
#include "NES/Mappers/FDS/FdsAudio.h"
#include "NES/Mappers/Audio/Namco163Audio.h"
#include "NES/Mappers/Audio/Sunsoft5bAudio.h"
#include "Shared/EmuSettings.h"
#include "Shared/SystemActionManager.h"
#include "Utilities/Serializer.h"

NsfMapper::NsfMapper()
{
}

NsfMapper::~NsfMapper()
{
}

void NsfMapper::InitMapper()
{
	_settings = _console->GetEmulator()->GetSettings();

	//Clear all register settings
	RemoveRegisterRange(0x0000, 0xFFFF, MemoryOperation::Any);

	AddRegisterRange(0x3F00, 0x3F1F, MemoryOperation::Read);
	AddRegisterRange(0x3F00, 0x3F00, MemoryOperation::Write);

	//NSF registers
	AddRegisterRange(0x5FF6, 0x5FFF, MemoryOperation::Write);
}

void NsfMapper::InitMapper(RomData& romData)
{
	_nsfHeader = romData.Info.NsfInfo;

	_songNumber = _nsfHeader.StartingSong - 1;

	if(_nsfHeader.SoundChips & NsfSoundChips::MMC5) {
		AddRegisterRange(0x5000, 0x5015, MemoryOperation::Write); //Registers
		AddRegisterRange(0x5205, 0x5206, MemoryOperation::Any); //Multiplication
		SetCpuMemoryMapping(0x5C00, 0x5FFF, PrgMemoryType::WorkRam, 0x3000, MemoryAccessType::ReadWrite); //Exram
	}

	if(_nsfHeader.SoundChips & NsfSoundChips::VRC6) {
		AddRegisterRange(0x9000, 0x9003, MemoryOperation::Write);
		AddRegisterRange(0xA000, 0xA002, MemoryOperation::Write);
		AddRegisterRange(0xB000, 0xB002, MemoryOperation::Write);
	}
	
	if(_nsfHeader.SoundChips & NsfSoundChips::VRC7) {
		AddRegisterRange(0x9010, 0x9010, MemoryOperation::Write);
		AddRegisterRange(0x9030, 0x9030, MemoryOperation::Write);
	}

	if(_nsfHeader.SoundChips & NsfSoundChips::Namco) {
		AddRegisterRange(0x4800, 0x4FFF, MemoryOperation::Any);
		AddRegisterRange(0xF800, 0xFFFF, MemoryOperation::Write);
	}

	if(_nsfHeader.SoundChips & NsfSoundChips::Sunsoft) {
		AddRegisterRange(0xC000, 0xFFFF, MemoryOperation::Write);
	}

	if(_nsfHeader.SoundChips & NsfSoundChips::FDS) {
		AddRegisterRange(0x4040, 0x4092, MemoryOperation::Any);
	}

	//Reset/IRQ vector
	AddRegisterRange(0xFFFC, 0xFFFF, MemoryOperation::Read);

	//Set PLAY/INIT addresses in NSF code
	_nsfBios[0x02] = _nsfHeader.InitAddress & 0xFF;
	_nsfBios[0x03] = (_nsfHeader.InitAddress >> 8) & 0xFF;
	_nsfBios[0x14] = _nsfHeader.PlayAddress & 0xFF;
	_nsfBios[0x15] = (_nsfHeader.PlayAddress >> 8) & 0xFF;
}

void NsfMapper::Reset(bool softReset)
{
	if(!softReset) {
		_songNumber = _nsfHeader.StartingSong - 1;
	}

	_mmc5Audio.reset(new Mmc5Audio(_console));
	_vrc6Audio.reset(new Vrc6Audio(_console));
	_vrc7Audio.reset(new Vrc7Audio(_console));
	_fdsAudio.reset(new FdsAudio(_console));
	_namcoAudio.reset(new Namco163Audio(_console));
	_sunsoftAudio.reset(new Sunsoft5bAudio(_console));
}

void NsfMapper::OnAfterResetPowerOn()
{
	//Do this after reset/power on sequence is over, otherwise cpu reset will reset all these values

	//INIT logic
	NesMemoryManager* mm = _console->GetMemoryManager();

	//Clear internal + work RAM
	memset(mm->GetInternalRam(), 0, GetInternalRamSize());
	memset(_workRam, 0, _workRamSize);

	//Reset APU
	for(uint16_t i = 0x4000; i < 0x4013; i++) {
		mm->Write(i, 0, MemoryOperationType::Write);
	}

	mm->Write(0x4015, 0, MemoryOperationType::Write);
	mm->Write(0x4015, 0x0F, MemoryOperationType::Write);
	mm->Write(0x4017, 0x40, MemoryOperationType::Write);

	//Disable PPU
	mm->Write(0x2000, 0, MemoryOperationType::Write);
	mm->Write(0x2001, 0, MemoryOperationType::Write);

	//Setup bankswitching
	_hasBankSwitching = HasBankSwitching();
	if(!_hasBankSwitching) {
		//Update bank config to select the right banks on init when no bankswitching is setup in header
		int8_t startBank = (_nsfHeader.LoadAddress / 0x1000);
		for(int32_t i = 0; i < (int32_t)GetPrgPageCount(); i++) {
			if((startBank + i) > 0x0F) {
				break;
			}
			if(startBank + i - 8 >= 0) {
				_nsfHeader.BankSetup[startBank + i - 8] = i;
			}
		}
	}

	for(uint16_t i = 0; i < 8; i++) {
		WriteRegister(0x5FF8 + i, _nsfHeader.BankSetup[i]);
	}

	if(_nsfHeader.SoundChips & NsfSoundChips::FDS) {
		WriteRegister(0x5FF6, _nsfHeader.BankSetup[6]);
		WriteRegister(0x5FF7, _nsfHeader.BankSetup[7]);
	}

	NesCpuState& state = _console->GetCpu()->GetState();
	state.A = _songNumber;
	state.X = (_nsfHeader.Flags & 0x01) ? 1 : 0; //PAL = 1, NTSC = 0
	state.Y = 0;
	state.SP = 0xFD;

	//Disable Frame Counter & DMC interrupts
	_console->GetCpu()->SetIrqMask((uint8_t)IRQSource::External);
	_irqCounter = 0;
}

void NsfMapper::GetMemoryRanges(MemoryRanges& ranges)
{
	BaseMapper::GetMemoryRanges(ranges);
	
	//Allows us to override the PPU's range (0x3F00 - 0x3F1F)
	ranges.SetAllowOverride();
	ranges.AddHandler(MemoryOperation::Read, 0x3F00, 0x3F1F);
	ranges.AddHandler(MemoryOperation::Write, 0x3F00, 0x3F1F);
}

uint32_t NsfMapper::GetIrqReloadValue()
{
	switch(_console->GetRegion()) {
		default:
		case ConsoleRegion::Ntsc: return (uint16_t)(_nsfHeader.PlaySpeedNtsc * (NesConstants::ClockRateNtsc / 1000000.0)); break;
		case ConsoleRegion::Pal: return (uint16_t)(_nsfHeader.PlaySpeedPal * (NesConstants::ClockRatePal / 1000000.0)); break;
		case ConsoleRegion::Dendy: return (uint16_t)(_nsfHeader.PlaySpeedPal * (NesConstants::ClockRateDendy / 1000000.0)); break;
	}
}

bool NsfMapper::HasBankSwitching()
{
	for(int i = 0; i < 8; i++) {
		if(_nsfHeader.BankSetup[i] != 0) {
			return true;
		}
	}
	return false;
}

void NsfMapper::TriggerIrq()
{
	_console->GetCpu()->SetIrqSource(IRQSource::External);
}

void NsfMapper::ClearIrq()
{
	_console->GetCpu()->ClearIrqSource(IRQSource::External);
}

void NsfMapper::ProcessCpuClock()
{
	BaseProcessCpuClock();

	if(_irqCounter > 0) {
		_irqCounter--;
		if(_irqCounter == 0) {
			_irqCounter = GetIrqReloadValue();
			TriggerIrq();
		}
	}

	if(_nsfHeader.SoundChips & NsfSoundChips::MMC5) {
		_mmc5Audio->Clock();
	}
	if(_nsfHeader.SoundChips & NsfSoundChips::VRC6) {
		_vrc6Audio->Clock();
	}
	if(_nsfHeader.SoundChips & NsfSoundChips::VRC7) {
		_vrc7Audio->Clock();
	}
	if(_nsfHeader.SoundChips & NsfSoundChips::Namco) {
		_namcoAudio->Clock();
	}
	if(_nsfHeader.SoundChips & NsfSoundChips::Sunsoft) {
		_sunsoftAudio->Clock();
	}
	if(_nsfHeader.SoundChips & NsfSoundChips::FDS) {
		_fdsAudio->Clock();
	}
}

uint8_t NsfMapper::ReadRegister(uint16_t addr)
{
	if((_nsfHeader.SoundChips & NsfSoundChips::FDS) && addr >= 0x4040 && addr <= 0x4092) {
		return _fdsAudio->ReadRegister(addr);
	} else if((_nsfHeader.SoundChips & NsfSoundChips::Namco) && addr >= 0x4800 && addr <= 0x4FFF) {
		return _namcoAudio->ReadRegister(addr);
	} else if(addr >= 0x3F00 && addr <= 0x3F1F) {
		return _nsfBios[addr & 0x1F];
	} else {
		switch(addr) {
			case 0x5205: return (_mmc5MultiplierValues[0] * _mmc5MultiplierValues[1]) & 0xFF;
			case 0x5206: return (_mmc5MultiplierValues[0] * _mmc5MultiplierValues[1]) >> 8;

			//Reset/irq vectors
			case 0xFFFC: return 0x00;
			case 0xFFFD: return 0x3F;
			case 0xFFFE: return 0x10;
			case 0xFFFF: return 0x3F;
		}
	}

	return _console->GetMemoryManager()->GetOpenBus();
}

void NsfMapper::WriteRegister(uint16_t addr, uint8_t value)
{
	if((_nsfHeader.SoundChips & NsfSoundChips::FDS) && addr >= 0x4040 && addr <= 0x4092) {
		_fdsAudio->WriteRegister(addr, value);
	} else if((_nsfHeader.SoundChips & NsfSoundChips::MMC5) && addr >= 0x5000 && addr <= 0x5015) {
		_mmc5Audio->WriteRegister(addr, value);
	} else if((_nsfHeader.SoundChips & NsfSoundChips::Namco) && ((addr >= 0x4800 && addr <= 0x4FFF) || (addr >= 0xF800 && addr <= 0xFFFF))) {
		_namcoAudio->WriteRegister(addr, value);

		//Technically we should call this, but doing so breaks some NSFs
		/*if(addr >= 0xF800 && _nsfHeader.SoundChips & NsfSoundChips::Sunsoft) {
		_sunsoftAudio.WriteRegister(addr, value);
		}*/
	} else if((_nsfHeader.SoundChips & NsfSoundChips::Sunsoft) && addr >= 0xC000 && addr <= 0xFFFF) {
		_sunsoftAudio->WriteRegister(addr, value);
	} else {
		switch(addr) {
			case 0x3F00:
				_irqCounter = GetIrqReloadValue();
				ClearIrq();
				break;

			//MMC5 multiplication
			case 0x5205: _mmc5MultiplierValues[0] = value; break;
			case 0x5206: _mmc5MultiplierValues[1] = value; break;

			case 0x5FF6:
				SetCpuMemoryMapping(0x6000, 0x6FFF, value, PrgMemoryType::PrgRom, MemoryAccessType::ReadWrite);
				break;

			case 0x5FF7:
				SetCpuMemoryMapping(0x7000, 0x7FFF, value, PrgMemoryType::PrgRom, MemoryAccessType::ReadWrite);
				break;

			case 0x5FF8: case 0x5FF9: case 0x5FFA: case 0x5FFB:
			case 0x5FFC: case 0x5FFD: case 0x5FFE: case 0x5FFF:
				SetCpuMemoryMapping(0x8000 + (addr & 0x07) * 0x1000, 0x8FFF + (addr & 0x07) * 0x1000, value, PrgMemoryType::PrgRom, (addr <= 0x5FFD && (_nsfHeader.SoundChips & NsfSoundChips::FDS)) ? MemoryAccessType::ReadWrite : MemoryAccessType::Read);
				break;

			case 0x9000: case 0x9001: case 0x9002: case 0x9003: case 0xA000: case 0xA001: case 0xA002: case 0xB000: case 0xB001: case 0xB002:
				_vrc6Audio->WriteRegister(addr, value);
				break;

			case 0x9010: case 0x9030:
				_vrc7Audio->WriteReg(addr, value);
				break;

		}
	}
}

void NsfMapper::SelectTrack(uint8_t trackNumber)
{
	if(trackNumber < _nsfHeader.TotalSongs) {
		_songNumber = trackNumber;

		//Need to change track while running
		//Some NSFs keep the interrupt flag on at all times, preventing us from triggering an IRQ to change tracks
		//Forcing the console to reset ensures changing tracks always works, even with a bad NSF file
		_emu->GetSystemActionManager()->Reset();
	}
}

uint8_t NsfMapper::GetCurrentTrack()
{
	return _songNumber;
}

NsfHeader NsfMapper::GetNsfHeader()
{
	return _nsfHeader;
}

AudioTrackInfo NsfMapper::GetAudioTrackInfo()
{
	AudioTrackInfo track = {};
	track.Artist = _nsfHeader.ArtistName;
	string copyright = _nsfHeader.CopyrightHolder;
	track.Comment = (copyright.size() > 0 ? copyright + " " : "") + _nsfHeader.RipperName;
	track.GameTitle = _nsfHeader.SongName;
	track.SongTitle = _nsfHeader.TrackNames.size() > _songNumber ? _nsfHeader.TrackNames[_songNumber] : "";
	track.Position = _console->GetPpuFrame().FrameCount / _console->GetFps();
	if(_nsfHeader.TrackLength[_songNumber] > 0) {
		track.Length = _nsfHeader.TrackLength[_songNumber] / 1000.0 + _nsfHeader.TrackFade[_songNumber] / 1000.0;
		track.FadeLength = _nsfHeader.TrackFade[_songNumber] / 1000.0;
	}
	track.TrackNumber = _songNumber + 1;
	track.TrackCount = _nsfHeader.TotalSongs;
	return track;
}

void NsfMapper::ProcessAudioPlayerAction(AudioPlayerActionParams p)
{
	int selectedTrack = _songNumber;
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
		selectedTrack = _nsfHeader.TotalSongs - 1;
	} else if(selectedTrack >= _nsfHeader.TotalSongs) {
		selectedTrack = 0;
	}

	SelectTrack(selectedTrack);
}

void NsfMapper::Serialize(Serializer& s)
{
	BaseMapper::Serialize(s);

	SV(_mmc5Audio);
	SV(_vrc6Audio);
	SV(_vrc7Audio);
	SV(_fdsAudio);
	SV(_namcoAudio);
	SV(_sunsoftAudio);

	SV(_irqCounter); SV(_mmc5MultiplierValues[0]); SV(_mmc5MultiplierValues[1]); SV(_hasBankSwitching); SV(_songNumber);
}