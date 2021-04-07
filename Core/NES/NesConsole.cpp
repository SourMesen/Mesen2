#include "stdafx.h"
#include "NesConsole.h"
#include "Emulator.h"
#include "NesControlManager.h"
#include "IControlManager.h"
#include "MapperFactory.h"
#include "NesApu.h"
#include "NesCpu.h"
#include "BaseMapper.h"
#include "NesSoundMixer.h"
#include "NesMemoryManager.h"
#include "NesPpu.h"
#include "Utilities/Serializer.h"
#include "EmuSettings.h"
#include "DebugTypes.h"

NesConsole::NesConsole(Emulator* emu)
{
	_emu = emu;
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
	}

	if(_slave) {
		//For VS Dualsystem, the slave console's savestate is appended to the end of the file
		_slave->LoadState(loadStream, stateVersion);
	}*/
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
}

void NesConsole::OnBeforeRun()
{
	//TODO
}

bool NesConsole::LoadRom(VirtualFile& romFile, VirtualFile& patchFile)
{
	RomData romData;
	shared_ptr<BaseMapper> mapper = MapperFactory::InitializeFromFile(shared_from_this(), romFile, romData);
	if(mapper) {
		shared_ptr<BaseMapper> previousMapper = _mapper;
		_mapper = mapper;
		_mixer.reset(new NesSoundMixer(shared_from_this()));
		_memoryManager.reset(new NesMemoryManager(shared_from_this()));
		_cpu.reset(new NesCpu(shared_from_this()));
		_apu.reset(new NesApu(shared_from_this()));

		_mapper->SetConsole(shared_from_this());
		_mapper->Initialize(romData);

		NesRomInfo romInfo = _mapper->GetRomInfo();

		/*if(!isDifferentGame && forPowerCycle) {
			_mapper->CopyPrgChrRom(previousMapper);
		}

		if(_slave) {
			_slave->Release(false);
			_slave.reset();
		}

		if(!_master && romInfo.VsType == VsSystemType::VsDualSystem) {
			_slave.reset(new Console(shared_from_this()));
			_slave->Init();
			_slave->Initialize(romFile, patchFile);
		}*/

		/*switch(romInfo.System) {
			case GameSystem::FDS:
				_settings->SetPpuModel(PpuModel::Ppu2C02);
				_systemActionManager.reset(new FdsSystemActionManager(shared_from_this(), _mapper));
				break;

			case GameSystem::VsSystem:
				_settings->SetPpuModel(romInfo.VsPpuModel);
				_systemActionManager.reset(new VsSystemActionManager(shared_from_this()));
				break;

			default:
				_settings->SetPpuModel(PpuModel::Ppu2C02);
				_systemActionManager.reset(new SystemActionManager(shared_from_this())); break;
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
		if(romInfo.System == GameSystem::VsSystem) {
			//_controlManager.reset(new VsControlManager(shared_from_this(), _systemActionManager, _mapper->GetMapperControlDevice()));
		} else {
			_controlManager.reset(new NesControlManager(shared_from_this(), nullptr /* TODO _systemActionManager */, _mapper->GetMapperControlDevice()));
		}
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
			_ppu.reset(new NesPpu(shared_from_this()));
		//}

		_memoryManager->SetMapper(_mapper);
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

		return true;
	}
    return false;

}

void NesConsole::Init()
{
	Reset();
}

void NesConsole::RunFrame()
{
	int frame = _ppu->GetFrameCount();
	while(frame == _ppu->GetFrameCount()) {
		_cpu->Exec();
	}
	_emu->ProcessEndOfFrame();
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

RomInfo NesConsole::GetRomInfo()
{
	//TODO
	return RomInfo();
}

void NesConsole::RunSingleFrame()
{
	//TODO
}

PpuFrameInfo NesConsole::GetPpuFrame()
{
	//TODO
	return PpuFrameInfo();
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
