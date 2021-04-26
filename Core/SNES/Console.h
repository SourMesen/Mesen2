#pragma once
#include "stdafx.h"
#include "SNES/CartTypes.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/Debugger.h"
#include "Shared/Interfaces/IConsole.h"
#include "Utilities/Timer.h"
#include "Utilities/VirtualFile.h"
#include "Utilities/SimpleLock.h"
#include "Shared/RomInfo.h"

class Cpu;
class Ppu;
class Spc;
class BaseCartridge;
class MemoryManager;
class InternalRegisters;
class ControlManager;
class IControlManager;
class DmaController;
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
class SpcHud;
class FrameLimiter;
class DebugStats;
class Msu1;

class Emulator;

enum class MemoryOperationType;
enum class SnesMemoryType;
enum class EventType;
enum class ConsoleRegion;
enum class ConsoleType;

class Console final : public std::enable_shared_from_this<Console>, public IConsole
{
private:
	shared_ptr<Cpu> _cpu;
	shared_ptr<Ppu> _ppu;
	shared_ptr<Spc> _spc;
	shared_ptr<MemoryManager> _memoryManager;
	shared_ptr<BaseCartridge> _cart;
	shared_ptr<InternalRegisters> _internalRegisters;
	shared_ptr<ControlManager> _controlManager;
	shared_ptr<DmaController> _dmaController;
	
	shared_ptr<Msu1> _msu1;
	EmuSettings* _settings;
	shared_ptr<SpcHud> _spcHud;
	Emulator* _emu;

	uint32_t _masterClockRate;
	ConsoleRegion _region;
	bool _frameRunning = false;

	void UpdateRegion();

public:
	Console(Emulator* emu);
	~Console();

	void Initialize();
	void Release();
	
	void OnBeforeRun() override;
	void Stop() override;
	void Reset() override;

	void RunFrame() override;
	void RunSingleFrame() override;

	void ProcessEndOfFrame();

	bool LoadRom(VirtualFile& romFile) override;
	void Init() override;

	uint64_t GetMasterClock() override;
	uint32_t GetMasterClockRate();
	ConsoleRegion GetRegion();
	ConsoleType GetConsoleType() override;

	void Serialize(Serializer& s) override;

	shared_ptr<Cpu> GetCpu();
	shared_ptr<Ppu> GetPpu();
	shared_ptr<Spc> GetSpc();
	shared_ptr<BaseCartridge> GetCartridge();
	shared_ptr<MemoryManager> GetMemoryManager();
	shared_ptr<InternalRegisters> GetInternalRegisters();
	shared_ptr<IControlManager> GetControlManager() override;
	shared_ptr<DmaController> GetDmaController();
	shared_ptr<Msu1> GetMsu1();
	
	Emulator* GetEmulator();
	
	bool IsRunning();

	AddressInfo GetAbsoluteAddress(AddressInfo relAddress) override;
	AddressInfo GetRelativeAddress(AddressInfo absAddress, CpuType cpuType) override;

	double GetFrameDelay() override;
	double GetFps() override;
	PpuFrameInfo GetPpuFrame() override;

	vector<CpuType> GetCpuTypes() override;
	void SaveBattery() override;

	BaseVideoFilter* GetVideoFilter() override;
};
