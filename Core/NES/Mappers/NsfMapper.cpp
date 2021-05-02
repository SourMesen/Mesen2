#include "stdafx.h"
#include <random>
#include "NES/Mappers/NsfMapper.h"
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
	//TODO
	/*_console->GetSettings()->DisableOverclocking(false);
	_console->GetSettings()->ClearFlags(EmulationFlags::NsfPlayerEnabled);*/
}

void NsfMapper::InitMapper()
{
	_settings = _console->GetEmulator()->GetSettings();
	//TODO
	/*_console->GetSettings()->DisableOverclocking(true);
	_console->GetSettings()->ClearFlags(EmulationFlags::Paused);
	_console->GetSettings()->SetFlags(EmulationFlags::NsfPlayerEnabled);*/

	SetCpuMemoryMapping(0x3F00, 0x3FFF, PrgMemoryType::WorkRam, 0x2000, MemoryAccessType::Read);
	memcpy(GetWorkRam() + 0x2000, _nsfBios, 0x100);

	//Clear all register settings
	RemoveRegisterRange(0x0000, 0xFFFF, MemoryOperation::Any);

	AddRegisterRange(0x3E00, 0x3E13, MemoryOperation::Read);
	AddRegisterRange(0x3E10, 0x3E13, MemoryOperation::Write);

	//NSF registers
	AddRegisterRange(0x5FF6, 0x5FFF, MemoryOperation::Write);
}

void NsfMapper::InitMapper(RomData& romData)
{
	_nsfHeader = romData.Info.NsfInfo;

	_hasBankSwitching = HasBankSwitching();
	if(!_hasBankSwitching) {
		//Update bank config to allow BIOS to select the right banks on init
		int8_t startBank = (_nsfHeader.LoadAddress / 0x1000);
		for(int32_t i = 0; i < (int32_t)GetPRGPageCount(); i++) {
			if((startBank + i) > 0x0F) {
				break;
			}
			if(startBank + i - 8 >= 0) {
				_nsfHeader.BankSetup[startBank + i - 8] = i;
			}
		}
	}

	_songNumber = _nsfHeader.StartingSong - 1;
	_ntscSpeed = (uint16_t)(_nsfHeader.PlaySpeedNtsc * (NesConstants::ClockRateNtsc / 1000000.0));
	_palSpeed = (uint16_t)(_nsfHeader.PlaySpeedPal * (NesConstants::ClockRatePal / 1000000.0));
	_dendySpeed = (uint16_t)(_nsfHeader.PlaySpeedPal * (NesConstants::ClockRateDendy / 1000000.0));

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
}

void NsfMapper::Reset(bool softReset)
{
	if(!softReset) {
		_songNumber = _nsfHeader.StartingSong - 1;
	}

	_needInit = true;
	_irqEnabled = false;
	_irqCounter = 0;
	
	switch(_emu->GetRegion()) {
		default:
		case ConsoleRegion::Ntsc: _irqReloadValue = _ntscSpeed; break;
		case ConsoleRegion::Pal: _irqReloadValue = _palSpeed; break;
		case ConsoleRegion::Dendy: _irqReloadValue = _dendySpeed; break;
	}

	_irqStatus = NsfIrqType::None;

	_allowSilenceDetection = false;
	_trackEndCounter = -1;
	_trackEnded = false;
	_trackFadeCounter = -1;

	_mmc5Audio.reset(new Mmc5Audio(_console));
	_vrc6Audio.reset(new Vrc6Audio(_console));
	_vrc7Audio.reset(new Vrc7Audio(_console));
	_fdsAudio.reset(new FdsAudio(_console));
	_namcoAudio.reset(new Namco163Audio(_console));
	_sunsoftAudio.reset(new Sunsoft5bAudio(_console));

	InternalSelectTrack(_songNumber, false);

	//Reset/IRQ vector
	AddRegisterRange(0xFFFC, 0xFFFF, MemoryOperation::Read);
}

