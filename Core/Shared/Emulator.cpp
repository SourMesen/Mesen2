#include "stdafx.h"
#include <assert.h>
#include "Shared/Emulator.h"
#include "Shared/NotificationManager.h"
#include "Shared/Audio/SoundMixer.h"
#include "Shared/Audio/AudioPlayerHud.h"
#include "Shared/Video/VideoDecoder.h"
#include "Shared/Video/VideoRenderer.h"
#include "Shared/Video/DebugHud.h"
#include "Shared/FrameLimiter.h"
#include "Shared/MessageManager.h"
#include "Shared/KeyManager.h"
#include "Shared/EmuSettings.h"
#include "Shared/SaveStateManager.h"
#include "Shared/Video/DebugStats.h"
#include "Shared/RewindManager.h"
#include "Shared/ShortcutKeyHandler.h"
#include "Shared/EmulatorLock.h"
#include "Shared/Movies/MovieManager.h"
#include "Shared/BatteryManager.h"
#include "Shared/CheatManager.h"
#include "Shared/SystemActionManager.h"
#include "Shared/Movies/MovieManager.h"
#include "Shared/TimingInfo.h"
#include "Netplay/GameServer.h"
#include "Netplay/GameClient.h"
#include "Shared/Interfaces/IConsole.h"
#include "Shared/Interfaces/IBarcodeReader.h"
#include "Shared/Interfaces/ITapeRecorder.h"
#include "Shared/BaseControlManager.h"
#include "Shared/SystemActionManager.h"
#include "SNES/SnesConsole.h"
#include "SNES/SnesDefaultVideoFilter.h"
#include "NES/NesConsole.h"
#include "Gameboy/Gameboy.h"
#include "PCE/PceConsole.h"
#include "Debugger/Debugger.h"
#include "Debugger/BaseEventManager.h"
#include "Debugger/DebugTypes.h"
#include "Utilities/Serializer.h"
#include "Utilities/Timer.h"
#include "Utilities/VirtualFile.h"
#include "Utilities/PlatformUtilities.h"
#include "Utilities/FolderUtilities.h"
#include "MemoryOperationType.h"
#include "EventType.h"

Emulator::Emulator() :
	_settings(new EmuSettings(this)),
	_debugHud(new DebugHud()),
	_notificationManager(new NotificationManager()),
	_batteryManager(new BatteryManager()),
	_soundMixer(new SoundMixer(this)),
	_videoRenderer(new VideoRenderer(this)),
	_videoDecoder(new VideoDecoder(this)),
	_saveStateManager(new SaveStateManager(this)),
	_cheatManager(new CheatManager(this)),
	_movieManager(new MovieManager(this)),
	_gameServer(new GameServer(this)),
	_gameClient(new GameClient(this))
{
	_paused = false;
	_pauseOnNextFrame = false;
	_stopFlag = false;
	_isRunAheadFrame = false;
	_lockCounter = 0;
	_threadPaused = false;
	_lockCounter = 0;

	_debugRequestCount = 0;
	_allowDebuggerRequest = true;

	_videoDecoder->Init();
}

Emulator::~Emulator()
{
}

void Emulator::Initialize()
{
	_systemActionManager.reset(new SystemActionManager(this));
	_shortcutKeyHandler.reset(new ShortcutKeyHandler(this));
	_notificationManager->RegisterNotificationListener(_shortcutKeyHandler);

	_videoDecoder->StartThread();
	_videoRenderer->StartThread();
}

void Emulator::Release()
{
	Stop(true);

	_gameClient->Disconnect();
	_gameServer->StopServer();

	_videoDecoder->StopThread();
	_videoRenderer->StopThread();
	_shortcutKeyHandler.reset();
}

