#pragma once
#include "stdafx.h"
#include "Core/Debugger/DebugTypes.h"
#include "Core/Debugger/Debugger.h"
#include "Core/Shared/EmulatorLock.h"
#include "Core/Shared/Interfaces/IConsole.h"
#include "Core/Shared/Audio/AudioPlayerTypes.h"
#include "Utilities/Timer.h"
#include "Utilities/safe_ptr.h"
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
class ShortcutKeyHandler;
class SystemActionManager;
class AudioPlayerHud;
class GameServer;
class GameClient;

struct RomInfo;
struct TimingInfo;

enum class MemoryOperationType;
enum class MemoryType;
enum class EventType;
enum class ConsoleRegion;
enum class ConsoleType;
enum class HashType;

struct ConsoleMemoryInfo
{
	void* Memory;
	uint32_t Size;
};

class Emulator
{
private:
	unique_ptr<thread> _emuThread;
	unique_ptr<AudioPlayerHud> _audioPlayerHud;
	unique_ptr<IConsole> _console;

	shared_ptr<ShortcutKeyHandler> _shortcutKeyHandler;
	shared_ptr<RewindManager> _rewindManager;
	safe_ptr<Debugger> _debugger;
	shared_ptr<SystemActionManager> _systemActionManager;

	const unique_ptr<EmuSettings> _settings;
	const unique_ptr<DebugHud> _debugHud;
	const unique_ptr<NotificationManager> _notificationManager;
	const unique_ptr<BatteryManager> _batteryManager;
	const unique_ptr<SoundMixer> _soundMixer;
	const unique_ptr<VideoRenderer> _videoRenderer;
	const unique_ptr<VideoDecoder> _videoDecoder;
	const unique_ptr<SaveStateManager> _saveStateManager;
	const unique_ptr<CheatManager> _cheatManager;
	const unique_ptr<MovieManager> _movieManager;
	
	const shared_ptr<GameServer> _gameServer;
	const shared_ptr<GameClient> _gameClient;

	thread::id _emulationThreadId;

	atomic<uint32_t> _lockCounter;
	SimpleLock _runLock;
	SimpleLock _loadLock;

	SimpleLock _debuggerLock;
	atomic<bool> _stopFlag;
	atomic<bool> _paused;
	atomic<bool> _pauseOnNextFrame;
	atomic<bool> _threadPaused;

	atomic<int> _debugRequestCount;
	atomic<bool> _allowDebuggerRequest;

	atomic<bool> _isRunAheadFrame;
	bool _frameRunning = false;

	RomInfo _rom;

	ConsoleMemoryInfo _consoleMemory[(int)MemoryType::Register] = {};

	unique_ptr<DebugStats> _stats;
	unique_ptr<FrameLimiter> _frameLimiter;
	Timer _lastFrameTimer;
	double _frameDelay = 0;
	
	uint32_t _autoSaveStateFrameCounter = 0;

	void WaitForLock();
	void WaitForPauseEnd();

	void ProcessAutoSaveState();
	bool ProcessSystemActions();
	void RunFrameWithRunAhead();

	void BlockDebuggerRequests();
	void ResetDebugger(bool startDebugger = false);

	double GetFrameDelay();

public:
	class DebuggerRequest
	{
	private:
		shared_ptr<Debugger> _debugger;
		Emulator* _emu = nullptr;

	public:
		DebuggerRequest(Emulator* emu)
		{
			if(emu) {
				_emu = emu;
				_debugger = _emu->_debugger.lock();
				_emu->_debugRequestCount++;
			}
		}

		~DebuggerRequest()
		{
			if(_emu) {
				_emu->_debugRequestCount--;
			}
		}

		Debugger* GetDebugger()
		{
			return _debugger.get();
		}
	};


	Emulator();
	~Emulator();

	void Initialize();
	void Release();

	void Run();
	void RunSingleFrame();
	void Stop(bool sendNotification, bool preventRecentGameSave = false);

	void OnBeforeSendFrame();
	void ProcessEndOfFrame();

	void Reset();
	void ReloadRom(bool forPowerCycle);
	void PowerCycle();

	void PauseOnNextFrame();

	void Pause();
	void Resume();
	bool IsPaused();

	bool LoadRom(VirtualFile romFile, VirtualFile patchFile, bool stopRom = true, bool forPowerCycle = false);
	RomInfo& GetRomInfo();
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

	void SuspendDebugger(bool release);

	void Serialize(ostream& out, int compressionLevel = 1);
	void Deserialize(istream& in, uint32_t fileFormatVersion, bool compressed = true);

	SoundMixer* GetSoundMixer();
	VideoRenderer* GetVideoRenderer();
	VideoDecoder* GetVideoDecoder();
	ShortcutKeyHandler* GetShortcutKeyHandler();
	NotificationManager* GetNotificationManager();
	EmuSettings* GetSettings();
	SaveStateManager* GetSaveStateManager();
	RewindManager* GetRewindManager();
	DebugHud* GetDebugHud();
	BatteryManager* GetBatteryManager();
	CheatManager* GetCheatManager();
	MovieManager* GetMovieManager();
	GameServer* GetGameServer();
	GameClient* GetGameClient();
	shared_ptr<SystemActionManager> GetSystemActionManager();

	IControlManager* GetControlManager();
	
	BaseVideoFilter* GetVideoFilter();

	bool IsShortcutAllowed(EmulatorShortcut shortcut, uint32_t shortcutParam);

	void InitDebugger();
	void StopDebugger();
	DebuggerRequest GetDebugger(bool autoInit = false);
	bool IsDebugging();

	thread::id GetEmulationThreadId();

	void RegisterMemory(MemoryType type, void* memory, uint32_t size);
	ConsoleMemoryInfo GetMemory(MemoryType type);

	AudioTrackInfo GetAudioTrackInfo();
	void ProcessAudioPlayerAction(AudioPlayerActionParams p);
	AudioPlayerHud* GetAudioPlayerHud();

	bool IsRunning();
	bool IsRunAheadFrame();

	TimingInfo GetTimingInfo();
	uint32_t GetFrameCount();
	double GetFps();
	
	template<CpuType type> __forceinline void ProcessInstruction()
	{
		if(_debugger) {
			_debugger->ProcessInstruction<type>();
		}
	}

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

	template<CpuType type> __forceinline void ProcessPpuRead(uint32_t addr, uint8_t value, MemoryType memoryType)
	{
		if(_debugger) {
			_debugger->ProcessPpuRead<type>(addr, value, memoryType);
		}
	}

	template<CpuType type> __forceinline void ProcessPpuWrite(uint32_t addr, uint8_t value, MemoryType memoryType)
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
	void BreakIfDebugging(CpuType sourceCpu, BreakSource source);
};

enum class HashType
{
	Crc32,
	Sha1
};