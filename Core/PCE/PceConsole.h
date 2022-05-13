#pragma once
#include "stdafx.h"
#include "Shared/Interfaces/IConsole.h"

class PceCpu;
class PcePpu;
class PceVce;
class PcePsg;
class PceCdRom;
class PceMemoryManager;
class PceControlManager;
class Emulator;

class PceConsole final: public IConsole
{
private:
	Emulator* _emu;

	unique_ptr<PceCpu> _cpu;
	unique_ptr<PcePpu> _ppu;
	unique_ptr<PceVce> _vce;
	unique_ptr<PcePsg> _psg;
	unique_ptr<PceMemoryManager> _memoryManager;
	unique_ptr<PceControlManager> _controlManager;
	unique_ptr<PceCdRom> _cdrom;

public:
	PceConsole(Emulator* emu);

	void Serialize(Serializer& s) override;

	void Stop() override;
	void Reset() override;
	void Init() override;
	
	LoadRomResult LoadRom(VirtualFile& romFile) override;

	void OnBeforeRun() override;
	void RunFrame() override;

	void SaveBattery() override;

	BaseControlManager* GetControlManager() override;
	ConsoleRegion GetRegion() override;
	ConsoleType GetConsoleType() override;
	vector<CpuType> GetCpuTypes() override;

	PceCpu* GetCpu();
	PcePpu* GetPpu();
	PceMemoryManager* GetMemoryManager();
	
	uint64_t GetMasterClock() override;
	uint32_t GetMasterClockRate() override;
	double GetFps() override;

	BaseVideoFilter* GetVideoFilter() override;

	PpuFrameInfo GetPpuFrame() override;
	RomFormat GetRomFormat() override;

	AudioTrackInfo GetAudioTrackInfo() override;

	void ProcessAudioPlayerAction(AudioPlayerActionParams p) override;

	AddressInfo GetAbsoluteAddress(AddressInfo& relAddress) override;
	AddressInfo GetRelativeAddress(AddressInfo& absAddress, CpuType cpuType) override;

	void GetConsoleState(BaseState& state, ConsoleType consoleType) override;
};