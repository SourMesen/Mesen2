#pragma once
#include "pch.h"
#include "Shared/Interfaces/IConsole.h"

class PceCpu;
class PceVdc;
class PceVpc;
class PceVce;
class PcePsg;
class PceTimer;
class PceCdRom;
class PceMemoryManager;
class PceControlManager;
class IPceMapper;
class Emulator;
struct PceVideoState;
struct HesFileData;
struct DiscInfo;
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
	unique_ptr<PceTimer> _timer;
	unique_ptr<PceMemoryManager> _memoryManager;
	unique_ptr<PceControlManager> _controlManager;
	unique_ptr<PceCdRom> _cdrom;
	unique_ptr<IPceMapper> _mapper;
	unique_ptr<HesFileData> _hesData;
	RomFormat _romFormat = RomFormat::Pce;

	static bool IsPopulousCard(uint32_t crc32);
	static bool IsSuperGrafxCard(uint32_t crc32);

	bool LoadHesFile(VirtualFile& hesFile);
	bool LoadFirmware(DiscInfo& disc, vector<uint8_t>& romData);

public:
	PceConsole(Emulator* emu);
	virtual ~PceConsole();
	
	static vector<string> GetSupportedExtensions() { return { ".pce", ".cue", ".sgx", ".hes" }; }
	static vector<string> GetSupportedSignatures() { return { "HESM" }; }

	void Serialize(Serializer& s) override;

	void InitializeRam(void* data, uint32_t length);

	void Reset() override;
	
	LoadRomResult LoadRom(VirtualFile& romFile) override;

	void RunFrame() override;

	void ProcessEndOfFrame();

	void SaveBattery() override;

	BaseControlManager* GetControlManager() override;
	ConsoleRegion GetRegion() override;
	ConsoleType GetConsoleType() override;
	vector<CpuType> GetCpuTypes() override;

	PceCpu* GetCpu();
	PceVdc* GetVdc();
	PceVce* GetVce();
	PceVpc* GetVpc();
	PcePsg* GetPsg();
	PceMemoryManager* GetMemoryManager();

	bool IsSuperGrafx() { return _vdc2 != nullptr; }
	
	uint64_t GetMasterClock() override;
	uint32_t GetMasterClockRate() override;
	double GetFps() override;

	BaseVideoFilter* GetVideoFilter(bool getDefaultFilter) override;

	PpuFrameInfo GetPpuFrame() override;
	RomFormat GetRomFormat() override;

	void InitHesPlayback(uint8_t selectedTrack);
	AudioTrackInfo GetAudioTrackInfo() override;
	void ProcessAudioPlayerAction(AudioPlayerActionParams p) override;

	AddressInfo GetAbsoluteAddress(AddressInfo& relAddress) override;
	AddressInfo GetRelativeAddress(AddressInfo& absAddress, CpuType cpuType) override;

	PceVideoState GetVideoState();
	void SetVideoState(PceVideoState& state);
	void GetConsoleState(BaseState& state, ConsoleType consoleType) override;
};