void Emulator::Run()
{
	if(!_console) {
		return;
	}

	while(!_runLock.TryAcquire(50)) {
		if(_stopFlag) {
			return;
		}
	}

	_stopFlag = false;
	_isRunAheadFrame = false;

	PlatformUtilities::EnableHighResolutionTimer();

	_emulationThreadId = std::this_thread::get_id();

	_frameDelay = GetFrameDelay();
	_stats.reset(new DebugStats());
	_frameLimiter.reset(new FrameLimiter(_frameDelay));
	_lastFrameTimer.Reset();

	while(!_stopFlag) {
		bool useRunAhead = _settings->GetEmulationConfig().RunAheadFrames > 0 && !_debugger && !_audioPlayerHud && !_rewindManager->IsRewinding() && _settings->GetEmulationSpeed() > 0 && _settings->GetEmulationSpeed() <= 100;
		if(useRunAhead) {
			RunFrameWithRunAhead();
		} else {
			_console->RunFrame();
			_rewindManager->ProcessEndOfFrame();
			ProcessSystemActions();
		}

		ProcessAutoSaveState();

		WaitForLock();

		if(_pauseOnNextFrame) {
			_pauseOnNextFrame = false;
			_paused = true;
		}

		if(_paused && !_stopFlag && !_debugger) {
			WaitForPauseEnd();
		}
	}

	_movieManager->Stop();

	_emulationThreadId = thread::id();

	if(_runLock.IsLockedByCurrentThread()) {
		//Lock might not be held by current frame is _stopFlag was set to interrupt the thread
		_runLock.Release();
	}

	PlatformUtilities::RestoreTimerResolution();
}

void Emulator::ProcessAutoSaveState()
{
	if(_autoSaveStateFrameCounter > 0) {
		_autoSaveStateFrameCounter--;
		if(_autoSaveStateFrameCounter == 0) {
			_saveStateManager->SaveState(SaveStateManager::AutoSaveStateIndex, false);
		}
	} else {
		uint32_t saveStateDelay = _settings->GetPreferences().AutoSaveStateDelay;
		if(saveStateDelay > 0) {
			_autoSaveStateFrameCounter = (uint32_t)(GetFps() * saveStateDelay * 60);
		}
	}
}

bool Emulator::ProcessSystemActions()
{
	if(_systemActionManager->IsResetPressed()) {
		Reset();
		
		shared_ptr<Debugger> debugger = _debugger.lock();
		if(debugger) {
			debugger->ResetSuspendCounter();
		}

		return true;
	} else if(_systemActionManager->IsPowerCyclePressed()) {
		PowerCycle();
		return true;
	}
	return false;
}

void Emulator::RunFrameWithRunAhead()
{
	stringstream runAheadState;
	uint32_t frameCount = _settings->GetEmulationConfig().RunAheadFrames;

	//Run a single frame and save the state (no audio/video)
	_isRunAheadFrame = true;
	_console->RunFrame();
	Serialize(runAheadState, false, 0);

	while(frameCount > 1) {
		//Run extra frames if the requested run ahead frame count is higher than 1
		frameCount--;
		_console->RunFrame();
	}
	_isRunAheadFrame = false;

	//Run one frame normally (with audio/video output)
	_console->RunFrame();
	_rewindManager->ProcessEndOfFrame();

	bool wasReset = ProcessSystemActions();
	if(!wasReset) {
		//Load the state we saved earlier
		_isRunAheadFrame = true;
		Deserialize(runAheadState, SaveStateManager::FileFormatVersion, false);
		_isRunAheadFrame = false;
	}
}

void Emulator::OnBeforeSendFrame()
{
	if(!_isRunAheadFrame) {
		if(_audioPlayerHud) {
			_audioPlayerHud->Draw();
		}

		if(_stats && _settings->GetPreferences().ShowDebugInfo) {
			double lastFrameTime = _lastFrameTimer.GetElapsedMS();
			_lastFrameTimer.Reset();
			_stats->DisplayStats(this, lastFrameTime);
		}
	}
}

