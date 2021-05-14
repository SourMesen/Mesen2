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
#include "NES/NesConstants.h"
#include "NES/Mappers/VsSystem/VsControlManager.h"
#include "NES/Mappers/NsfMapper.h"
#include "Shared/Emulator.h"
#include "Shared/Interfaces/IControlManager.h"
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

	//TODO
	/*if(_hdAudioDevice) {
		_hdAudioDevice->LoadSnapshot(&loadStream, stateVersion);
	} else {
		Snapshotable::SkipBlock(&loadStream);
	}*/

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
			LoadRomResult result = _vsSubConsole->LoadRom(romFile);
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
	int frame = _ppu->GetFrameCount();
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

shared_ptr<IControlManager> NesConsole::GetControlManager()
{
	return std::dynamic_pointer_cast<IControlManager>(_controlManager);
}

double NesConsole::GetFrameDelay()
{
	UpdateRegion();

	switch(_region) {
		default:
		case ConsoleRegion::Ntsc: return _emu->GetSettings()->GetVideoConfig().IntegerFpsMode ? 16.6666666666666666667 : 16.63926405550947;
		
		case ConsoleRegion::Dendy:
		case ConsoleRegion::Pal:
			return _emu->GetSettings()->GetVideoConfig().IntegerFpsMode ? 20 : 19.99720882631146;
	}
}

double NesConsole::GetFps()
{
	if(_region == ConsoleRegion::Ntsc) {
		return _emu->GetSettings()->GetVideoConfig().IntegerFpsMode ? 60.0 : 60.0988118623484;
	} else {
		return _emu->GetSettings()->GetVideoConfig().IntegerFpsMode ? 50.0 : 50.00697796826829;
	}
}

void NesConsole::RunSingleFrame()
{
	//TODO
}

PpuFrameInfo NesConsole::GetPpuFrame()
{
	PpuFrameInfo frame;
	frame.FrameBuffer = (uint8_t*)_ppu->GetScreenBuffer(false);
	frame.Width = NesConstants::ScreenWidth;
	frame.Height = NesConstants::ScreenHeight;
	frame.FrameCount = _ppu->GetFrameCount();
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

AddressInfo NesConsole::GetAbsoluteAddress(AddressInfo relAddress)
{
	return _mapper->GetAbsoluteAddress(relAddress.Address);
}

AddressInfo NesConsole::GetRelativeAddress(AddressInfo absAddress, CpuType cpuType)
{
	AddressInfo addr;
	addr.Address = _mapper->GetRelativeAddress(absAddress);
	addr.Type = SnesMemoryType::NesMemory;
	return addr;
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
		//TODO
		/*shared_ptr<IBattery> device = std::dynamic_pointer_cast<IBattery>(_controlManager->GetControlDevice(BaseControlDevice::ExpDevicePort));
		if(device) {
			device->SaveBattery();
		}*/
	}
}

BaseVideoFilter* NesConsole::GetVideoFilter()
{
	if(_hdData) {
		return new HdVideoFilter(_emu, _hdData.get());
	} else {
		VideoFilterType filterType = _emu->GetSettings()->GetVideoConfig().VideoFilter;
		if(filterType == VideoFilterType::NTSC && GetRomFormat() != RomFormat::Nsf) {
			return new NesNtscFilter(_emu);
		} else {
			return new NesDefaultVideoFilter(_emu);
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

