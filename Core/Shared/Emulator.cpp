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
#include "Shared/Interfaces/IControlManager.h"
#include "SNES/SnesConsole.h"
#include "SNES/SnesDefaultVideoFilter.h"
#include "NES/NesConsole.h"
#include "Gameboy/Gameboy.h"
#include "Debugger/Debugger.h"
#include "Debugger/BaseEventManager.h"
#include "Debugger/DebugTypes.h"
#include "Utilities/Serializer.h"
#include "Utilities/Timer.h"
#include "Utilities/VirtualFile.h"
#include "Utilities/PlatformUtilities.h"
#include "Utilities/FolderUtilities.h"
#include "SystemActionManager.h"
#include "MemoryOperationType.h"
#include "EventType.h"

Emulator::Emulator() :
	_settings(new EmuSettings(this)),
	_debugHud(new DebugHud()),
	_notificationManager(new NotificationManager()),
	_batteryManager(new BatteryManager()),
	_videoDecoder(new VideoDecoder(this)),
	_videoRenderer(new VideoRenderer(this)),
	_saveStateManager(new SaveStateManager(this)),
	_soundMixer(new SoundMixer(this)),
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

	_console->OnBeforeRun();

	_frameDelay = GetFrameDelay();
	_stats.reset(new DebugStats());
	_frameLimiter.reset(new FrameLimiter(_frameDelay));
	_lastFrameTimer.Reset();

	while(!_stopFlag) {
		bool useRunAhead = _settings->GetEmulationConfig().RunAheadFrames > 0 && !_debugger && !_rewindManager->IsRewinding() && _settings->GetEmulationSpeed() > 0 && _settings->GetEmulationSpeed() <= 100;
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

		//TODO
		/*if(_memoryManager->GetMasterClock() == 0) {
			//After a reset or power cycle, run the PPU/etc ahead of the CPU (simulates delay CPU takes to get out of reset)
			_memoryManager->IncMasterClockStartup();
		}*/
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
	Serialize(runAheadState, 0);

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
	}
}

void Emulator::ProcessEndOfFrame()
{
#ifndef LIBRETRO
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

		PreferencesConfig cfg = _settings->GetPreferences();
		if(cfg.ShowDebugInfo) {
			double lastFrameTime = _lastFrameTimer.GetElapsedMS();
			_lastFrameTimer.Reset();
			_stats->DisplayStats(this, lastFrameTime);
		}
	}
#endif
	_frameRunning = false;
}

void Emulator::RunSingleFrame()
{
	//Used by Libretro
	//TODO
	/*_emulationThreadId = std::this_thread::get_id();
	_isRunAheadFrame = false;

	_controlManager->UpdateInputState();
	_internalRegisters->ProcessAutoJoypadRead();

	_console->RunFrame();

	_cart->RunCoprocessors();
	if(_cart->GetCoprocessor()) {
		_cart->GetCoprocessor()->ProcessEndOfFrame();
	}

	_controlManager->UpdateControlDevices();*/
}

void Emulator::Stop(bool sendNotification, bool preventRecentGameSave)
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
		_console->Stop();
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
	_notificationManager->SendNotification(ConsoleNotificationType::BeforeGameLoad);

	BlockDebuggerRequests();

	auto dbgLock = _debuggerLock.AcquireSafe();
	auto emuLock = AcquireLock();
	auto lock = _loadLock.AcquireSafe();

	if(!romFile.IsValid()) {
		return false;
	}

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
	
	//TODO need to restore name if load fails
	_batteryManager->Initialize(FolderUtilities::GetFilename(romFile.GetFileName(), false));

	//TODO fix
	//backup emulation config (can be temporarily overriden to control the power on RAM state)
	EmulationConfig orgConfig = _settings->GetEmulationConfig();

	static const vector<string> _nesExtensions = { { ".nes", ".fds", ".unif", ".unf", ".nsf", ".nsfe", ".studybox" } };
	static const vector<string> _snesExtensions = { { ".sfc", ".swc", ".fig", ".smc", ".bs", ".gb", ".gbc", ".spc" } };
	static const vector<string> _gbExtensions = { { ".gb", ".gbc", ".gbs" } };
	
	unique_ptr<IConsole> console;
	string romExt = romFile.GetFileExtension();
	LoadRomResult result = LoadRomResult::UnknownType;

	if(std::find(_nesExtensions.begin(), _nesExtensions.end(), romExt) != _nesExtensions.end()) {
		//TODO consolememory rework
		memset(_consoleMemory, 0, sizeof(_consoleMemory));
		console.reset(new NesConsole(this));
		result = console->LoadRom(romFile);
	} 
	
	if(result == LoadRomResult::UnknownType && std::find(_snesExtensions.begin(), _snesExtensions.end(), romExt) != _snesExtensions.end()) {
		memset(_consoleMemory, 0, sizeof(_consoleMemory));
		console.reset(new SnesConsole(this));
		result = console->LoadRom(romFile);
	}
	
	if(result == LoadRomResult::UnknownType && std::find(_gbExtensions.begin(), _gbExtensions.end(), romExt) != _gbExtensions.end()) {
		memset(_consoleMemory, 0, sizeof(_consoleMemory));
		console.reset(new Gameboy(this, false));
		result = console->LoadRom(romFile);
	}

	_settings->SetEmulationConfig(orgConfig);

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
		Stop(false, !gameChanged);
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

	//TODO
	//UpdateRegion();

	_console.swap(console);
	_console->Init();

	if(debuggerActive) {
		InitDebugger();
	}

	_rewindManager.reset(new RewindManager(this));
	_notificationManager->RegisterNotificationListener(_rewindManager);

	//TODO
	GetControlManager()->UpdateControlDevices();
	GetControlManager()->UpdateInputState();
	//UpdateRegion();

	_autoSaveStateFrameCounter = 0;
	_allowDebuggerRequest = true;
	_threadPaused = true; //To avoid deadlocks with DebugBreakHelper if GameLoaded event starts the debugger
	_notificationManager->SendNotification(ConsoleNotificationType::GameLoaded, (void*)forPowerCycle);
	_threadPaused = false;
	_paused = false;

	if(!forPowerCycle && !_audioPlayerHud) {
		string modelName = _console->GetRegion() == ConsoleRegion::Pal ? "PAL" : "NTSC";
		MessageManager::DisplayMessage(modelName, FolderUtilities::GetFilename(GetRomInfo().RomFile.GetFileName(), false));
	}

	_videoDecoder->StartThread();
	_videoRenderer->StartThread();

	if(stopRom) {
#ifndef LIBRETRO
		_emuThread.reset(new thread(&Emulator::Run, this));
#endif
	}

	return true;
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
		frameDelay = 1000 / _console->GetFps();
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
		switch(GetConsoleType()) {
			case ConsoleType::Snes:
				debugger->Step(CpuType::Snes, 1, StepType::Step);
				break;

			case ConsoleType::Gameboy:
			case ConsoleType::GameboyColor:
				debugger->Step(CpuType::Gameboy, 1, StepType::Step);
				break;

			case ConsoleType::Nes:
				debugger->Step(CpuType::Nes, 1, StepType::Step);
				break;
		}
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
		return debugger->IsExecutionStopped();
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
	shared_ptr<Debugger> debugger = _debugger.lock();
	if(debugger) {
		debugger->SuspendDebugger(false);
	}

	_lockCounter++;
	_runLock.Acquire();
}