void Emulator::ProcessEndOfFrame()
{
	if(!_isRunAheadFrame) {
		_frameLimiter->ProcessFrame();
		while(_frameLimiter->WaitForNextFrame()) {
			if(_stopFlag || _frameDelay != GetFrameDelay() || _paused || _pauseOnNextFrame || _lockCounter > 0) {
				//Need to process another event, stop sleeping
				break;
			}
		}

		double newFrameDelay = GetFrameDelay();
		if(newFrameDelay != _frameDelay) {
			_frameDelay = newFrameDelay;
			_frameLimiter->SetDelay(_frameDelay);
		}
	}
	_frameRunning = false;
}

void Emulator::Stop(bool sendNotification, bool preventRecentGameSave, bool saveBattery)
{
	BlockDebuggerRequests();

	_stopFlag = true;

	_notificationManager->SendNotification(ConsoleNotificationType::BeforeGameUnload);

	ResetDebugger();

	if(_emuThread) {
		_emuThread->join();
		_emuThread.release();
	}

	if(!preventRecentGameSave && _console && !_settings->GetPreferences().DisableGameSelectionScreen && !_audioPlayerHud) {
		RomInfo romInfo = GetRomInfo();
		_saveStateManager->SaveRecentGame(romInfo.RomFile.GetFileName(), romInfo.RomFile, romInfo.PatchFile);
	}

	if(sendNotification) {
		_notificationManager->SendNotification(ConsoleNotificationType::BeforeEmulationStop);
	}

	_videoDecoder->StopThread();
	_videoRenderer->StopThread();
	_rewindManager.reset();

	if(_console) {
		if(saveBattery) {
			//Only save battery on power off, otherwise SaveBattery() is called by LoadRom()
			_console->SaveBattery();
		}
		_console.reset();
	}

	_soundMixer->StopAudio(true);

	if(sendNotification) {
		_notificationManager->SendNotification(ConsoleNotificationType::EmulationStopped);
	}
}

void Emulator::Reset()
{
	Lock();

	_console->Reset();
	GetControlManager()->UpdateInputState();

	_videoRenderer->ClearFrame();

	_notificationManager->SendNotification(ConsoleNotificationType::GameReset);
	ProcessEvent(EventType::Reset);

	Unlock();
}

void Emulator::ReloadRom(bool forPowerCycle)
{
	RomInfo info = GetRomInfo();
	LoadRom(info.RomFile, info.PatchFile, false, forPowerCycle);
}

void Emulator::PowerCycle()
{
	ReloadRom(true);
}

