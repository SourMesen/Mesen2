#include "stdafx.h"
#include "NES/NesConsole.h"
#include "NES/NesControlManager.h"
#include "NES/MapperFactory.h"
#include "NES/APU/NesApu.h"
#include "NES/NesCpu.h"
#include "NES/BaseMapper.h"
#include "NES/NesSoundMixer.h"
#include "NES/NesMemoryManager.h"
#include "NES/DefaultNesPpu.h"
#include "NES/NsfPpu.h"
#include "NES/HdPacks/HdAudioDevice.h"
#include "NES/HdPacks/HdData.h"
#include "NES/HdPacks/HdNesPpu.h"
#include "NES/HdPacks/HdPackLoader.h"
#include "NES/HdPacks/HdVideoFilter.h"
#include "NES/NesDefaultVideoFilter.h"
#include "NES/NesNtscFilter.h"
#include "NES/BisqwitNtscFilter.h"
#include "NES/NesConstants.h"
#include "NES/Mappers/VsSystem/VsControlManager.h"
#include "NES/Mappers/NsfMapper.h"
#include "NES/Mappers/FDS/Fds.h"
#include "Shared/Emulator.h"
#include "Shared/CheatManager.h"
#include "Netplay/GameClient.h"
#include "Shared/Movies/MovieManager.h"
#include "Shared/BaseControlManager.h"
#include "Shared/Interfaces/IBattery.h"
#include "Shared/EmuSettings.h"
#include "Debugger/DebugTypes.h"
#include "Utilities/Serializer.h"

NesConsole::NesConsole(Emulator* emu)
{
	_emu = emu;
}

NesConsole::~NesConsole()
{
}

Emulator* NesConsole::GetEmulator()
{
	return _emu;
}

NesConfig& NesConsole::GetNesConfig()
{
	return _emu->GetSettings()->GetNesConfig();
}

void NesConsole::ProcessCpuClock() 
{
	_mapper->ProcessCpuClock();
	_apu->ProcessCpuClock();
}

NesConsole* NesConsole::GetVsMainConsole()
{
	return _vsMainConsole;
}

NesConsole* NesConsole::GetVsSubConsole()
{
	return _vsSubConsole.get();
}

bool NesConsole::IsVsMainConsole()
{
	return _vsMainConsole == nullptr;
}

void NesConsole::Serialize(Serializer& s)
{
	_apu->EndFrame();

	s.Stream(_cpu.get());
	s.Stream(_ppu.get());
	s.Stream(_memoryManager.get());
	s.Stream(_apu.get());
	s.Stream(_controlManager.get());
	s.Stream(_mapper.get());
	s.Stream(_mixer.get());

	if(_hdAudioDevice) {
		//For HD packs, save the state of the bgm playback
		s.Stream(_hdAudioDevice.get());
	}

	if(_vsSubConsole) {
		//For VS Dualsystem, the sub console's savestate is appended to the end of the file
		s.Stream(_vsSubConsole.get());
	}
}

void NesConsole::Stop()
{
	//TODO
}

void NesConsole::Reset()
{
	_memoryManager->Reset(true);
	_ppu->Reset();
	_apu->Reset(true);
	_cpu->Reset(true, ConsoleRegion::Ntsc);
	_controlManager->Reset(true);
	_mixer->Reset();
	if(_vsSubConsole) {
		_vsSubConsole->Reset();
	}
}

void NesConsole::OnBeforeRun()
{
	//TODO
}

