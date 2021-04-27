#include "stdafx.h"
#include "NES/NesConsole.h"
#include "NES/NesControlManager.h"
#include "NES/MapperFactory.h"
#include "NES/APU/NesApu.h"
#include "NES/NesCpu.h"
#include "NES/BaseMapper.h"
#include "NES/NesSoundMixer.h"
#include "NES/NesMemoryManager.h"
#include "NES/NesPpu.h"
#include "NES/NesDefaultVideoFilter.h"
#include "NES/NesNtscFilter.h"
#include "NES/Mappers/VsSystem/VsControlManager.h"
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
	_cpu->Reset(true, NesModel::NTSC);
	_controlManager->Reset(true);
	if(_vsSubConsole) {
		_vsSubConsole->Reset();
	}
}

void NesConsole::OnBeforeRun()
{
	//TODO
}

bool NesConsole::LoadRom(VirtualFile& romFile)
{
	RomData romData;

	shared_ptr<BaseMapper> mapper = MapperFactory::InitializeFromFile(this, romFile, romData);
	if(mapper) {
		shared_ptr<BaseMapper> previousMapper = _mapper;
		_mapper = mapper;
		_mixer.reset(new NesSoundMixer(this));
		_memoryManager.reset(new NesMemoryManager(this));
		_cpu.reset(new NesCpu(this));
		_apu.reset(new NesApu(this));

		if(romData.Info.System == GameSystem::VsSystem) {
			_controlManager.reset(new VsControlManager(this));
		} else {
			_controlManager.reset(new NesControlManager(this));
		}

		_mapper->SetConsole(this);
		_mapper->Initialize(romData);

		/*if(!isDifferentGame && forPowerCycle) {
			_mapper->CopyPrgChrRom(previousMapper);
		}*/

		if(!_vsMainConsole && romData.Info.VsType == VsSystemType::VsDualSystem) {
			_vsSubConsole.reset(new NesConsole(_emu));
			_vsSubConsole->_vsMainConsole = this;
			if(!_vsSubConsole->LoadRom(romFile)) {
				return false;
			}
		}

		/*switch(romInfo.System) {
			case GameSystem::FDS:
				_settings->SetPpuModel(PpuModel::Ppu2C02);
				break;

			case GameSystem::VsSystem:
				_settings->SetPpuModel(romInfo.VsPpuModel);
				break;

			default:
				_settings->SetPpuModel(PpuModel::Ppu2C02);
		}
		

		//Temporarely disable battery saves to prevent battery files from being created for the wrong game (for Battle Box & Turbo File)
		_batteryManager->SetSaveEnabled(false);
		*/
		uint32_t pollCounter = 0;
		/*if(_controlManager && !isDifferentGame) {
			//When power cycling, poll counter must be preserved to allow movies to playback properly
			pollCounter = _controlManager->GetPollCounter();
		}
		*/

		_controlManager->SetPollCounter(pollCounter);
		_controlManager->UpdateControlDevices();

		//Re-enable battery saves
		/*_batteryManager->SetSaveEnabled(true);

		if(_hdData && (!_hdData->Tiles.empty() || !_hdData->Backgrounds.empty())) {
			_ppu.reset(new HdPpu(shared_from_this(), _hdData.get()));
		} else if(std::dynamic_pointer_cast<NsfMapper>(_mapper)) {
			//Disable most of the PPU for NSFs
			_ppu.reset(new NsfPpu(shared_from_this()));
		} else {*/
			_ppu.reset(new NesPpu(this));
		//}

		_memoryManager->SetMapper(_mapper.get());
		_memoryManager->RegisterIODevice(_ppu.get());
		_memoryManager->RegisterIODevice(_apu.get());
		_memoryManager->RegisterIODevice(_controlManager.get());
		_memoryManager->RegisterIODevice(_mapper.get());

		NesModel model = NesModel::NTSC;
		_cpu->SetMasterClockDivider(model);
		_mapper->SetNesModel(model);
		_ppu->SetNesModel(model);
		_apu->SetNesModel(model);
		_mixer->Reset();
		
		_cpu->Reset(false, model);
		_ppu->Reset();
		_mapper->Reset(false);
		_apu->Reset(false);
		_memoryManager->Reset(false);
		_controlManager->Reset(false);

		return true;
	}
    return false;
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
	_emu->ProcessEndOfFrame();
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
	//TODO
	return 16.6666667;
}

double NesConsole::GetFps()
{
	//TODO
	return 60;
}

void NesConsole::RunSingleFrame()
{
	//TODO
}

PpuFrameInfo NesConsole::GetPpuFrame()
{
	PpuFrameInfo frame;
	frame.FrameBuffer = (uint8_t*)_ppu->GetScreenBuffer(false);
	frame.Width = NesPpu::ScreenWidth;
	frame.Height = NesPpu::ScreenHeight;
	frame.FrameCount = _ppu->GetFrameCount();
	return frame;
}

ConsoleType NesConsole::GetConsoleType()
{
	return ConsoleType::Nes;
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
	return NesCpu::ClockRateNtsc;
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
	VideoFilterType filterType = _emu->GetSettings()->GetVideoConfig().VideoFilter;
	if(filterType == VideoFilterType::NTSC) {
		return new NesNtscFilter(_emu);
	} else {
		return new NesDefaultVideoFilter(_emu);
	}
}