bool Emulator::LoadRom(VirtualFile romFile, VirtualFile patchFile, bool stopRom, bool forPowerCycle)
{
	if(GetEmulationThreadId() == std::this_thread::get_id()) {
		_threadPaused = true;
	}

	BlockDebuggerRequests();

	auto emuLock = AcquireLock();
	auto dbgLock = _debuggerLock.AcquireSafe();
	auto lock = _loadLock.AcquireSafe();

	//Once emulation is stopped, warn the UI that a game is about to be loaded
	//This allows the UI to finish processing pending calls to the debug tools, etc.
	_notificationManager->SendNotification(ConsoleNotificationType::BeforeGameLoad);
	
	if(!romFile.IsValid()) {
		return false;
	}

	bool wasPaused = IsPaused();

	//Keep a reference to the original debugger
	shared_ptr<Debugger> debugger = _debugger.lock();
	bool debuggerActive = debugger != nullptr;
	
	//Unset _debugger to ensure nothing calls the debugger while initializing the new rom
	ResetDebugger();

	if(patchFile.IsValid()) {
		if(romFile.ApplyPatch(patchFile)) {
			MessageManager::DisplayMessage("Patch", "ApplyingPatch", patchFile.GetFileName());
		}
	}

	if(_console) {
		//Make sure the battery is saved to disk before we load another game (or reload the same game)
		_console->SaveBattery();
	}
	
	unique_ptr<IConsole> console;
	LoadRomResult result = LoadRomResult::UnknownType;
	TryLoadRom<NesConsole>(romFile, result, console);
	TryLoadRom<SnesConsole>(romFile, result, console);
	TryLoadRom<Gameboy>(romFile, result, console);
	TryLoadRom<PceConsole>(romFile, result, console);
	
	if(result != LoadRomResult::Success) {
		_notificationManager->SendNotification(ConsoleNotificationType::GameLoadFailed);
		MessageManager::DisplayMessage("Error", "CouldNotLoadFile", romFile.GetFileName());
		if(debugger) {
			_debugger.reset(debugger);
			debugger->ResetSuspendCounter();
		}
		_allowDebuggerRequest = true;
		return false;
	}

	//Cleanup debugger instance if one was active
	if(debugger) {
		debugger->Release();
		debugger.reset();
	}

	if(stopRom) {
		//Only update the recent game entry if the game that was loaded is a different game
		bool gameChanged = (string)_rom.RomFile != (string)romFile || (string)_rom.PatchFile != (string)patchFile;
		Stop(false, !gameChanged, false);
		//TODO PERF
		//KeyManager::UpdateDevices();
	}

	_videoDecoder->StopThread();
	_videoRenderer->StopThread();

	//Cast VirtualFiles to string to ensure the original file data isn't kept in memory
	_rom.RomFile = (string)romFile;
	_rom.PatchFile = (string)patchFile;
	_rom.Format = console->GetRomFormat();

	if(_rom.Format == RomFormat::Spc || _rom.Format == RomFormat::Nsf || _rom.Format == RomFormat::Gbs) {
		_audioPlayerHud.reset(new AudioPlayerHud(this));
	} else {
		_audioPlayerHud.reset();
	}

	_cheatManager->ClearCheats(false);

	uint32_t pollCounter = 0;
	if(forPowerCycle && GetControlManager()) {
		//When power cycling, poll counter must be preserved to allow movies to playback properly
		pollCounter = GetControlManager()->GetPollCounter();
	}

	_console.swap(console);

	//Restore pollcounter (used by movies when a power cycle is in the movie)
	GetControlManager()->SetPollCounter(pollCounter);

	if(debuggerActive) {
		InitDebugger();
	}

	_rewindManager.reset(new RewindManager(this));
	_notificationManager->RegisterNotificationListener(_rewindManager);

	GetControlManager()->UpdateControlDevices();
	GetControlManager()->UpdateInputState();

	_autoSaveStateFrameCounter = 0;

	//Mark the thread as paused, and release the debugger lock to avoid
	//deadlocks with DebugBreakHelper if GameLoaded event starts the debugger
	_allowDebuggerRequest = true;
	dbgLock.Release();
	
	_threadPaused = true;
	if(wasPaused && _debugger) {
		//Break on the current instruction if emulation was already paused
		//(must be done after setting _threadPaused to true)
		_debugger->Step(GetCpuTypes()[0], 1, StepType::Step);
	}
	_notificationManager->SendNotification(ConsoleNotificationType::GameLoaded, (void*)forPowerCycle);
	_threadPaused = false;

	if(!forPowerCycle && !_audioPlayerHud) {
		string modelName = _console->GetRegion() == ConsoleRegion::Pal ? "PAL" : "NTSC";
		MessageManager::DisplayMessage(modelName, FolderUtilities::GetFilename(GetRomInfo().RomFile.GetFileName(), false));
	}

	_videoDecoder->StartThread();
	_videoRenderer->StartThread();

	if(stopRom) {
		_stopFlag = false;
		_emuThread.reset(new thread(&Emulator::Run, this));
	}

	return true;
}

