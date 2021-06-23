#pragma once 
#include "stdafx.h"

#include "Shared/SettingTypes.h"
#include "Shared/Interfaces/IConsole.h"
#include "Debugger/DebugTypes.h"

class Emulator;
class NesCpu;
class BaseNesPpu;
class NesApu;
class NesMemoryManager;
class NesControlManager;
class BaseMapper;
class EmuSettings;
class NesSoundMixer;
class BaseVideoFilter;
class HdAudioDevice;
struct HdPackData;

enum class DebugEventType;
enum class EventType;
enum class ConsoleRegion;

class NesConsole final : public IConsole, public std::enable_shared_from_this<NesConsole>
{
private:
	Emulator* _emu = nullptr;

	unique_ptr<NesConsole> _vsSubConsole;
	NesConsole* _vsMainConsole = nullptr;

	unique_ptr<NesCpu> _cpu;
	unique_ptr<BaseNesPpu> _ppu;
	unique_ptr<NesApu> _apu;
	unique_ptr<NesMemoryManager> _memoryManager;
	unique_ptr<BaseMapper> _mapper;
	shared_ptr<NesControlManager> _controlManager;
	unique_ptr<NesSoundMixer> _mixer;

	unique_ptr<HdPackData> _hdData;
	unique_ptr<HdAudioDevice> _hdAudioDevice;

	ConsoleRegion _region = ConsoleRegion::Auto;
	
	void UpdateRegion();
	void LoadHdPack(VirtualFile& romFile);

public:
	NesConsole(Emulator* emulator);
	~NesConsole();

	NesCpu* GetCpu() { return _cpu.get(); }
	BaseNesPpu* GetPpu() { return _ppu.get(); }
	NesApu* GetApu() { return _apu.get(); }
	NesMemoryManager* GetMemoryManager() { return _memoryManager.get(); }
	BaseMapper* GetMapper() { return _mapper.get(); }
	NesSoundMixer* GetSoundMixer() { return _mixer.get(); }
	Emulator* GetEmulator();
	NesConfig& GetNesConfig();

	void ProcessCpuClock();

	NesConsole* GetVsMainConsole();
	NesConsole* GetVsSubConsole();
	bool IsVsMainConsole();
	void RunVsSubConsole();

	//TODO
	bool IsNsf() { return false; }
	void DebugSetLastFramePpuScroll(uint16_t addr, uint8_t xScroll, bool updateHorizontalScrollOnly) {}
	void SetNextFrameOverclockStatus(bool enabled) {}

	// Inherited via IConsole
	void Serialize(Serializer& s) override;
	void Stop() override;
	void Reset() override;
	void OnBeforeRun() override;
	LoadRomResult LoadRom(VirtualFile& romFile) override;
	void Init() override;
	void RunFrame() override;
	shared_ptr<IControlManager> GetControlManager() override;
	double GetFrameDelay() override;
	double GetFps() override;
	void RunSingleFrame() override;
	PpuFrameInfo GetPpuFrame() override;
	ConsoleRegion GetRegion() override;
	ConsoleType GetConsoleType() override;
	vector<CpuType> GetCpuTypes() override;

	virtual AddressInfo GetAbsoluteAddress(AddressInfo& relAddress) override;
	virtual AddressInfo GetRelativeAddress(AddressInfo& absAddress, CpuType cpuType) override;

	uint64_t GetMasterClock() override;
	uint32_t GetMasterClockRate() override;

	void SaveBattery() override;

	BaseVideoFilter* GetVideoFilter() override;

	RomFormat GetRomFormat() override;
	AudioTrackInfo GetAudioTrackInfo() override;
	void ProcessAudioPlayerAction(AudioPlayerActionParams p) override;
	uint8_t DebugRead(uint16_t addr);
	void DebugWrite(uint16_t addr, uint8_t value);
	uint8_t DebugReadVram(uint16_t addr);
	void DebugWriteVram(uint16_t addr, uint8_t value);
};
