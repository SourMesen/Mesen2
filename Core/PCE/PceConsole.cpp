#include "stdafx.h"
#include "Shared/SettingTypes.h"
#include "PCE/PceConsole.h"
#include "PCE/PceControlManager.h"
#include "PCE/PceMemoryManager.h"
#include "PCE/PceDefaultVideoFilter.h"
#include "PCE/PceCpu.h"
#include "PCE/PcePpu.h"
#include "PCE/PceConstants.h"
#include "MemoryType.h"

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
	vector<uint8_t> data;
	romFile.ReadFile(data);

	_controlManager.reset(new PceControlManager(_emu));
	_ppu.reset(new PcePpu(_emu, this));
	_memoryManager.reset(new PceMemoryManager(_emu, _ppu.get(), _controlManager.get(), data));
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
	uint32_t frameCount = _ppu->GetState().FrameCount;
	while(frameCount == _ppu->GetState().FrameCount) {
		_cpu->Exec();
	}
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
	return 60.0;
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
	frame.ScanlineCount = 263;

	//TODO
	//frame.FrameBuffer = ...
	//frame.Height
	//frame.Width
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
}
