#include "pch.h"
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
#include "Shared/DebuggerRequest.h"
#include "Shared/Movies/MovieManager.h"
#include "Shared/BatteryManager.h"
#include "Shared/CheatManager.h"
#include "Shared/SystemActionManager.h"
#include "Shared/TimingInfo.h"
#include "Shared/HistoryViewer.h"
#include "Netplay/GameServer.h"
#include "Netplay/GameClient.h"
#include "Shared/Interfaces/IConsole.h"
#include "Shared/Interfaces/IBarcodeReader.h"
#include "Shared/Interfaces/ITapeRecorder.h"
#include "Shared/BaseControlManager.h"
#include "SNES/SnesConsole.h"
#include "SNES/SnesDefaultVideoFilter.h"
#include "NES/NesConsole.h"
#include "Gameboy/Gameboy.h"
#include "PCE/PceConsole.h"
#include "SMS/SmsConsole.h"
#include "GBA/GbaConsole.h"
#include "WS/WsConsole.h"
#include "Debugger/Debugger.h"
#include "Debugger/BaseEventManager.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/DebugUtilities.h"
#include "Utilities/Serializer.h"
#include "Utilities/Timer.h"
#include "Utilities/VirtualFile.h"
#include "Utilities/PlatformUtilities.h"
#include "Utilities/FolderUtilities.h"
#include "Shared/MemoryOperationType.h"
#include "Shared/EventType.h"

Emulator::Emulator() :
	_settings(new EmuSettings(this)),
	_debugHud(new DebugHud()),
	_scriptHud(new DebugHud()),
	_notificationManager(new NotificationManager()),
	_batteryManager(new BatteryManager()),
	_soundMixer(new SoundMixer(this)),
	_videoRenderer(new VideoRenderer(this)),
	_videoDecoder(new VideoDecoder(this)),
	_saveStateManager(new SaveStateManager(this)),
	_cheatManager(new CheatManager(this)),
	_movieManager(new MovieManager(this)),
	_historyViewer(new HistoryViewer(this)),
	_gameServer(new GameServer(this)),
	_gameClient(new GameClient(this)),
	_rewindManager(new RewindManager(this))
{
	_paused = false;
	_pauseOnNextFrame = false;
	_stopFlag = false;
	_isRunAheadFrame = false;
	_lockCounter = 0;
	_threadPaused = false;

	_debugRequestCount = 0;
	_blockDebuggerRequestCount = 0;

	_videoDecoder->Init();
}

Emulator::~Emulator()
{
}

void Emulator::Initialize(bool enableShortcuts)
{
	_systemActionManager.reset(new SystemActionManager(this));
	if(enableShortcuts) {
		_shortcutKeyHandler.reset(new ShortcutKeyHandler(this));
		_notificationManager->RegisterNotificationListener(_shortcutKeyHandler);
	}

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
	PlatformUtilities::DisableScreensaver();

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
			_historyViewer->ProcessEndOfFrame();
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

	_emulationThreadId = thread::id();

	if(_runLock.IsLockedByCurrentThread()) {
		//Lock might not be held by current frame is _stopFlag was set to interrupt the thread
		_runLock.Release();
	}

	PlatformUtilities::EnableScreensaver();
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
	_historyViewer->ProcessEndOfFrame();

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
			_audioPlayerHud->Draw(GetFrameCount(), GetFps());
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

		_console->GetControlManager()->ProcessEndOfFrame();
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

	if(_console && saveBattery) {
		//Only save battery on power off, otherwise SaveBattery() is called by LoadRom()
		_console->SaveBattery();
	}

	if(!preventRecentGameSave && _console && !_settings->GetPreferences().DisableGameSelectionScreen && !_audioPlayerHud) {
		RomInfo romInfo = GetRomInfo();
		_saveStateManager->SaveRecentGame(romInfo.RomFile.GetFileName(), romInfo.RomFile, romInfo.PatchFile);
	}

	if(sendNotification) {
		_notificationManager->SendNotification(ConsoleNotificationType::BeforeEmulationStop);
	}

	_movieManager->Stop();
	_videoDecoder->StopThread();
	_rewindManager->Reset();

	if(_console) {
		_console.reset();
	}

	OnBeforePause(true);

	if(sendNotification) {
		_notificationManager->SendNotification(ConsoleNotificationType::EmulationStopped);
	}

	_blockDebuggerRequestCount--;
}

