#include "stdafx.h"
#include "Shared/SettingTypes.h"
#include "PCE/PceConsole.h"
#include "PCE/PceControlManager.h"
#include "PCE/PceMemoryManager.h"
#include "PCE/PceDefaultVideoFilter.h"
#include "PCE/PceCpu.h"
#include "PCE/PcePpu.h"
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
	}

	_controlManager.reset(new PceControlManager(_emu));
	_ppu.reset(new PcePpu(_emu, this));
	_psg.reset(new PcePsg(_emu));
	_memoryManager.reset(new PceMemoryManager(_emu, this, _ppu.get(), _controlManager.get(), _psg.get(), _cdrom.get(), romData));
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
	uint32_t frameCount = _ppu->GetFrameCount();
	while(frameCount == _ppu->GetFrameCount()) {
		_cpu->Exec();
	}

	_psg->Run();
}

void PceConsole::SaveBattery()
{
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

PcePpu* PceConsole::GetPpu()
{
	return _ppu.get();
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
	return new PceDefaultVideoFilter(_emu);
}

PpuFrameInfo PceConsole::GetPpuFrame()
{
	PpuFrameInfo frame = {};
	PcePpuState& state = _ppu->GetState();
	frame.FrameCount = state.FrameCount;
	frame.CycleCount = 341;

	frame.FirstScanline = 0;
	frame.ScanlineCount = PceConstants::ScanlineCount;

	frame.FrameBuffer = (uint8_t*)_ppu->GetScreenBuffer();
	frame.Height = PceConstants::ScreenHeight;
	frame.Width = _ppu->GetScreenWidth();
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
	state.Ppu = _ppu->GetState();
	state.MemoryManager = _memoryManager->GetState();
	state.Psg = _psg->GetState();
	for(int i = 0; i < 6; i++) {
		state.PsgChannels[i] = _psg->GetChannelState(i);
	}
}