LoadRomResult NesConsole::LoadRom(VirtualFile& romFile)
{
	RomData romData;

	LoadHdPack(romFile);

	LoadRomResult result = LoadRomResult::UnknownType;
	unique_ptr<BaseMapper> mapper = MapperFactory::InitializeFromFile(this, romFile, romData, result);
	if(mapper) {
		if(!_vsMainConsole && romData.Info.VsType == VsSystemType::VsDualSystem) {
			//Create 2nd console (sub) dualsystem games
			_vsSubConsole.reset(new NesConsole(_emu));
			_vsSubConsole->_vsMainConsole = this;
			result = _vsSubConsole->LoadRom(romFile);
			if(result != LoadRomResult::Success) {
				return result;
			}
		}

		_mapper.swap(mapper);
		_mixer.reset(new NesSoundMixer(this));
		_memoryManager.reset(new NesMemoryManager(this));
		_cpu.reset(new NesCpu(this));
		_apu.reset(new NesApu(this));

		if(romData.Info.System == GameSystem::VsSystem) {
			_controlManager.reset(new VsControlManager(this));
		} else {
			_controlManager.reset(new NesControlManager(this));
		}

		/*if(!isDifferentGame && forPowerCycle) {
			_mapper->CopyPrgChrRom(previousMapper);
		}*/

		/*
		//Temporarely disable battery saves to prevent battery files from being created for the wrong game (for Battle Box & Turbo File)
		_batteryManager->SetSaveEnabled(false);
		*/
		uint32_t pollCounter = 0;
		/*if(_controlManager && !isDifferentGame) {
			//When power cycling, poll counter must be preserved to allow movies to playback properly
			pollCounter = _controlManager->GetPollCounter();
		}
		*/

		//Re-enable battery saves
		/*_batteryManager->SetSaveEnabled(true);
		*/
		if(_hdData && (!_hdData->Tiles.empty() || !_hdData->Backgrounds.empty())) {
			_ppu.reset(new HdNesPpu(this, _hdData.get()));
		} else if(dynamic_cast<NsfMapper*>(_mapper.get())) {
			//Disable most of the PPU for NSFs
			_ppu.reset(new NsfPpu(this));
		} else {
			_ppu.reset(new DefaultNesPpu(this));
		}

		_controlManager->SetPollCounter(pollCounter);
		_controlManager->UpdateControlDevices();

		_mapper->InitSpecificMapper(romData);

		_memoryManager->SetMapper(_mapper.get());
		_memoryManager->RegisterIODevice(_ppu.get());
		_memoryManager->RegisterIODevice(_apu.get());
		_memoryManager->RegisterIODevice(_controlManager.get());
		_memoryManager->RegisterIODevice(_mapper.get());

		if(_hdData && (!_hdData->BgmFilesById.empty() || !_hdData->SfxFilesById.empty())) {
			_hdAudioDevice.reset(new HdAudioDevice(_emu, _hdData.get()));
			_memoryManager->RegisterIODevice(_hdAudioDevice.get());
		} else {
			_hdAudioDevice.reset();
		}

		UpdateRegion();

		_mixer->Reset();
		
		_ppu->Reset();
		_apu->Reset(false);
		_memoryManager->Reset(false);
		_controlManager->Reset(false);
		_cpu->Reset(false, _region);
	}
    return result;
}

void NesConsole::LoadHdPack(VirtualFile& romFile)
{
	_hdData.reset();
	if(GetNesConfig().EnableHdPacks) {
		_hdData.reset(new HdPackData());
		if(!HdPackLoader::LoadHdNesPack(romFile, *_hdData.get())) {
			_hdData.reset();
		} else {
			auto result = _hdData->PatchesByHash.find(romFile.GetSha1Hash());
			if(result != _hdData->PatchesByHash.end()) {
				VirtualFile patchFile = result->second;
				romFile.ApplyPatch(patchFile);
			}
		}
	}
}

void NesConsole::UpdateRegion()
{
	ConsoleRegion region = GetNesConfig().Region;
	if(region == ConsoleRegion::Auto) {
		switch(_mapper->GetRomInfo().System) {
			case GameSystem::NesPal: region = ConsoleRegion::Pal; break;
			case GameSystem::Dendy: region = ConsoleRegion::Dendy; break;
			default: region = ConsoleRegion::Ntsc; break;
		}
	}
	if(_region != region) {
		_region = region;

		_cpu->SetMasterClockDivider(_region);
		_mapper->SetRegion(_region);
		_ppu->SetRegion(_region);
		_apu->SetRegion(_region);
		_mixer->SetRegion(_region);
	}
}

void NesConsole::Init()
{
	//Reset();
}