template<typename T>
void Emulator::TryLoadRom(VirtualFile& romFile, LoadRomResult& result, unique_ptr<IConsole>& console)
{
	if(result == LoadRomResult::UnknownType) {
		string romExt = romFile.GetFileExtension();
		vector<string> extensions = T::GetSupportedExtensions();
		if(std::find(extensions.begin(), extensions.end(), romExt) != extensions.end()) {
			//Keep a copy of the current state of _consoleMemory
			ConsoleMemoryInfo consoleMemory[(int)MemoryType::Register + 1] = {};
			memcpy(consoleMemory, _consoleMemory, sizeof(_consoleMemory));

			//Attempt to load rom with specified core
			memset(_consoleMemory, 0, sizeof(_consoleMemory));

			//Change filename for batterymanager to allow loading the correct files
			_batteryManager->Initialize(FolderUtilities::GetFilename(romFile.GetFileName(), false));

			console.reset(new T(this));
			result = console->LoadRom(romFile);

			if(result != LoadRomResult::Success) {
				//Restore state if load fails
				memcpy(_consoleMemory, consoleMemory, sizeof(_consoleMemory));
				_batteryManager->Initialize(FolderUtilities::GetFilename(_rom.RomFile.GetFileName(), false));
			}
		}
	}
}

RomInfo& Emulator::GetRomInfo()
{
	return _rom;
}

string Emulator::GetHash(HashType type)
{
	//TODO
	if(type == HashType::Sha1) {
		return _rom.RomFile.GetSha1Hash();
	}
	return "";
}

uint32_t Emulator::GetCrc32()
{
	//TODO
	return 0;
}

PpuFrameInfo Emulator::GetPpuFrame()
{
	if(_console) {
		return _console->GetPpuFrame();
	} else {
		return {};
	}
}

ConsoleRegion Emulator::GetRegion()
{
	//TODO is this really useful?
	if(_console) {
		return _console->GetRegion();
	} else {
		return ConsoleRegion::Ntsc;
	}
}

IConsole* Emulator::GetConsole()
{
	return _console.get();
}

ConsoleType Emulator::GetConsoleType()
{
	if(_console) {
		return _console->GetConsoleType();
	} else {
		return ConsoleType::Snes;
	}
}

vector<CpuType> Emulator::GetCpuTypes()
{
	if(_console) {
		return _console->GetCpuTypes();
	} else {
		return {};
	}
}

//TODO: add cputype param to allow getting GB timing info from UI in SGB mode
TimingInfo Emulator::GetTimingInfo()
{
	TimingInfo info = {};
	if(_console) {
		info.MasterClock = GetMasterClock();
		info.MasterClockRate = GetMasterClockRate();
		info.FrameCount = GetFrameCount();
		info.Fps = GetFps();

		PpuFrameInfo frameInfo = GetPpuFrame();
		info.CycleCount = frameInfo.CycleCount;
		info.ScanlineCount = frameInfo.ScanlineCount;
		info.FirstScanline = frameInfo.FirstScanline;
	}
	return info;
}

uint64_t Emulator::GetMasterClock()
{
	return _console->GetMasterClock();
}

uint32_t Emulator::GetMasterClockRate()
{
	//TODO this is not accurate when overclocking options are turned on
	return _console->GetMasterClockRate();
}

uint32_t Emulator::GetFrameCount()
{
	if(_console) {
		return _console->GetPpuFrame().FrameCount;
	} else {
		return 0;
	}
}

double Emulator::GetFps()
{
	return _console->GetFps();
}

double Emulator::GetFrameDelay()
{
	uint32_t emulationSpeed = _settings->GetEmulationSpeed();
	double frameDelay;
	if(emulationSpeed == 0) {
		frameDelay = 0;
	} else {
		double fps = GetFps();
		if(_settings->GetVideoConfig().IntegerFpsMode) {
			fps = std::round(fps);
		}
		frameDelay = 1000 / fps;
		frameDelay /= (emulationSpeed / 100.0);
	}
	return frameDelay;
}

void Emulator::PauseOnNextFrame()
{
	shared_ptr<Debugger> debugger = _debugger.lock();
	if(debugger) {
		switch(GetConsoleType()) {
			case ConsoleType::Snes:
				debugger->Step(CpuType::Snes, 240, StepType::SpecificScanline);
				break;

			case ConsoleType::Gameboy:
			case ConsoleType::GameboyColor:
				debugger->Step(CpuType::Gameboy, 144, StepType::SpecificScanline);
				break;

			case ConsoleType::Nes:
				debugger->Step(CpuType::Nes, 240, StepType::SpecificScanline);
				break;
		}
	} else {
		_pauseOnNextFrame = true;
		_paused = false;
	}
}

