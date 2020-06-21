#pragma once
#include "stdafx.h"
#include "CartTypes.h"
#include "DebugTypes.h"
#include "Debugger.h"
#include "ConsoleLock.h"
#include "../Utilities/Timer.h"
#include "../Utilities/VirtualFile.h"
#include "../Utilities/SimpleLock.h"

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

enum class MemoryOperationType;
enum class SnesMemoryType;
enum class EventType;
enum class ConsoleRegion;
enum class ConsoleType;

class Console : public std::enable_shared_from_this<Console>
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

	shared_ptr<NotificationManager> _notificationManager;
	shared_ptr<BatteryManager> _batteryManager;
	shared_ptr<SoundMixer> _soundMixer;
	shared_ptr<VideoRenderer> _videoRenderer;
	shared_ptr<VideoDecoder> _videoDecoder;
	shared_ptr<DebugHud> _debugHud;
	shared_ptr<EmuSettings> _settings;
	shared_ptr<SaveStateManager> _saveStateManager;
	shared_ptr<RewindManager> _rewindManager;
	shared_ptr<CheatManager> _cheatManager;
	shared_ptr<MovieManager> _movieManager;
	shared_ptr<SpcHud> _spcHud;

	thread::id _emulationThreadId;
	
	atomic<uint32_t> _lockCounter;
	SimpleLock _runLock;
	SimpleLock _emulationLock;

	SimpleLock _debuggerLock;
	atomic<bool> _stopFlag;
	atomic<bool> _paused;
	atomic<bool> _pauseOnNextFrame;
	atomic<bool> _threadPaused;

	ConsoleRegion _region;
	ConsoleType _consoleType;
	uint32_t _masterClockRate;

	atomic<bool> _isRunAheadFrame;
	bool _frameRunning = false;

	unique_ptr<DebugStats> _stats;
	unique_ptr<FrameLimiter> _frameLimiter;
	Timer _lastFrameTimer;
	double _frameDelay = 0;

	double GetFrameDelay();
	void UpdateRegion();
	void WaitForLock();
	void WaitForPauseEnd();

	void RunFrame();
	bool ProcessSystemActions();
	void RunFrameWithRunAhead();

public:
	Console();
	~Console();

	void Initialize();
	void Release();


	void Run();
	void RunSingleFrame();
	void Stop(bool sendNotification);

	void ProcessEndOfFrame();

	void Reset();
	void ReloadRom(bool forPowerCycle);
	void PowerCycle();

	void PauseOnNextFrame();
	
	void Pause();
	void Resume();
	bool IsPaused();

	bool LoadRom(VirtualFile romFile, VirtualFile patchFile, bool stopRom = true, bool forPowerCycle = false);
	RomInfo GetRomInfo();
	uint64_t GetMasterClock();
	uint32_t GetMasterClockRate();
	ConsoleRegion GetRegion();
	ConsoleType GetConsoleType();

	ConsoleLock AcquireLock();
	void Lock();
	void Unlock();
	bool IsThreadPaused();

	void Serialize(ostream &out, int compressionLevel = 1);
	void Deserialize(istream &in, uint32_t fileFormatVersion, bool compressed = true);

	shared_ptr<SoundMixer> GetSoundMixer();
	shared_ptr<VideoRenderer> GetVideoRenderer();
	shared_ptr<VideoDecoder> GetVideoDecoder();
	shared_ptr<NotificationManager> GetNotificationManager();
	shared_ptr<EmuSettings> GetSettings();
	shared_ptr<SaveStateManager> GetSaveStateManager();
	shared_ptr<RewindManager> GetRewindManager();
	shared_ptr<DebugHud> GetDebugHud();
	shared_ptr<BatteryManager> GetBatteryManager();
	shared_ptr<CheatManager> GetCheatManager();
	shared_ptr<MovieManager> GetMovieManager();

	shared_ptr<Cpu> GetCpu();
	shared_ptr<Ppu> GetPpu();
	shared_ptr<Spc> GetSpc();
	shared_ptr<BaseCartridge> GetCartridge();
	shared_ptr<MemoryManager> GetMemoryManager();
	shared_ptr<InternalRegisters> GetInternalRegisters();
	shared_ptr<ControlManager> GetControlManager();
	shared_ptr<DmaController> GetDmaController();
	shared_ptr<Msu1> GetMsu1();

	shared_ptr<Debugger> GetDebugger(bool autoStart = true);
	void StopDebugger();
	bool IsDebugging();

	thread::id GetEmulationThreadId();
	
	bool IsRunning();
	bool IsRunAheadFrame();

	uint32_t GetFrameCount();	
	double GetFps();

	template<CpuType type> __forceinline void ProcessMemoryRead(uint32_t addr, uint8_t value, MemoryOperationType opType)
	{
		if(_debugger) {
			_debugger->ProcessMemoryRead<type>(addr, value, opType);
		}
	}

	template<CpuType type> __forceinline void ProcessMemoryWrite(uint32_t addr, uint8_t value, MemoryOperationType opType)
	{
		if(_debugger) {
			_debugger->ProcessMemoryWrite<type>(addr, value, opType);
		}
	}

	__forceinline void ProcessPpuRead(uint32_t addr, uint8_t value, SnesMemoryType memoryType)
	{
		if(_debugger) {
			_debugger->ProcessPpuRead(addr, value, memoryType);
		}
	}

	__forceinline void ProcessPpuWrite(uint32_t addr, uint8_t value, SnesMemoryType memoryType)
	{
		if(_debugger) {
			_debugger->ProcessPpuWrite(addr, value, memoryType);
		}
	}

	__forceinline void ProcessWorkRamRead(uint32_t addr, uint8_t value)
	{
		if(_debugger) {
			_debugger->ProcessWorkRamRead(addr, value);
		}
	}

	__forceinline void ProcessWorkRamWrite(uint32_t addr, uint8_t value)
	{
		if(_debugger) {
			_debugger->ProcessWorkRamWrite(addr, value);
		}
	}
	
	template<CpuType cpuType> __forceinline void ProcessPpuCycle()
	{
		if(_debugger) {
			_debugger->ProcessPpuCycle<cpuType>();
		}
	}

	template<CpuType type> void ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi);
	void ProcessEvent(EventType type);
	void BreakImmediately(BreakSource source);
};