void NsfMapper::GetMemoryRanges(MemoryRanges& ranges)
{
	BaseMapper::GetMemoryRanges(ranges);
	
	//Allows us to override the PPU's range (0x3E00 - 0x3FFF)
	ranges.SetAllowOverride();
	ranges.AddHandler(MemoryOperation::Read, 0x3E00, 0x3FFF);
	ranges.AddHandler(MemoryOperation::Write, 0x3E00, 0x3FFF);
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

void NsfMapper::TriggerIrq(NsfIrqType type)
{
	if(type == NsfIrqType::Init) {
		_trackEnded = false;
	}

	_debugIrqStatus = type;
	_irqStatus = type;
	_console->GetCpu()->SetIrqSource(IRQSource::External);
}

void NsfMapper::ClearIrq()
{
	_irqStatus = NsfIrqType::None;
	_console->GetCpu()->ClearIrqSource(IRQSource::External);
}

void NsfMapper::ClockLengthAndFadeCounters()
{
	if(_trackEndCounter > 0) {
		_trackEndCounter--;
		if(_trackEndCounter == 0) {
			_trackEnded = true;
		}
	}

	//TODO
	/*if((_trackEndCounter < 0 || _allowSilenceDetection) && _console->GetSettings()->GetNsfAutoDetectSilenceDelay() > 0) {
		//No track length specified
		if(_console->GetSoundMixer()->GetMuteFrameCount() * SoundMixer::CycleLength > _silenceDetectDelay) {
			//Auto detect end of track after AutoDetectSilenceDelay (in ms) has gone by without sound
			_trackEnded = true;
			_console->GetSoundMixer()->ResetMuteFrameCount();
		}
	}*/

	if(_trackEnded) {
		if(_trackFadeCounter > 0) {
			if(_fadeLength != 0) {
				double fadeRatio = (double)_trackFadeCounter / (double)_fadeLength * 1.2;
				_console->GetSoundMixer()->SetFadeRatio(std::max(0.0, fadeRatio - 0.2));
			}
			_trackFadeCounter--;
		}

		if(_trackFadeCounter <= 0) {
			SelectNextTrack();
		}
	}
}

void NsfMapper::SelectNextTrack()
{
	if(!_settings->GetAudioPlayerConfig().Repeat) {
		if(_settings->GetAudioPlayerConfig().Shuffle) {
			std::random_device rd;
			std::mt19937 mt(rd());
			std::uniform_int_distribution<> dist(0, _nsfHeader.TotalSongs - 1);
			_songNumber = dist(mt);
		} else {
			_songNumber = (_songNumber + 1) % _nsfHeader.TotalSongs;
		}
	}
	InternalSelectTrack(_songNumber);
	_trackEnded = false;
}

void NsfMapper::ProcessCpuClock()
{
	//TODO DEBUGGER
	/*if(_console->IsDebuggerAttached()) {
		shared_ptr<Debugger> debugger = _console->GetDebugger(false);
		if(debugger) {
			uint16_t programCounter = _console->GetCpu()->GetPC();
			if(_debugIrqStatus == NsfIrqType::Init && programCounter == _nsfHeader.InitAddress) {
				_debugIrqStatus = NsfIrqType::None;
				if(debugger->CheckFlag(DebuggerFlags::BreakOnInit)) {
					debugger->Step(1);
				}
			} else if(_debugIrqStatus == NsfIrqType::Play && programCounter == _nsfHeader.PlayAddress) {
				_debugIrqStatus = NsfIrqType::None;
				if(debugger->CheckFlag(DebuggerFlags::BreakOnPlay)) {
					debugger->Step(1);
				}
			}
		}
	}
	*/
	_console->GetCpu()->SetIrqMask((uint8_t)IRQSource::External);

	if(_needInit) {
		TriggerIrq(NsfIrqType::Init);
		_needInit = false;
	}

	if(_irqEnabled) {
		_irqCounter--;
		if(_irqCounter == 0) {
			_irqCounter = _irqReloadValue;
			TriggerIrq(NsfIrqType::Play);
		}
	}

	ClockLengthAndFadeCounters();

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
	} else {
		switch(addr) {
			case 0x3E00: return _nsfHeader.InitAddress & 0xFF;
			case 0x3E01: return (_nsfHeader.InitAddress >> 8) & 0xFF;
			case 0x3E02: return _nsfHeader.PlayAddress & 0xFF;
			case 0x3E03: return (_nsfHeader.PlayAddress >> 8) & 0xFF;
			
			case 0x3E04:
			case 0x3E05:
				switch(_console->GetRegion()) {
					default:
					case ConsoleRegion::Ntsc: return _ntscSpeed & 0xFF;
					case ConsoleRegion::Pal: return _palSpeed & 0xFF;
					case ConsoleRegion::Dendy: return _dendySpeed & 0xFF;
				}
				break;

			case 0x3E06:
			case 0x3E07:
				switch(_console->GetRegion()) {
					default:
					case ConsoleRegion::Ntsc: return (_ntscSpeed >> 8) & 0xFF;
					case ConsoleRegion::Pal: return (_palSpeed >> 8) & 0xFF;
					case ConsoleRegion::Dendy: return (_dendySpeed >> 8) & 0xFF;
				}
				break;

			case 0x3E08: case 0x3E09: case 0x3E0A: case 0x3E0B:
			case 0x3E0C: case 0x3E0D: case 0x3E0E: case 0x3E0F:
				return _nsfHeader.BankSetup[addr & 0x07];

			case 0x3E10: return _songNumber;

			case 0x3E11: return _console->GetRegion() == ConsoleRegion::Pal ? 0x01 : 0x00;

			case 0x3E12: {
				NsfIrqType result = _irqStatus;
				ClearIrq();
				return (uint8_t)result;
			}

			case 0x3E13: return _nsfHeader.SoundChips & 0x3F;

			case 0x5205: return (_mmc5MultiplierValues[0] * _mmc5MultiplierValues[1]) & 0xFF;
			case 0x5206: return (_mmc5MultiplierValues[0] * _mmc5MultiplierValues[1]) >> 8;

			//Reset/irq vectors
			case 0xFFFC: case 0xFFFD: case 0xFFFE: case 0xFFFF:
				return _nsfBios[addr & 0xFF];
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
			/*case 0x3E10: _irqReloadValue = (_irqReloadValue & 0xFF00) | value; break;
			case 0x3E11: _irqReloadValue = (_irqReloadValue & 0xFF) | (value << 8); break;*/

			case 0x3E12:
				_irqCounter = _irqReloadValue * 5;
				_irqEnabled = (value > 0);
				break;

			case 0x3E13:
				_irqCounter = _irqReloadValue;
				break;

			//MMC5 multiplication
			case 0x5205: _mmc5MultiplierValues[0] = value; break;
			case 0x5206: _mmc5MultiplierValues[1] = value; break;

			case 0x5FF6: case 0x5FF7: {
				uint16_t addrOffset = (addr == 0x5FF7 ? 0x1000 : 0x0000);
				if(value == 0xFF || value == 0xFE) {
					if(!_hasBankSwitching && _nsfHeader.LoadAddress < 0x7000) {
						//Load address = 0x6000, put bank 0 at $6000
						SetCpuMemoryMapping(0x6000 + addrOffset, 0x6FFF + addrOffset, value & 0x01, PrgMemoryType::PrgRom, MemoryAccessType::ReadWrite);
					} else if(!_hasBankSwitching && addrOffset == 0x1000 && _nsfHeader.LoadAddress < 0x8000) {
						//Load address = 0x7000, put bank 0 at $7000
						SetCpuMemoryMapping(0x6000 + addrOffset, 0x6FFF + addrOffset, 0, PrgMemoryType::PrgRom, MemoryAccessType::ReadWrite);
					} else {
						//Set ram at $6000/7000 (default behavior)
						SetCpuMemoryMapping(0x6000 + addrOffset, 0x6FFF + addrOffset, value & 0x01, PrgMemoryType::WorkRam);
					}
				} else {
					SetCpuMemoryMapping(0x6000 + addrOffset, 0x6FFF + addrOffset, value, PrgMemoryType::PrgRom, MemoryAccessType::ReadWrite);
				}
				break;
			}

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

uint32_t NsfMapper::GetClockRate()
{
	return ((_nsfHeader.Flags & 0x01) ? NesConstants::ClockRatePal : NesConstants::ClockRateNtsc);
}

void NsfMapper::InternalSelectTrack(uint8_t trackNumber, bool requestReset)
{
	_songNumber = trackNumber;
	if(requestReset) {
		//Need to change track while running
		//Some NSFs keep the interrupt flag on at all times, preventing us from triggering an IRQ to change tracks
		//Forcing the console to reset ensures changing tracks always works, even with a bad NSF file
		_emu->GetSystemActionManager()->Reset();
	} else {
		//Selecting tracking after a reset
		_console->GetSoundMixer()->SetFadeRatio(1.0);
		NesConfig& cfg = _console->GetNesConfig();

		//Set track length/fade counters (NSFe)
		if(_nsfHeader.TrackLength[trackNumber] >= 0) {
			_trackEndCounter = (int32_t)((double)_nsfHeader.TrackLength[trackNumber] / 1000.0 * GetClockRate());
			_allowSilenceDetection = false;
		} else if(_nsfHeader.TotalSongs > 1) {
			//Only apply a maximum duration to multi-track NSFs
			//Single track NSFs will loop or restart after a portion of silence
			//Substract 1 sec from default track time to account for 1 sec default fade time
			if(cfg.NsfMoveToNextTrackAfterTime) {
				_trackEndCounter = (cfg.NsfMoveToNextTrackTime - 1) * GetClockRate();
				_allowSilenceDetection = true;
			} else {
				_trackEndCounter = 0;
				_allowSilenceDetection = false;
			}
		}
		if(_nsfHeader.TrackFade[trackNumber] >= 0) {
			_trackFadeCounter = (int32_t)((double)_nsfHeader.TrackFade[trackNumber] / 1000.0 * GetClockRate());
		} else {
			//Default to 1 sec fade if none is specified (negative number)
			_trackFadeCounter = GetClockRate();
		}

		if(cfg.NsfAutoDetectSilence) {
			_silenceDetectDelay = (uint32_t)((double)cfg.NsfAutoDetectSilenceDelay / 1000.0 * GetClockRate());
		} else {
			_silenceDetectDelay = 0;
		}

		_fadeLength = _trackFadeCounter;
		TriggerIrq(NsfIrqType::Init);
	}
}

void NsfMapper::SelectTrack(uint8_t trackNumber)
{
	if(trackNumber < _nsfHeader.TotalSongs) {
		InternalSelectTrack(trackNumber);
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

ConsoleFeatures NsfMapper::GetAvailableFeatures()
{
	return ConsoleFeatures::Nsf;
}

AudioTrackInfo NsfMapper::GetAudioTrackInfo()
{
	AudioTrackInfo track = {};
	track.Artist = _nsfHeader.ArtistName;
	track.Comment = _nsfHeader.RipperName;
	track.GameTitle = _nsfHeader.SongName;
	track.SongTitle = _nsfHeader.TrackNames.size() > _songNumber ? _nsfHeader.TrackNames[_songNumber] : "";
	track.Position = _console->GetPpuFrame().FrameCount / _console->GetFps();
	track.Length = _nsfHeader.TrackLength[_songNumber] / 1000 + _nsfHeader.TrackFade[_songNumber] / 1000;
	track.TrackNumber = _songNumber + 1;
	track.TrackCount = _nsfHeader.TotalSongs;
	return track;
}

void NsfMapper::ProcessAudioPlayerAction(AudioPlayerActionParams p)
{
	int selectedTrack = _songNumber;
	switch(p.Action) {
		case AudioPlayerAction::NextTrack: selectedTrack++; break;
		case AudioPlayerAction::PrevTrack: selectedTrack--; break;
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

	s.Stream(_mmc5Audio.get());
	s.Stream(_vrc6Audio.get());
	s.Stream(_vrc7Audio.get());
	s.Stream(_fdsAudio.get());
	s.Stream(_namcoAudio.get());
	s.Stream(_sunsoftAudio.get());

	s.Stream(
		_needInit, _irqEnabled, _irqReloadValue, _irqCounter, _irqStatus, _debugIrqStatus, _mmc5MultiplierValues[0], _mmc5MultiplierValues[1],
		_trackEndCounter, _trackFadeCounter, _fadeLength, _silenceDetectDelay, _trackEnded, _allowSilenceDetection, _hasBankSwitching, _ntscSpeed,
		_palSpeed, _dendySpeed, _songNumber
	);
}