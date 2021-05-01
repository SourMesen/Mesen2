#pragma once
#include "stdafx.h"
#include "Core/Debugger/DebugTypes.h"
#include "Core/Debugger/Debugger.h"
#include "Core/Shared/EmulatorLock.h"
#include "Core/Shared/Interfaces/IConsole.h"
#include "Core/Shared/Audio/AudioTrackInfo.h"
#include "Utilities/Timer.h"
#include "Utilities/SimpleLock.h"
#include "Utilities/VirtualFile.h"

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
class IControlManager;
class VirtualFile;
class BaseVideoFilter;
class SystemActionManager;
class AudioPlayerHud;

struct RomInfo;

enum class MemoryOperationType;
enum class SnesMemoryType;
enum class EventType;
enum class ConsoleRegion;
enum class ConsoleType;
enum class HashType;

struct ConsoleMemoryInfo
{
	void* Memory;
	uint32_t Size;
};

class Emulator : public std::enable_shared_from_this<Emulator>
{
private:
	unique_ptr<thread> _emuThread;

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
	shared_ptr<SystemActionManager> _systemActionManager;

	shared_ptr<IConsole> _console;

	unique_ptr<AudioPlayerHud> _audioPlayerHud;

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

	atomic<bool> _isRunAheadFrame;
	bool _frameRunning = false;

	RomInfo _rom;

	ConsoleMemoryInfo _consoleMemory[(int)SnesMemoryType::Register] = {};

	unique_ptr<DebugStats> _stats;
	unique_ptr<FrameLimiter> _frameLimiter;
	Timer _lastFrameTimer;
	double _frameDelay = 0;

	void WaitForLock();
	void WaitForPauseEnd();

	bool ProcessSystemActions();
	void RunFrameWithRunAhead();

public:
	Emulator();
	~Emulator();

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
	string GetHash(HashType type);
	uint32_t GetCrc32();
	PpuFrameInfo GetPpuFrame();
	ConsoleRegion GetRegion();
	IConsole* GetConsole();
	ConsoleType GetConsoleType();
	vector<CpuType> GetCpuTypes();
	uint64_t GetMasterClock();
	uint32_t GetMasterClockRate();

	EmulatorLock AcquireLock();
	void Lock();
	void Unlock();
	bool IsThreadPaused();

	void Serialize(ostream& out, int compressionLevel = 1);
	void Deserialize(istream& in, uint32_t fileFormatVersion, bool compressed = true);

	shared_ptr<SoundMixer> GetSoundMixer();
	shared_ptr<VideoRenderer> GetVideoRenderer();
	shared_ptr<VideoDecoder> GetVideoDecoder();
	shared_ptr<NotificationManager> GetNotificationManager();
	EmuSettings* GetSettings();
	shared_ptr<SaveStateManager> GetSaveStateManager();
	shared_ptr<RewindManager> GetRewindManager();
	shared_ptr<DebugHud> GetDebugHud();
	shared_ptr<BatteryManager> GetBatteryManager();
	shared_ptr<CheatManager> GetCheatManager();
	shared_ptr<MovieManager> GetMovieManager();
	shared_ptr<SystemActionManager> GetSystemActionManager();

	shared_ptr<IControlManager> GetControlManager();
	
	BaseVideoFilter* GetVideoFilter();

	shared_ptr<Debugger> GetDebugger(bool autoStart = true);
	void StopDebugger();
	bool IsDebugging();

	thread::id GetEmulationThreadId();

	void RegisterMemory(SnesMemoryType type, void* memory, uint32_t size);
	ConsoleMemoryInfo GetMemory(SnesMemoryType type);

	AudioTrackInfo GetAudioTrackInfo();
	AudioPlayerHud* GetAudioPlayerHud();

	bool IsRunning();
	bool IsRunAheadFrame();

	uint32_t GetFrameCount();
	double GetFps();
	double GetFrameDelay();

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

	template<CpuType type> __forceinline void ProcessPpuRead(uint32_t addr, uint8_t value, SnesMemoryType memoryType)
	{
		if(_debugger) {
			_debugger->ProcessPpuRead<type>(addr, value, memoryType);
		}
	}

	template<CpuType type> __forceinline void ProcessPpuWrite(uint32_t addr, uint8_t value, SnesMemoryType memoryType)
	{
		if(_debugger) {
			_debugger->ProcessPpuWrite<type>(addr, value, memoryType);
		}
	}

	template<CpuType type> __forceinline void ProcessPpuCycle()
	{
		if(_debugger) {
			_debugger->ProcessPpuCycle<type>();
		}
	}

	__forceinline void DebugLog(string log)
	{
		if(_debugger) {
			_debugger->Log(log);
		}
	}

	template<CpuType type> void ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi);
	void ProcessEvent(EventType type);
	template<CpuType cpuType> void AddDebugEvent(DebugEventType evtType);
	void BreakImmediately(BreakSource source);
};

enum class HashType
{
	Crc32,
	Sha1
};