void Emulator::Pause()
{
	shared_ptr<Debugger> debugger = _debugger.lock();
	if(debugger) {
		debugger->Step(GetCpuTypes()[0], 1, StepType::Step);
	} else {
		_paused = true;
	}
}

void Emulator::Resume()
{
	shared_ptr<Debugger> debugger = _debugger.lock();
	if(debugger) {
		debugger->Run();
	} else {
		_paused = false;
	}
}

bool Emulator::IsPaused()
{
	shared_ptr<Debugger> debugger = _debugger.lock();
	if(debugger) {
		return debugger->IsPaused();
	} else {
		return _paused;
	}
}

void Emulator::WaitForPauseEnd()
{
	_notificationManager->SendNotification(ConsoleNotificationType::GamePaused);

	//Prevent audio from looping endlessly while game is paused
	_soundMixer->StopAudio();
	_runLock.Release();

	PlatformUtilities::EnableScreensaver();
	PlatformUtilities::RestoreTimerResolution();

	while(_paused && !_stopFlag && !_debugger) {
		//Sleep until emulation is resumed
		std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(30));

		if(_systemActionManager->IsResetPending()) {
			//Reset/power cycle was pressed, stop waiting and process it now
			break;
		}
	}

	PlatformUtilities::DisableScreensaver();
	PlatformUtilities::EnableHighResolutionTimer();

	if(!_stopFlag) {
		_runLock.Acquire();
		_notificationManager->SendNotification(ConsoleNotificationType::GameResumed);
	}
}

EmulatorLock Emulator::AcquireLock()
{
	return EmulatorLock(this);
}

void Emulator::Lock()
{
	SuspendDebugger(false);
	_lockCounter++;
	_runLock.Acquire();
}

void Emulator::Unlock()
{
	SuspendDebugger(true);
	_runLock.Release();
	_lockCounter--;
}

bool Emulator::IsThreadPaused()
{
	return !_emuThread || _threadPaused;
}

void Emulator::SuspendDebugger(bool release)
{
	shared_ptr<Debugger> debugger = _debugger.lock();
	if(debugger) {
		debugger->SuspendDebugger(release);
	}
}

void Emulator::WaitForLock()
{
	if(_lockCounter > 0) {
		//Need to temporarely pause the emu (to save/load a state, etc.)
		_runLock.Release();

		_threadPaused = true;

		//Spin wait until we are allowed to start again
		while(_lockCounter > 0 && !_stopFlag) {}

		shared_ptr<Debugger> debugger = _debugger.lock();
		if(debugger) {
			while(debugger->HasBreakRequest() && !_stopFlag) {}
		}

		if(!_stopFlag) {
			_threadPaused = false;

			_runLock.Acquire();
		}
	}
}

void Emulator::Serialize(ostream& out, bool includeSettings, int compressionLevel)
{
	Serializer s(SaveStateManager::FileFormatVersion, true);
	if(includeSettings) {
		SV(_settings);
	}
	s.Stream(_console, "");
	s.SaveTo(out, compressionLevel);
}

bool Emulator::Deserialize(istream& in, uint32_t fileFormatVersion, bool includeSettings)
{
	Serializer s(fileFormatVersion, false);
	if(!s.LoadFrom(in)) {
		return false;
	}

	if(includeSettings) {
		SV(_settings);
	}
	s.Stream(_console, "");
	_notificationManager->SendNotification(ConsoleNotificationType::StateLoaded);
	return true;
}

SoundMixer* Emulator::GetSoundMixer()
{
	return _soundMixer.get();
}

VideoRenderer* Emulator::GetVideoRenderer()
{
	return _videoRenderer.get();
}

