#pragma once
#include "stdafx.h"
#include "CartTypes.h"
#include "DebugTypes.h"
#include "Debugger.h"
#include "IConsole.h"
#include "Utilities/Timer.h"
#include "Utilities/VirtualFile.h"
#include "Utilities/SimpleLock.h"

class Cpu;
class Ppu;
class Spc;
class BaseCartridge;
class MemoryManager;
class InternalRegisters;
class ControlManager;
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

class Console : public std::enable_shared_from_this<Console>, public IConsole
{
private:
	unique_ptr<thread> _emuThread;

	shared_ptr<Cpu> _cpu;
	shared_ptr<Ppu> _ppu;
	shared_ptr<Spc> _spc;
	shared_ptr<MemoryManager> _memoryManager;
	shared_ptr<BaseCartridge> _cart;
	shared_ptr<InternalRegisters> _internalRegisters;
	shared_ptr<ControlManager> _controlManager;
	shared_ptr<DmaController> _dmaController;
	
	shared_ptr<Msu1> _msu1;

	shared_ptr<Debugger> _debugger;

	Emulator* _emu;

	shared_ptr<EmuSettings> _settings;

	shared_ptr<SpcHud> _spcHud;

	uint32_t _masterClockRate;

	ConsoleRegion _region;
	ConsoleType _consoleType;

	atomic<bool> _isRunAheadFrame;
	bool _frameRunning = false;

	unique_ptr<DebugStats> _stats;
	unique_ptr<FrameLimiter> _frameLimiter;
	Timer _lastFrameTimer;
	double _frameDelay = 0;

	void UpdateRegion();

	void RunFrame();
	bool ProcessSystemActions();

public:
	Console(Emulator* emu);
	~Console();

	void Initialize();
	void Release();
	
	void OnBeforeRun() override;
	void Stop() override;
	void Reset() override;

	void RunSingleFrame();

	void ProcessEndOfFrame();

	bool LoadRom(VirtualFile& romFile, VirtualFile& patchFile) override;
	void Init() override;

	RomInfo GetRomInfo();
	uint64_t GetMasterClock();
	uint32_t GetMasterClockRate();
	ConsoleRegion GetRegion();
	ConsoleType GetConsoleType();

	void Serialize(Serializer& s);

	shared_ptr<Cpu> GetCpu();
	shared_ptr<Ppu> GetPpu();
	shared_ptr<Spc> GetSpc();
	shared_ptr<BaseCartridge> GetCartridge();
	shared_ptr<MemoryManager> GetMemoryManager();
	shared_ptr<InternalRegisters> GetInternalRegisters();
	shared_ptr<ControlManager> GetControlManager();
	shared_ptr<DmaController> GetDmaController();
	shared_ptr<Msu1> GetMsu1();
	
	Emulator* GetEmulator();
	
	bool IsRunning();
	bool IsRunAheadFrame();

	double GetFrameDelay() override;
	double GetFps() override;
	PpuFrameInfo GetPpuFrame() override;
};