void NesConsole::RunFrame()
{
	uint32_t frame = _ppu->GetFrameCount();
	while(frame == _ppu->GetFrameCount()) {
		_cpu->Exec();
		if(_vsSubConsole) {
			RunVsSubConsole();
		}
	}
}

void NesConsole::RunVsSubConsole()
{
	int64_t cycleGap;
	while(true) {
		//Run the sub console until it catches up to the main CPU
		cycleGap = (int64_t)(_cpu->GetCycleCount() - _vsSubConsole->_cpu->GetCycleCount());
		if(cycleGap > 5 || _ppu->GetFrameCount() > _vsSubConsole->_ppu->GetFrameCount()) {
			_vsSubConsole->_cpu->Exec();
		} else {
			break;
		}
	}
}

BaseControlManager* NesConsole::GetControlManager()
{
	return _controlManager.get();
}

double NesConsole::GetFps()
{
	UpdateRegion();
	if(_region == ConsoleRegion::Ntsc) {
		return _emu->GetSettings()->GetVideoConfig().IntegerFpsMode ? 60.0 : 60.0988118623484;
	} else {
		return _emu->GetSettings()->GetVideoConfig().IntegerFpsMode ? 50.0 : 50.0069789081886;
	}
}

PpuFrameInfo NesConsole::GetPpuFrame()
{
	PpuFrameInfo frame;
	frame.FrameBuffer = (uint8_t*)_ppu->GetScreenBuffer(false);
	frame.Width = NesConstants::ScreenWidth;
	frame.Height = NesConstants::ScreenHeight;
	frame.FrameCount = _ppu->GetFrameCount();
	frame.FirstScanline = -1;
	frame.ScanlineCount = _ppu->GetScanlineCount();
	frame.CycleCount = 341;
	return frame;
}

ConsoleType NesConsole::GetConsoleType()
{
	return ConsoleType::Nes;
}

ConsoleRegion NesConsole::GetRegion()
{
	return _region;
}

vector<CpuType> NesConsole::GetCpuTypes()
{
	return { CpuType::Nes };
}

AddressInfo NesConsole::GetAbsoluteAddress(AddressInfo& relAddress)
{
	if(relAddress.Type == MemoryType::NesMemory) {
		return _mapper->GetAbsoluteAddress(relAddress.Address);
	} else {
		return _mapper->GetPpuAbsoluteAddress(relAddress.Address);
	}
}

AddressInfo NesConsole::GetRelativeAddress(AddressInfo& absAddress, CpuType cpuType)
{
	AddressInfo addr;
	addr.Address = _mapper->GetRelativeAddress(absAddress);
	addr.Type = MemoryType::NesMemory;
	return addr;
}

void NesConsole::GetConsoleState(BaseState& baseState, ConsoleType consoleType)
{
	NesState& state = (NesState&)baseState;

	state.ClockRate = GetMasterClockRate();
	state.Snes = _cpu->GetState();
	_ppu->GetState(state.Ppu);
	state.Cartridge = _mapper->GetState();
	state.Apu = _apu->GetState();
}

uint64_t NesConsole::GetMasterClock()
{
	return _cpu->GetCycleCount();
}

uint32_t NesConsole::GetMasterClockRate()
{
	return NesConstants::GetClockRate(_region);
}

void NesConsole::SaveBattery()
{
	if(_mapper) {
		_mapper->SaveBattery();
	}
	
	if(_controlManager) {
		_controlManager->SaveBattery();
	}
}