VideoDecoder* Emulator::GetVideoDecoder()
{
	return _videoDecoder.get();
}

ShortcutKeyHandler* Emulator::GetShortcutKeyHandler()
{
	return _shortcutKeyHandler.get();
}

NotificationManager* Emulator::GetNotificationManager()
{
	return _notificationManager.get();
}

EmuSettings* Emulator::GetSettings()
{
	return _settings.get();
}

SaveStateManager* Emulator::GetSaveStateManager()
{
	return _saveStateManager.get();
}

RewindManager* Emulator::GetRewindManager()
{
	return _rewindManager.get();
}

DebugHud* Emulator::GetDebugHud()
{
	return _debugHud.get();
}

BatteryManager* Emulator::GetBatteryManager()
{
	return _batteryManager.get();
}

CheatManager* Emulator::GetCheatManager()
{
	return _cheatManager.get();
}

MovieManager* Emulator::GetMovieManager()
{
	return _movieManager.get();
}

GameServer* Emulator::GetGameServer()
{
	return _gameServer.get();
}

GameClient* Emulator::GetGameClient()
{
	return _gameClient.get();
}

shared_ptr<SystemActionManager> Emulator::GetSystemActionManager()
{
	return _systemActionManager;
}

BaseControlManager* Emulator::GetControlManager()
{
	if(_console) {
		return _console->GetControlManager();
	} else {
		return nullptr;
	}
}

BaseVideoFilter* Emulator::GetVideoFilter()
{
	if(_console) {
		return _console->GetVideoFilter();
	} else {
		return new SnesDefaultVideoFilter(this);
	}
}

void Emulator::InputBarcode(uint64_t barcode, uint32_t digitCount)
{
	if(_console) {
		shared_ptr<IBarcodeReader> reader = GetControlManager()->GetControlDevice<IBarcodeReader>();
		if(reader) {
			reader->InputBarcode(barcode, digitCount);
		}
	}
}

void Emulator::ProcessTapeRecorderAction(TapeRecorderAction action, string filename)
{
	if(_console) {
		shared_ptr<ITapeRecorder> recorder = GetControlManager()->GetControlDevice<ITapeRecorder>();
		if(recorder) {
			recorder->ProcessTapeRecorderAction(action, filename);
		}
	}
}

ShortcutState Emulator::IsShortcutAllowed(EmulatorShortcut shortcut, uint32_t shortcutParam)
{
	if(_console) {
		return _console->IsShortcutAllowed(shortcut, shortcutParam);
	}

	return ShortcutState::Default;
}

bool Emulator::IsKeyboardConnected()
{
	if(_console) {
		return _console->GetControlManager()->IsKeyboardConnected();
	}
	return false;
}

void Emulator::BlockDebuggerRequests()
{
	//Block all new debugger calls
	auto lock = _debuggerLock.AcquireSafe();
	_allowDebuggerRequest = false;
	while(_debugRequestCount > 0) {
		//Wait until debugger calls are all done
		std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(10));
	}
}

Emulator::DebuggerRequest Emulator::GetDebugger(bool autoInit)
{
	if(IsRunning() && _allowDebuggerRequest) {
		auto lock = _debuggerLock.AcquireSafe();
		if(IsRunning() && _allowDebuggerRequest) {
			if(!_debugger && autoInit) {
				InitDebugger();
			}
			return DebuggerRequest(this);
		}
	}
	return DebuggerRequest(nullptr);
}

void Emulator::ResetDebugger(bool startDebugger)
{
	shared_ptr<Debugger> currentDbg = _debugger.lock();
	if(currentDbg) {
		currentDbg->SuspendDebugger(false);
	}

	if(_emulationThreadId == std::this_thread::get_id()) {
		_debugger.reset(startDebugger ? new Debugger(this, _console.get()) : nullptr);
	} else {
		//Need to pause emulator to change _debugger (when not called from the emulation thread)
		auto emuLock = AcquireLock();
		_debugger.reset(startDebugger ? new Debugger(this, _console.get()) : nullptr);
	}
}

