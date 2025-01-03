#pragma once
#include "pch.h"
#include "SNES/CartTypes.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/Debugger.h"
#include "Shared/Interfaces/IConsole.h"
#include "Utilities/Timer.h"
#include "Utilities/VirtualFile.h"
#include "Utilities/SimpleLock.h"
#include "Shared/RomInfo.h"

class SnesCpu;
class SnesPpu;
class Spc;
class BaseCartridge;
class SnesMemoryManager;
class InternalRegisters;
class SnesControlManager;
class BaseControlManager;
class SnesDmaController;
class Debugger;
class DebugHud;
class SoundMixer;
class VideoRenderer;
class VideoDecoder;
class NotificationManager;
class EmuSettings;
class SaveStateManager;
class RewindManager;
class BatteryManager;
class CheatManager;
class MovieManager;
class FrameLimiter;
class DebugStats;
class Msu1;

class Emulator;

enum class MemoryOperationType;
enum class MemoryType;
enum class EventType;
enum class ConsoleRegion;
enum class ConsoleType;

class SnesConsole final : public IConsole
{
private:
	unique_ptr<SnesCpu> _cpu;
	unique_ptr<SnesPpu> _ppu;
	unique_ptr<Spc> _spc;
	unique_ptr<SnesMemoryManager> _memoryManager;
	unique_ptr<BaseCartridge> _cart;
	unique_ptr<InternalRegisters> _internalRegisters;
	unique_ptr<SnesControlManager> _controlManager;
	unique_ptr<SnesDmaController> _dmaController;
	
	unique_ptr<Msu1> _msu1;
	EmuSettings* _settings = nullptr;
	Emulator* _emu = nullptr;

	vector<string> _spcPlaylist;
	uint32_t _spcTrackNumber = 0;

	uint32_t _masterClockRate = 0;
	ConsoleRegion _region = {};
	bool _frameRunning = false;

	void UpdateRegion();
	bool LoadSpcFile(VirtualFile& romFile);

public:
	SnesConsole(Emulator* emu);
	~SnesConsole();
	
	static vector<string> GetSupportedExtensions() { return { ".sfc", ".swc", ".fig", ".smc", ".bs", ".gb", ".gbc", ".gbx", ".spc", ".st" }; }
	static vector<string> GetSupportedSignatures() { return { "SNES-SPC700 Sound File Data" }; }

	void Initialize();
	void Release();
	
	void Reset() override;

	void RunFrame() override;

	void ProcessEndOfFrame();

	LoadRomResult LoadRom(VirtualFile& romFile) override;

	uint64_t GetMasterClock() override;
	uint32_t GetMasterClockRate() override;
	ConsoleRegion GetRegion() override;
	ConsoleType GetConsoleType() override;

	void Serialize(Serializer& s) override;
	SaveStateCompatInfo ValidateSaveStateCompatibility(ConsoleType stateConsoleType) override;

	SnesCpu* GetCpu();
	SnesPpu* GetPpu();
	Spc* GetSpc();
	BaseCartridge* GetCartridge();
	SnesMemoryManager* GetMemoryManager();
	InternalRegisters* GetInternalRegisters();
	BaseControlManager* GetControlManager() override;
	SnesDmaController* GetDmaController();
	Msu1* GetMsu1();
	
	Emulator* GetEmulator();
	
	bool IsRunning();

	AddressInfo GetAbsoluteAddress(AddressInfo& relAddress) override;
	AddressInfo GetRelativeAddress(AddressInfo& absAddress, CpuType cpuType) override;
	void GetConsoleState(BaseState& state, ConsoleType consoleType) override;

	void InitializeRam(void* data, uint32_t length);

	double GetFps() override;
	PpuFrameInfo GetPpuFrame() override;

	TimingInfo GetTimingInfo(CpuType cpuType) override;

	vector<CpuType> GetCpuTypes() override;
	void SaveBattery() override;

	BaseVideoFilter* GetVideoFilter(bool getDefaultFilter) override;

	RomFormat GetRomFormat() override;
	AudioTrackInfo GetAudioTrackInfo() override;
	void ProcessAudioPlayerAction(AudioPlayerActionParams p) override;
};