ShortcutState NesConsole::IsShortcutAllowed(EmulatorShortcut shortcut, uint32_t shortcutParam)
{
	bool isRunning = _emu->IsRunning();
	bool isNetplayClient = _emu->GetGameClient()->Connected();
	bool isMoviePlaying = _emu->GetMovieManager()->Playing();
	RomFormat romFormat = GetRomFormat();
	
	switch(shortcut) {
		case EmulatorShortcut::FdsEjectDisk:
		case EmulatorShortcut::FdsInsertNextDisk:
		case EmulatorShortcut::FdsSwitchDiskSide:
			return (ShortcutState)(isRunning && !isNetplayClient && !isMoviePlaying && romFormat == RomFormat::Fds);

		case EmulatorShortcut::FdsInsertDiskNumber:
			if(isRunning && !isNetplayClient && !isMoviePlaying && romFormat == RomFormat::Fds) {
				Fds* fds = dynamic_cast<Fds*>(_mapper.get());
				return (ShortcutState)(fds && shortcutParam < fds->GetSideCount());
			}
			return ShortcutState::Disabled;

		case EmulatorShortcut::VsInsertCoin1:
		case EmulatorShortcut::VsInsertCoin2:
		case EmulatorShortcut::VsServiceButton:
			return (ShortcutState)(isRunning && !isNetplayClient && !isMoviePlaying && (romFormat == RomFormat::VsSystem || romFormat == RomFormat::VsDualSystem));

		case EmulatorShortcut::VsInsertCoin3:
		case EmulatorShortcut::VsInsertCoin4:
		case EmulatorShortcut::VsServiceButton2:
			return (ShortcutState)(isRunning && !isNetplayClient && !isMoviePlaying && romFormat == RomFormat::VsDualSystem);
	}

	return ShortcutState::Default;
}

BaseVideoFilter* NesConsole::GetVideoFilter()
{
	if(_hdData) {
		return new HdVideoFilter(_emu, _hdData.get());
	} else if(GetRomFormat() == RomFormat::Nsf) {
		return new NesDefaultVideoFilter(_emu);
	} else {
		VideoFilterType filterType = _emu->GetSettings()->GetVideoConfig().VideoFilter;

		switch(filterType) {
			case VideoFilterType::NtscBlargg: return new NesNtscFilter(_emu);
			case VideoFilterType::NtscBisqwit: return new BisqwitNtscFilter(_emu);
			default: return new NesDefaultVideoFilter(_emu);
		}
	}
}

RomFormat NesConsole::GetRomFormat()
{
	return _mapper->GetRomInfo().Format;
}

AudioTrackInfo NesConsole::GetAudioTrackInfo()
{
	NsfMapper* nsfMapper = dynamic_cast<NsfMapper*>(_mapper.get());
	if(nsfMapper) {
		return nsfMapper->GetAudioTrackInfo();
	}
	return {};
}

void NesConsole::ProcessAudioPlayerAction(AudioPlayerActionParams p)
{
	NsfMapper* nsfMapper = dynamic_cast<NsfMapper*>(_mapper.get());
	if(nsfMapper) {
		return nsfMapper->ProcessAudioPlayerAction(p);
	}
}

uint8_t NesConsole::DebugRead(uint16_t addr)
{
	return _memoryManager->DebugRead(addr);
}

void NesConsole::DebugWrite(uint16_t addr, uint8_t value)
{
	_memoryManager->DebugWrite(addr, value);
}

uint8_t NesConsole::DebugReadVram(uint16_t addr)
{
	if(addr >= 0x3F00) {
		return _ppu->ReadPaletteRam(addr);
	} else {
		return _mapper->DebugReadVram(addr);
	}
}

void NesConsole::DebugWriteVram(uint16_t addr, uint8_t value)
{
	if(addr >= 0x3F00) {
		_ppu->WritePaletteRam(addr, value);
	} else {
		_mapper->DebugWriteVram(addr, value);
	}
}

void NesConsole::ProcessCheatCode(InternalCheatCode& code, uint32_t addr, uint8_t& value)
{
	if(code.Type == CheatType::NesGameGenie && addr >= 0xC020) {
		if(GetNesConfig().DisableGameGenieBusConflicts) {
			return;
		}

		AddressInfo absAddr = _mapper->GetAbsoluteAddress(addr - 0x8000);
		if(absAddr.Address >= 0) {
			//Game Genie causes a bus conflict when the cartridge maps anything below $8000
			//Only processed when addr >= $C020 because the mapper implementation never maps anything below $4020
			value &= _mapper->DebugReadRAM(addr - 0x8000);
		}
	}
}