void Emulator::InitDebugger()
{
	if(!_debugger) {
		//Lock to make sure we don't try to start debuggers in 2 separate threads at once
		auto lock = _debuggerLock.AcquireSafe();
		if(!_debugger) {
			BlockDebuggerRequests();
			ResetDebugger(true);
			_allowDebuggerRequest = true;
		}
	}
}

void Emulator::StopDebugger()
{
	//Pause/unpause the regular emulation thread based on the debugger's pause state
	_paused = IsPaused();

	if(_debugger) {
		auto lock = _debuggerLock.AcquireSafe();
		if(_debugger) {
			BlockDebuggerRequests();
			ResetDebugger();
			_allowDebuggerRequest = true;
		}
	}
}

bool Emulator::IsDebugging()
{
	return !!_debugger;
}

thread::id Emulator::GetEmulationThreadId()
{
	return _emulationThreadId;
}

void Emulator::RegisterMemory(MemoryType type, void* memory, uint32_t size)
{
	_consoleMemory[(int)type] = { memory, size };
}

ConsoleMemoryInfo Emulator::GetMemory(MemoryType type)
{
	return _consoleMemory[(int)type];
}

AudioTrackInfo Emulator::GetAudioTrackInfo()
{
	AudioTrackInfo track = _console->GetAudioTrackInfo();
	AudioConfig audioCfg = _settings->GetAudioConfig();
	if(track.Length <= 0 && audioCfg.AudioPlayerEnableTrackLength) {
		track.Length = audioCfg.AudioPlayerTrackLength;
		track.FadeLength = 1;
	}
	return track;
}

void Emulator::ProcessAudioPlayerAction(AudioPlayerActionParams p)
{
	return _console->ProcessAudioPlayerAction(p);
}

AudioPlayerHud* Emulator::GetAudioPlayerHud()
{
	return _audioPlayerHud.get();
}

bool Emulator::IsRunning()
{
	return _console != nullptr;
}

bool Emulator::IsRunAheadFrame()
{
	return _isRunAheadFrame;
}

template<CpuType type>
void Emulator::ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi)
{
	if(_debugger) {
		_debugger->ProcessInterrupt<type>(originalPc, currentPc, forNmi);
	}
}

void Emulator::ProcessEvent(EventType type)
{
	if(_debugger) {
		_debugger->ProcessEvent(type);
	}
}

template<CpuType cpuType>
void Emulator::AddDebugEvent(DebugEventType evtType)
{
	if(_debugger) {
		_debugger->GetEventManager(cpuType)->AddEvent(evtType);
	}
}

void Emulator::BreakIfDebugging(CpuType sourceCpu, BreakSource source)
{
	if(_debugger) {
		_debugger->BreakImmediately(sourceCpu, source);
	}
}

template void Emulator::ProcessInterrupt<CpuType::Snes>(uint32_t originalPc, uint32_t currentPc, bool forNmi);
template void Emulator::ProcessInterrupt<CpuType::Sa1>(uint32_t originalPc, uint32_t currentPc, bool forNmi);
template void Emulator::ProcessInterrupt<CpuType::Gameboy>(uint32_t originalPc, uint32_t currentPc, bool forNmi);
template void Emulator::ProcessInterrupt<CpuType::Nes>(uint32_t originalPc, uint32_t currentPc, bool forNmi);
template void Emulator::ProcessInterrupt<CpuType::Pce>(uint32_t originalPc, uint32_t currentPc, bool forNmi);

template void Emulator::AddDebugEvent<CpuType::Snes>(DebugEventType evtType);
template void Emulator::AddDebugEvent<CpuType::Gameboy>(DebugEventType evtType);
template void Emulator::AddDebugEvent<CpuType::Nes>(DebugEventType evtType);
template void Emulator::AddDebugEvent<CpuType::Pce>(DebugEventType evtType);