void Emulator::Reset()
{
	Lock();

	_console->Reset();

	//Ensure reset button flag is off before recording input for first frame
	_systemActionManager->ResetState();

	_console->GetControlManager()->UpdateInputState();
	_console->GetControlManager()->ResetLagCounter();

	_videoRenderer->ClearFrame();

	_notificationManager->SendNotification(ConsoleNotificationType::GameReset);
	ProcessEvent(EventType::Reset);

	Unlock();
}

void Emulator::ReloadRom(bool forPowerCycle)
{
	RomInfo info = GetRomInfo();
	
	//Cast RomFile/PatchFile to string to make sure the file is reloaded from the disk
	//In some scenarios, the file might be in memory already, which will prevent the reload
	//from actually reloading the rom from the disk.
	if(!LoadRom((string)info.RomFile, (string)info.PatchFile, !forPowerCycle, forPowerCycle)) {
		if(forPowerCycle) {
			//Power cycle failed (rom not longer exists, etc.), reset flag
			//(otherwise power cycle will continue to be attempted on each frame)
			_systemActionManager->ResetState();

			//Unsuspend debugger, otherwise deadlocks can occur after a power cycle fails when the debugger is active
			SuspendDebugger(true);
		}
	}
}

void Emulator::PowerCycle()
{
	ReloadRom(true);
}

bool Emulator::LoadRom(VirtualFile romFile, VirtualFile patchFile, bool stopRom, bool forPowerCycle)
{
	bool result = false;
	try {
		result = InternalLoadRom(romFile, patchFile, stopRom, forPowerCycle);
	} catch(std::exception& ex) {
		_videoDecoder->StartThread();
		_videoRenderer->StartThread();

		MessageManager::DisplayMessage("Error", "UnexpectedError", ex.what());
		Stop(false, true, false);
	}

	if(!result) {
		_notificationManager->SendNotification(ConsoleNotificationType::GameLoadFailed);
	}

	return result;
}

