#pragma once
#include "stdafx.h"
#include "Shared/Interfaces/IConsole.h"

class PceCpu;
class PceVdc;
class PceVpc;
class PceVce;
class PcePsg;
class PceCdRom;
class PceMemoryManager;
class PceControlManager;
class IPceMapper;
class Emulator;
struct PceVideoState;
enum class PceConsoleType;

class PceConsole final: public IConsole
{
private:
	Emulator* _emu;

	unique_ptr<PceCpu> _cpu;
	unique_ptr<PceVdc> _vdc;
	unique_ptr<PceVdc> _vdc2;
	unique_ptr<PceVpc> _vpc;
	unique_ptr<PceVce> _vce;
	unique_ptr<PcePsg> _psg;
	unique_ptr<PceMemoryManager> _memoryManager;
	unique_ptr<PceControlManager> _controlManager;
	unique_ptr<PceCdRom> _cdrom;
	unique_ptr<IPceMapper> _mapper;
	RomFormat _romFormat = RomFormat::Pce;
	
	static bool IsPopulousCard(uint32_t crc32);
	static bool IsSuperGrafxCard(uint32_t crc32);

public:
	PceConsole(Emulator* emu);
	
	static vector<string> GetSupportedExtensions() { return { ".pce", ".cue", ".sgx" }; }
	
	void Serialize(Serializer& s) override;

	void InitializeRam(void* data, uint32_t length);

	void Reset() override;
	
	LoadRomResult LoadRom(VirtualFile& romFile) override;

	void RunFrame() override;

	void SaveBattery() override;

	BaseControlManager* GetControlManager() override;
	ConsoleRegion GetRegion() override;
	ConsoleType GetConsoleType() override;
	vector<CpuType> GetCpuTypes() override;

	PceCpu* GetCpu();
	PceVdc* GetVdc();
	PceVce* GetVce();
	PceVpc* GetVpc();
	PceMemoryManager* GetMemoryManager();

	bool IsSuperGrafx() { return _vdc2 != nullptr; }
	
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

	PceVideoState GetVideoState();
	void GetConsoleState(BaseState& state, ConsoleType consoleType) override;
};