#include "stdafx.h"
#include "Shared/SettingTypes.h"
#include "PCE/PceConsole.h"
#include "PCE/PceControlManager.h"
#include "PCE/PceMemoryManager.h"
#include "PCE/PceDefaultVideoFilter.h"
#include "PCE/PceNtscFilter.h"
#include "PCE/PceCpu.h"
#include "PCE/PceVdc.h"
#include "PCE/PceVce.h"
#include "PCE/PcePsg.h"
#include "PCE/PceConstants.h"
#include "MemoryType.h"
#include "FirmwareHelper.h"

PceConsole::PceConsole(Emulator* emu)
{
	_emu = emu;
}

void PceConsole::Serialize(Serializer& s)
{
}

void PceConsole::Stop()
{
}

void PceConsole::Reset()
{
}

void PceConsole::OnBeforeRun()
{
}

LoadRomResult PceConsole::LoadRom(VirtualFile& romFile)
{
	PcEngineConfig& cfg = _emu->GetSettings()->GetPcEngineConfig();

	vector<uint8_t> romData;
	if(romFile.GetFileExtension() == ".cue") {
		if(!FirmwareHelper::LoadPceSuperCdFirmware(_emu, romData)) {
			return LoadRomResult::Failure;
		}

		DiscInfo disc = {};
		if(!CdReader::LoadCue(romFile, disc)) {
			return LoadRomResult::Failure;
		}

		_cdrom.reset(new PceCdRom(_emu, this, disc));
	} else {
		romFile.ReadFile(romData);
		if((romData.size() % 0x2000) == 512) {
			//File probably has header, discard it
			romData.erase(romData.begin(), romData.begin() + 512);
		}

		if(cfg.EnableCdRomForHuCardGames) {
			DiscInfo emptyDisc = {};
			_cdrom.reset(new PceCdRom(_emu, this, emptyDisc));
		}
	}

	_controlManager.reset(new PceControlManager(_emu));
	_vce.reset(new PceVce(_emu, this));
	_vdc.reset(new PceVdc(_emu, this, _vce.get()));
	_psg.reset(new PcePsg(_emu));
	_memoryManager.reset(new PceMemoryManager(_emu, this, _vdc.get(), _vce.get(), _controlManager.get(), _psg.get(), _cdrom.get(), romData));
	_cpu.reset(new PceCpu(_emu, this, _memoryManager.get()));

	MessageManager::Log("-----------------");
	MessageManager::Log("Loaded: " + romFile.GetFileName());
	MessageManager::Log("-----------------");

	return LoadRomResult::Success;
}

void PceConsole::Init()
{
}

void PceConsole::RunFrame()
{
	uint32_t frameCount = _vdc->GetFrameCount();
	while(frameCount == _vdc->GetFrameCount()) {
		_cpu->Exec();
	}

	_psg->Run();
}

void PceConsole::SaveBattery()
{
	_memoryManager->SaveBattery();
}

BaseControlManager* PceConsole::GetControlManager()
{
	return _controlManager.get();
}

ConsoleRegion PceConsole::GetRegion()
{
	return ConsoleRegion::Ntsc;
}

ConsoleType PceConsole::GetConsoleType()
{
	return ConsoleType::PcEngine;
}

vector<CpuType> PceConsole::GetCpuTypes()
{
	return { CpuType::Pce };
}

PceCpu* PceConsole::GetCpu()
{
	return _cpu.get();
}

PceVdc* PceConsole::GetVdc()
{
	return _vdc.get();
}

PceMemoryManager* PceConsole::GetMemoryManager()
{
	return _memoryManager.get();
}

uint64_t PceConsole::GetMasterClock()
{
	return _memoryManager->GetState().CycleCount;
}

uint32_t PceConsole::GetMasterClockRate()
{
	return PceConstants::MasterClockRate;
}

double PceConsole::GetFps()
{
	//59.82609786
	return (double)PceConstants::MasterClockRate / PceConstants::ClockPerScanline / PceConstants::ScanlineCount;
}

BaseVideoFilter* PceConsole::GetVideoFilter()
{
	VideoFilterType filterType = _emu->GetSettings()->GetVideoConfig().VideoFilter;

	switch(filterType) {
		case VideoFilterType::NtscBlargg:
		case VideoFilterType::NtscBisqwit:
			return new PceNtscFilter(_emu);

		default:
			return new PceDefaultVideoFilter(_emu);
	}
}

PpuFrameInfo PceConsole::GetPpuFrame()
{
	PpuFrameInfo frame = {};
	PceVdcState& state = _vdc->GetState();
	frame.FrameCount = state.FrameCount;
	frame.CycleCount = 341;

	frame.FirstScanline = 0;
	frame.ScanlineCount = PceConstants::ScanlineCount;

	frame.FrameBuffer = (uint8_t*)_vdc->GetScreenBuffer();
	frame.Height = PceConstants::ScreenHeight;
	frame.Width = PceConstants::MaxScreenWidth;
	return frame;
}

RomFormat PceConsole::GetRomFormat()
{
	return RomFormat::Pce;
}

AudioTrackInfo PceConsole::GetAudioTrackInfo()
{
	return {};
}

void PceConsole::ProcessAudioPlayerAction(AudioPlayerActionParams p)
{
}

AddressInfo PceConsole::GetAbsoluteAddress(AddressInfo& relAddress)
{
	if(relAddress.Type == MemoryType::PceMemory) {
		return _memoryManager->GetAbsoluteAddress(relAddress.Address);
	}
	return { -1, MemoryType::Register };
}

AddressInfo PceConsole::GetRelativeAddress(AddressInfo& absAddress, CpuType cpuType)
{
	return _memoryManager->GetRelativeAddress(absAddress);
}

void PceConsole::GetConsoleState(BaseState& baseState, ConsoleType consoleType)
{
	PceState& state = (PceState&)baseState;
	state.Cpu = _cpu->GetState();
	state.Vdc = _vdc->GetState();
	state.Vce = _vce->GetState();
	state.MemoryManager = _memoryManager->GetState();
	state.Psg = _psg->GetState();
	for(int i = 0; i < 6; i++) {
		state.PsgChannels[i] = _psg->GetChannelState(i);
	}
}