void Emulator::Unlock()
{
	shared_ptr<Debugger> debugger = _debugger.lock();
	if(debugger) {
		debugger->SuspendDebugger(true);
	}

	_runLock.Release();
	_lockCounter--;
}

bool Emulator::IsThreadPaused()
{
	return !_emuThread || _threadPaused;
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

void Emulator::Serialize(ostream& out, int compressionLevel)
{
	Serializer serializer(SaveStateManager::FileFormatVersion);
	serializer.Stream(_console.get());
	serializer.Save(out, compressionLevel);
}

void Emulator::Deserialize(istream& in, uint32_t fileFormatVersion, bool compressed)
{
	Serializer serializer(in, fileFormatVersion, compressed);
	serializer.Stream(_console.get());
	_notificationManager->SendNotification(ConsoleNotificationType::StateLoaded);
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

IControlManager* Emulator::GetControlManager()
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

bool Emulator::IsShortcutAllowed(EmulatorShortcut shortcut, uint32_t shortcutParam)
{
	if(_console) {
		return _console->IsShortcutAllowed(shortcut, shortcutParam);
	}
	return true;
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

void Emulator::ResetDebugger(Debugger* dbg)
{
	BlockDebuggerRequests();

	shared_ptr<Debugger> currentDbg = _debugger.lock();
	if(currentDbg) {
		currentDbg->SuspendDebugger(false);
	}

	if(_emulationThreadId == std::this_thread::get_id()) {
		_debugger.reset(dbg);
	} else {
		//Need to pause emulator to change _debugger (when not called from the emulation thread)
		auto emuLock = AcquireLock();
		_debugger.reset(dbg);
	}

	_allowDebuggerRequest = true;
}

void Emulator::InitDebugger()
{
	if(!_debugger) {
		//Lock to make sure we don't try to start debuggers in 2 separate threads at once
		auto lock = _debuggerLock.AcquireSafe();
		if(!_debugger) {
			ResetDebugger(new Debugger(this, _console.get()));
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
			ResetDebugger();
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

template void Emulator::ProcessMemoryRead<CpuType::Snes>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Emulator::ProcessMemoryRead<CpuType::Sa1>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Emulator::ProcessMemoryRead<CpuType::Spc>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Emulator::ProcessMemoryRead<CpuType::Gsu>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Emulator::ProcessMemoryRead<CpuType::NecDsp>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Emulator::ProcessMemoryRead<CpuType::Cx4>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Emulator::ProcessMemoryRead<CpuType::Gameboy>(uint32_t addr, uint8_t value, MemoryOperationType opType);

template void Emulator::ProcessMemoryWrite<CpuType::Snes>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Emulator::ProcessMemoryWrite<CpuType::Sa1>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Emulator::ProcessMemoryWrite<CpuType::Spc>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Emulator::ProcessMemoryWrite<CpuType::Gsu>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Emulator::ProcessMemoryWrite<CpuType::NecDsp>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Emulator::ProcessMemoryWrite<CpuType::Cx4>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Emulator::ProcessMemoryWrite<CpuType::Gameboy>(uint32_t addr, uint8_t value, MemoryOperationType opType);

template void Emulator::ProcessInterrupt<CpuType::Snes>(uint32_t originalPc, uint32_t currentPc, bool forNmi);
template void Emulator::ProcessInterrupt<CpuType::Sa1>(uint32_t originalPc, uint32_t currentPc, bool forNmi);
template void Emulator::ProcessInterrupt<CpuType::Gameboy>(uint32_t originalPc, uint32_t currentPc, bool forNmi);
template void Emulator::ProcessInterrupt<CpuType::Nes>(uint32_t originalPc, uint32_t currentPc, bool forNmi);

template void Emulator::AddDebugEvent<CpuType::Snes>(DebugEventType evtType);
template void Emulator::AddDebugEvent<CpuType::Gameboy>(DebugEventType evtType);
template void Emulator::AddDebugEvent<CpuType::Nes>(DebugEventType evtType);