bool Emulator::InternalLoadRom(VirtualFile romFile, VirtualFile patchFile, bool stopRom, bool forPowerCycle)
{
	if(!romFile.IsValid()) {
		MessageManager::DisplayMessage("Error", "CouldNotLoadFile", romFile.GetFileName());
		return false;
	}

	if(IsEmulationThread()) {
		_threadPaused = true;
	}

	BlockDebuggerRequests();

	auto emuLock = AcquireLock();
	auto dbgLock = _debuggerLock.AcquireSafe();
	auto lock = _loadLock.AcquireSafe();

	//Once emulation is stopped, warn the UI that a game is about to be loaded
	//This allows the UI to finish processing pending calls to the debug tools, etc.
	_notificationManager->SendNotification(ConsoleNotificationType::BeforeGameLoad);
	
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

	_soundMixer->StopAudio();

	if(!forPowerCycle) {
		_movieManager->Stop();
	}

	//Keep copy of current memory types, to allow keeping ROM changes when power cycling
	ConsoleMemoryInfo originalConsoleMemory[DebugUtilities::GetMemoryTypeCount()] = {};
	memcpy(originalConsoleMemory, _consoleMemory, sizeof(_consoleMemory));

	unique_ptr<IConsole> console;
	LoadRomResult result = LoadRomResult::UnknownType;

	//Try loading the rom, give priority to file extension, then trying to check for file signatures if extension is unknown
	TryLoadRom(romFile, result, console, false);
	TryLoadRom(romFile, result, console, true);
	
	if(result != LoadRomResult::Success) {
		MessageManager::DisplayMessage("Error", "CouldNotLoadFile", romFile.GetFileName());
		if(debugger) {
			_debugger.reset(debugger);
			debugger->ResetSuspendCounter();
		}
		_blockDebuggerRequestCount--;
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
		memset(originalConsoleMemory, 0, sizeof(originalConsoleMemory));
	}

	_videoDecoder->StopThread();
	_videoRenderer->StopThread();

	//Cast VirtualFiles to string to ensure the original file data isn't kept in memory
	_rom.RomFile = (string)romFile;
	_rom.PatchFile = (string)patchFile;
	_rom.Format = console->GetRomFormat();
	_rom.DipSwitches = console->GetDipSwitchInfo();

	if(_rom.Format == RomFormat::Spc || _rom.Format == RomFormat::Nsf || _rom.Format == RomFormat::Gbs || _rom.Format == RomFormat::PceHes) {
		_audioPlayerHud.reset(new AudioPlayerHud(this));
	} else {
		_audioPlayerHud.reset();
	}

	_cheatManager->ClearCheats(false);

	uint32_t pollCounter = 0;
	if(forPowerCycle && console->GetControlManager()) {
		//When power cycling, poll counter must be preserved to allow movies to playback properly
		pollCounter = console->GetControlManager()->GetPollCounter();
	}

	InitConsole(console, originalConsoleMemory, forPowerCycle);

	//Restore pollcounter (used by movies when a power cycle is in the movie)
	_console->GetControlManager()->SetPollCounter(pollCounter);

	_rewindManager->InitHistory();

	if(debuggerActive || _settings->CheckFlag(EmulationFlags::ConsoleMode)) {
		InitDebugger();
	}

	_notificationManager->RegisterNotificationListener(_rewindManager);

	//Ensure power cycle flag is off before recording input for first frame
	_systemActionManager->ResetState();

	_console->GetControlManager()->UpdateControlDevices();
	_console->GetControlManager()->UpdateInputState();

	_autoSaveStateFrameCounter = 0;

	//Mark the thread as paused, and release the debugger lock to avoid
	//deadlocks with DebugBreakHelper if GameLoaded event starts the debugger
	_blockDebuggerRequestCount--;
	dbgLock.Release();
	
	_threadPaused = true;
	bool needPause = wasPaused && _debugger;
	if(needPause) {
		//Break on the current instruction if emulation was already paused
		//(must be done after setting _threadPaused to true)
		_debugger->Step(GetCpuTypes()[0], 1, StepType::Step, BreakSource::Pause);
	}
	
	GameLoadedEventParams params = { needPause, forPowerCycle };
	_notificationManager->SendNotification(ConsoleNotificationType::GameLoaded, &params);
	_threadPaused = false;

	if(!forPowerCycle && !_audioPlayerHud) {
		ConsoleRegion region = _console->GetRegion();
		string modelName = region == ConsoleRegion::Pal ? "PAL" : (region == ConsoleRegion::Dendy ? "Dendy" : "NTSC");
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

void Emulator::InitConsole(unique_ptr<IConsole>& newConsole, ConsoleMemoryInfo originalConsoleMemory[], bool preserveRom)
{
	if(preserveRom && _console) {
		//When power cycling, copy over the content of any ROM memory from the previous instance
		magic_enum::enum_for_each<MemoryType>([&](MemoryType memType) {
			if(DebugUtilities::IsRom(memType)) {
				uint32_t orgSize = originalConsoleMemory[(int)memType].Size;
				if(orgSize > 0 && GetMemory(memType).Size == orgSize) {
					memcpy(_consoleMemory[(int)memType].Memory, originalConsoleMemory[(int)memType].Memory, orgSize);
				}
			}
		});
	}

	_console.reset(newConsole);
	_consoleType = _console->GetConsoleType();
	_notificationManager->RegisterNotificationListener(_console.lock());
}

void Emulator::TryLoadRom(VirtualFile& romFile, LoadRomResult& result, unique_ptr<IConsole>& console, bool useFileSignature)
{
	TryLoadRom<NesConsole>(romFile, result, console, useFileSignature);
	TryLoadRom<SnesConsole>(romFile, result, console, useFileSignature);
	TryLoadRom<Gameboy>(romFile, result, console, useFileSignature);
	TryLoadRom<PceConsole>(romFile, result, console, useFileSignature);
	TryLoadRom<SmsConsole>(romFile, result, console, useFileSignature);
	TryLoadRom<GbaConsole>(romFile, result, console, useFileSignature);
	TryLoadRom<WsConsole>(romFile, result, console, useFileSignature);
}

template<typename T>
void Emulator::TryLoadRom(VirtualFile& romFile, LoadRomResult& result, unique_ptr<IConsole>& console, bool useFileSignature)
{
	if(result == LoadRomResult::UnknownType) {
		string romExt = romFile.GetFileExtension();
		vector<string> extensions = T::GetSupportedExtensions();
		if(std::find(extensions.begin(), extensions.end(), romExt) != extensions.end() || (useFileSignature && romFile.CheckFileSignature(T::GetSupportedSignatures()))) {
			//Keep a copy of the current state of _consoleMemory
			ConsoleMemoryInfo consoleMemory[DebugUtilities::GetMemoryTypeCount()] = {};
			memcpy(consoleMemory, _consoleMemory, sizeof(_consoleMemory));

			//Attempt to load rom with specified core
			memset(_consoleMemory, 0, sizeof(_consoleMemory));

			//Change filename for batterymanager to allow loading the correct files
			bool hasBattery = _batteryManager->HasBattery();
			_batteryManager->Initialize(FolderUtilities::GetFilename(romFile.GetFileName(), false));

			console.reset(new T(this));
			result = console->LoadRom(romFile);

			if(result != LoadRomResult::Success) {
				//Restore state if load fails
				memcpy(_consoleMemory, consoleMemory, sizeof(_consoleMemory));
				_batteryManager->Initialize(FolderUtilities::GetFilename(_rom.RomFile.GetFileName(), false), hasBattery);
			}
		}
	}
}

string Emulator::GetHash(HashType type)
{
	shared_ptr<IConsole> console = _console.lock();
	string hash = console->GetHash(type);
	if(hash.size()) {
		return hash;
	} else if(type == HashType::Sha1) {
		return _rom.RomFile.GetSha1Hash();
	} else if(type == HashType::Sha1Cheat) {
		return _rom.RomFile.GetSha1Hash();
	}
	return "";
}

uint32_t Emulator::GetCrc32()
{
	return _rom.RomFile.GetCrc32();
}

PpuFrameInfo Emulator::GetPpuFrame()
{
	shared_ptr<IConsole> console = GetConsole();
	return console ? console->GetPpuFrame() : PpuFrameInfo {};
}

ConsoleRegion Emulator::GetRegion()
{
	shared_ptr<IConsole> console = GetConsole();
	return console ? console->GetRegion() : ConsoleRegion::Ntsc;
}

shared_ptr<IConsole> Emulator::GetConsole()
{
	return _console.lock();
}

IConsole* Emulator::GetConsoleUnsafe()
{
#ifdef _DEBUG
	if(!IsEmulationThread()) {
		throw std::runtime_error("GetConsoleUnsafe should only be called from the emulation thread");
	}
#endif
	return _console.get();
}

ConsoleType Emulator::GetConsoleType()
{
	return _consoleType;
}

vector<CpuType> Emulator::GetCpuTypes()
{
	shared_ptr<IConsole> console = GetConsole();
	return console ? console->GetCpuTypes() : vector<CpuType>{};
}

TimingInfo Emulator::GetTimingInfo(CpuType cpuType)
{
	shared_ptr<IConsole> console = GetConsole();
	return console ? console->GetTimingInfo(cpuType) : TimingInfo {};
}

uint64_t Emulator::GetMasterClock()
{
#if DEBUG
	if(!IsEmulationThread()) {
		throw std::runtime_error("called on wrong thread");
	}
#endif

	return _console->GetMasterClock();
}

uint32_t Emulator::GetMasterClockRate()
{
#if DEBUG
	if(!IsEmulationThread()) {
		throw std::runtime_error("called on wrong thread");
	}
#endif

	//TODOv2 this is not accurate when overclocking options are turned on
	return _console->GetMasterClockRate();
}

uint32_t Emulator::GetFrameCount()
{
	return GetPpuFrame().FrameCount;
}

uint32_t Emulator::GetLagCounter()
{
	shared_ptr<IConsole> console = GetConsole();
	return console ? console->GetControlManager()->GetLagCounter() : 0;
}

void Emulator::ResetLagCounter()
{
	shared_ptr<IConsole> console = GetConsole();
	if(console) {
		console->GetControlManager()->ResetLagCounter();
	}
}

bool Emulator::HasControlDevice(ControllerType type)
{
	shared_ptr<IConsole> console = GetConsole();
	return console ? console->GetControlManager()->HasControlDevice(type) : false;
}

void Emulator::RegisterInputRecorder(IInputRecorder* recorder)
{
	shared_ptr<IConsole> console = GetConsole();
	if(console) {
		console->GetControlManager()->RegisterInputRecorder(recorder);
	}
}

void Emulator::UnregisterInputRecorder(IInputRecorder* recorder)
{
	shared_ptr<IConsole> console = GetConsole();
	if(console) {
		console->GetControlManager()->UnregisterInputRecorder(recorder);
	}
}

void Emulator::RegisterInputProvider(IInputProvider* provider)
{
	shared_ptr<IConsole> console = GetConsole();
	if(console) {
		console->GetControlManager()->RegisterInputProvider(provider);
	}
}

void Emulator::UnregisterInputProvider(IInputProvider* provider)
{
	shared_ptr<IConsole> console = GetConsole();
	if(console) {
		console->GetControlManager()->UnregisterInputProvider(provider);
	}
}

double Emulator::GetFps()
{
	shared_ptr<IConsole> console = GetConsole();
	double fps = console ? console->GetFps() : 60.0;
	if(_settings->GetVideoConfig().IntegerFpsMode) {
		fps = std::round(fps);
	}
	return fps;
}

double Emulator::GetFrameDelay()
{
	uint32_t emulationSpeed = _settings->GetEmulationSpeed();
	double frameDelay;
	if(emulationSpeed == 0) {
		frameDelay = 0;
	} else {
		frameDelay = 1000 / GetFps();
		frameDelay /= (emulationSpeed / 100.0);
	}
	return frameDelay;
}

void Emulator::PauseOnNextFrame()
{
	//Used by "Run single frame" shortcut
	shared_ptr<Debugger> debugger = _debugger.lock();
	if(debugger) {
		debugger->PauseOnNextFrame();
	} else {
		_pauseOnNextFrame = true;
		_paused = false;
	}
}

void Emulator::Pause()
{
	shared_ptr<Debugger> debugger = _debugger.lock();
	if(debugger) {
		debugger->Step(GetCpuTypes()[0], 1, StepType::Step, BreakSource::Pause);
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

void Emulator::OnBeforePause(bool clearAudioBuffer)
{
	//Prevent audio from looping endlessly while game is paused
	_soundMixer->StopAudio(clearAudioBuffer);

	//Stop force feedback
	KeyManager::SetForceFeedback(0);
}

void Emulator::WaitForPauseEnd()
{
	_notificationManager->SendNotification(ConsoleNotificationType::GamePaused);

	OnBeforePause(false);
	_runLock.Release();

	PlatformUtilities::EnableScreensaver();
	PlatformUtilities::RestoreTimerResolution();

	while(_paused && !_rewindManager->IsRewinding() && !_stopFlag && !_debugger) {
		//Sleep until emulation is resumed
		std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(30));

		if(_systemActionManager->IsResetPending()) {
			//Reset/power cycle was pressed, stop waiting and process it now
			break;
		}
	}

	PlatformUtilities::DisableScreensaver();
	PlatformUtilities::EnableHighResolutionTimer();

	while(!_stopFlag && !_runLock.TryAcquire(50)) { }

	if(!_stopFlag) {
		_notificationManager->SendNotification(ConsoleNotificationType::GameResumed);
	}
}

EmulatorLock Emulator::AcquireLock(bool allowDebuggerLock)
{
	//When allowDebuggerLock is true, the debugger is allowed to pause the emulation thread
	//instead of using the emulator's lock to pause the emulation at the end of the next frame
	//This is to allow, for example, loading or save a save state while the debugger tools are
	//opened without forcing the emulation to run for an entire frame each time.
	//However, sometimes this causes issues, e.g calling AcquireLock and then LoadRom from the
	//same thread while the debugger is active will cause a deadlock. This is why this behavior
	//can be disabled, as required.
	return EmulatorLock(this, allowDebuggerLock);
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

DeserializeResult Emulator::Deserialize(istream& in, uint32_t fileFormatVersion, bool includeSettings, optional<ConsoleType> srcConsoleType, bool sendNotification)
{
	Serializer s(fileFormatVersion, false);
	if(!s.LoadFrom(in)) {
		return DeserializeResult::InvalidFile;
	}

	if(includeSettings) {
		SV(_settings);
	}

	if(srcConsoleType.has_value() && srcConsoleType.value() != _console->GetConsoleType()) {
		//Used to allow save states taken on GB/GBC/SGB to be loaded on any of the 3 systems
		SaveStateCompatInfo compatInfo = _console->ValidateSaveStateCompatibility(srcConsoleType.value());
		if(!compatInfo.IsCompatible) {
			MessageManager::DisplayMessage("SaveStates", "SaveStateWrongSystem");
			return DeserializeResult::SpecificError;
		}

		s.RemoveKeys(compatInfo.FieldsToRemove);

		if(!compatInfo.PrefixToAdd.empty()) {
			s.AddKeyPrefix(compatInfo.PrefixToAdd);
		} else if(!compatInfo.PrefixToRemove.empty()) {
			s.RemoveKeyPrefix(compatInfo.PrefixToRemove);
		}

		if(!s.IsValid()) {
			MessageManager::DisplayMessage("SaveStates", "SaveStateWrongSystem");
			return DeserializeResult::SpecificError;
		}
	}

	s.Stream(_console, "");
	if(s.HasError()) {
		return DeserializeResult::SpecificError;
	}

	if(sendNotification) {
		_notificationManager->SendNotification(ConsoleNotificationType::StateLoaded);
	}
	return DeserializeResult::Success;
}

BaseVideoFilter* Emulator::GetVideoFilter(bool getDefaultFilter)
{
	shared_ptr<IConsole> console = GetConsole();
	return console ? console->GetVideoFilter(getDefaultFilter) : new SnesDefaultVideoFilter(this);
}

void Emulator::GetScreenRotationOverride(uint32_t& rotation)
{
	shared_ptr<IConsole> console = GetConsole();
	if(console) {
		console->GetScreenRotationOverride(rotation);
	}
}

void Emulator::InputBarcode(uint64_t barcode, uint32_t digitCount)
{
	shared_ptr<IConsole> console = GetConsole();
	if(console) {
		shared_ptr<IBarcodeReader> reader = console->GetControlManager()->GetControlDevice<IBarcodeReader>();
		if(reader) {
			auto lock = AcquireLock();
			reader->InputBarcode(barcode, digitCount);
		}
	}
}

void Emulator::ProcessTapeRecorderAction(TapeRecorderAction action, string filename)
{
	shared_ptr<IConsole> console = GetConsole();
	if(console) {
		shared_ptr<ITapeRecorder> recorder = console->GetControlManager()->GetControlDevice<ITapeRecorder>();
		if(recorder) {
			auto lock = AcquireLock();
			recorder->ProcessTapeRecorderAction(action, filename);
		}
	}
}

ShortcutState Emulator::IsShortcutAllowed(EmulatorShortcut shortcut, uint32_t shortcutParam)
{
	shared_ptr<IConsole> console = GetConsole();
	return console ? console->IsShortcutAllowed(shortcut, shortcutParam) : ShortcutState::Default;
}

bool Emulator::IsKeyboardConnected()
{
	shared_ptr<IConsole> console = GetConsole();
	return console ? console->GetControlManager()->IsKeyboardConnected() : false;
}

void Emulator::BlockDebuggerRequests()
{
	//Block all new debugger calls
	auto lock = _debuggerLock.AcquireSafe();
	_blockDebuggerRequestCount++;
	if(_debugger) {
		//Ensure any thread waiting on DebugBreakHelper is allowed to resume/finish (prevent deadlock)
		_debugger->ResetSuspendCounter();
	}

	while(_debugRequestCount > 0) {
		//Wait until debugger calls are all done
		std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(10));
	}
}

DebuggerRequest Emulator::GetDebugger(bool autoInit)
{
	if(IsRunning() && _blockDebuggerRequestCount == 0) {
		auto lock = _debuggerLock.AcquireSafe();
		if(IsRunning() && _blockDebuggerRequestCount == 0) {
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
			_blockDebuggerRequestCount--;

			//_paused should be false while debugger is enabled
			_paused = false;
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
			_blockDebuggerRequestCount--;
		}
	}
}

bool Emulator::IsEmulationThread()
{
	return _emulationThreadId == std::this_thread::get_id();
}

void Emulator::SetStopCode(int32_t stopCode)
{
	if(_stopCode != 0) {
		//If a non-0 code was already set, keep the previous value
		return;
	}

	_stopCode = stopCode;
	if(!_stopFlag && !_stopRequested) {
		_stopRequested = true;
		thread stopEmuTask([this]() {
			Stop(true);
		});
		stopEmuTask.detach();
	}
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
	shared_ptr<IConsole> console = GetConsole();
	if(!console) {
		return {};
	}

	AudioTrackInfo track = console->GetAudioTrackInfo();
	AudioConfig audioCfg = _settings->GetAudioConfig();
	if(track.Length <= 0 && audioCfg.AudioPlayerEnableTrackLength) {
		track.Length = audioCfg.AudioPlayerTrackLength;
		track.FadeLength = 1;
	}
	return track;
}

void Emulator::ProcessAudioPlayerAction(AudioPlayerActionParams p)
{
	shared_ptr<IConsole> console = GetConsole();
	if(console) {
		console->ProcessAudioPlayerAction(p);
	}
}

void Emulator::ProcessEvent(EventType type, std::optional<CpuType> cpuType)
{
	if(_debugger) {
		_debugger->ProcessEvent(type, cpuType);
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

template void Emulator::AddDebugEvent<CpuType::Snes>(DebugEventType evtType);
template void Emulator::AddDebugEvent<CpuType::Gameboy>(DebugEventType evtType);
template void Emulator::AddDebugEvent<CpuType::Nes>(DebugEventType evtType);
template void Emulator::AddDebugEvent<CpuType::Pce>(DebugEventType evtType);
