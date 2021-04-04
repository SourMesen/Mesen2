#include "stdafx.h"
#include "Emulator.h"
#include "SNES/Console.h"
#include "Debugger.h"
#include "DebugTypes.h"
#include "NotificationManager.h"
#include "SoundMixer.h"
#include "VideoDecoder.h"
#include "VideoRenderer.h"
#include "DebugHud.h"
#include "FrameLimiter.h"
#include "MessageManager.h"
#include "KeyManager.h"
#include "EventType.h"
#include "EmuSettings.h"
#include "SaveStateManager.h"
#include "DebugStats.h"
#include "RewindManager.h"
#include "EmulatorLock.h"
#include "MovieManager.h"
#include "BatteryManager.h"
#include "CheatManager.h"
#include "MovieManager.h"
#include "IConsole.h"
#include "SystemActionManager.h"
#include "Utilities/Serializer.h"
#include "Utilities/Timer.h"
#include "Utilities/VirtualFile.h"
#include "Utilities/PlatformUtilities.h"
#include "Utilities/FolderUtilities.h"

Emulator::Emulator()
{
	_settings.reset(new EmuSettings(this));

	_paused = false;
	_pauseOnNextFrame = false;
	_stopFlag = false;
	_isRunAheadFrame = false;
	_lockCounter = 0;
	_threadPaused = false;
}

Emulator::~Emulator()
{
}

void Emulator::Initialize()
{
	_lockCounter = 0;

	_notificationManager.reset(new NotificationManager());
	_batteryManager.reset(new BatteryManager());
	_videoDecoder.reset(new VideoDecoder(shared_from_this()));
	_videoRenderer.reset(new VideoRenderer(shared_from_this()));
	_saveStateManager.reset(new SaveStateManager(shared_from_this()));
	_soundMixer.reset(new SoundMixer(this));
	_debugHud.reset(new DebugHud());
	_cheatManager.reset(new CheatManager(this));
	_movieManager.reset(new MovieManager(shared_from_this()));

	_videoDecoder->StartThread();
	_videoRenderer->StartThread();
}

void Emulator::Release()
{
	Stop(true);

	_videoDecoder->StopThread();
	_videoRenderer->StopThread();

	_videoDecoder.reset();
	_videoRenderer.reset();
	_debugHud.reset();
	_notificationManager.reset();
	_saveStateManager.reset();
	_soundMixer.reset();
	_settings.reset();
	_cheatManager.reset();
	_movieManager.reset();
}

void Emulator::Run()
{
	if(!_console) {
		return;
	}

	auto emulationLock = _emulationLock.AcquireSafe();
	auto lock = _runLock.AcquireSafe();

	_stopFlag = false;
	_isRunAheadFrame = false;

	PlatformUtilities::EnableHighResolutionTimer();

	_videoDecoder->StartThread();
	_emulationThreadId = std::this_thread::get_id();

	_console->OnBeforeRun();

	_frameDelay = _console->GetFrameDelay();
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

	PlatformUtilities::RestoreTimerResolution();
}

bool Emulator::ProcessSystemActions()
{
	//TODO
	/*if(_controlManager->GetSystemActionManager()->IsResetPressed()) {
		Reset();
		return true;
	} else if(_controlManager->GetSystemActionManager()->IsPowerCyclePressed()) {
		PowerCycle();
		return true;
	}*/
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

void Emulator::ProcessEndOfFrame()
{
#ifndef LIBRETRO
	if(!_isRunAheadFrame) {
		_frameLimiter->ProcessFrame();
		while(_frameLimiter->WaitForNextFrame()) {
			if(_stopFlag || _frameDelay != _console->GetFrameDelay() || _paused || _pauseOnNextFrame || _lockCounter > 0) {
				//Need to process another event, stop sleeping
				break;
			}
		}

		double newFrameDelay = _console->GetFrameDelay();
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

void Emulator::Stop(bool sendNotification)
{
	_stopFlag = true;

	_notificationManager->SendNotification(ConsoleNotificationType::BeforeGameUnload);

	shared_ptr<Debugger> debugger = _debugger;
	if(debugger) {
		debugger->SuspendDebugger(false);
		debugger->Run();
	}

	_emulationLock.WaitForRelease();

	if(_emuThread) {
		_emuThread->join();
		_emuThread.release();
	}

	if(_console && !_settings->GetPreferences().DisableGameSelectionScreen) {
		RomInfo romInfo = GetRomInfo();
		_saveStateManager->SaveRecentGame(romInfo.RomFile.GetFileName(), romInfo.RomFile, romInfo.PatchFile);
	}

	if(sendNotification) {
		_notificationManager->SendNotification(ConsoleNotificationType::BeforeEmulationStop);
	}

	_settings->ClearFlag(EmulationFlags::GameboyMode);

	//Make sure we release both pointers to destroy the debugger before everything else
	_debugger.reset();
	debugger.reset();

	_videoDecoder->StopThread();
	_rewindManager.reset();

	if(_console) {
		_console->Stop();
	}

	_soundMixer->StopAudio(true);

	if(sendNotification) {
		_notificationManager->SendNotification(ConsoleNotificationType::EmulationStopped);
	}
}

void Emulator::Reset()
{
	shared_ptr<Debugger> debugger = _debugger;

	_lockCounter++;
	_runLock.Acquire();

	_console->Reset();

	_notificationManager->SendNotification(ConsoleNotificationType::GameReset);
	ProcessEvent(EventType::Reset);

	if(debugger) {
		//Debugger was suspended in SystemActionManager::Reset(), resume debugger here
		debugger->SuspendDebugger(true);
	}

	_runLock.Release();
	_lockCounter--;
}

void Emulator::ReloadRom(bool forPowerCycle)
{
	shared_ptr<Debugger> debugger = _debugger;
	if(debugger) {
		debugger->Run();
	}

	RomInfo info = GetRomInfo();
	Lock();
	LoadRom(info.RomFile, info.PatchFile, false, forPowerCycle);
	Unlock();
}

void Emulator::PowerCycle()
{
	ReloadRom(true);
}

bool Emulator::LoadRom(VirtualFile romFile, VirtualFile patchFile, bool stopRom, bool forPowerCycle)
{
	if(_console) {
		//Make sure the battery is saved to disk before we load another game (or reload the same game)
		//TODO
		//_console->SaveBattery();
	}

	bool result = false;
	EmulationConfig orgConfig = _settings->GetEmulationConfig(); //backup emulation config (can be temporarily overriden to control the power on RAM state)
	
	shared_ptr<IConsole> console = shared_ptr<IConsole>(new Console(this));
	if(!console->LoadRom(romFile, patchFile)) {
		return false;
	}

	if(console) {
		bool debuggerActive = _debugger != nullptr;
		if(stopRom) {
			KeyManager::UpdateDevices();
			Stop(false);
		}

		_cheatManager->ClearCheats(false);

		auto lock = _debuggerLock.AcquireSafe();
		if(_debugger) {
			//Reset debugger if it was running before
			_debugger->Release();
			_debugger.reset();
		}

		_batteryManager->Initialize(FolderUtilities::GetFilename(romFile.GetFileName(), false));

		//TODO
		//UpdateRegion();

		_console = console;
		console->Init();

		if(debuggerActive) {
			GetDebugger();
		}

		_rewindManager.reset(new RewindManager(shared_from_this()));
		_notificationManager->RegisterNotificationListener(_rewindManager);

		//TODO
		//_controlManager->UpdateControlDevices();
		//UpdateRegion();

		_notificationManager->SendNotification(ConsoleNotificationType::GameLoaded, (void*)forPowerCycle);

		_paused = false;

		if(!forPowerCycle) {
			string modelName = _region == ConsoleRegion::Pal ? "PAL" : "NTSC";
			string messageTitle = MessageManager::Localize("GameLoaded") + " (" + modelName + ")";
			MessageManager::DisplayMessage(messageTitle, FolderUtilities::GetFilename(GetRomInfo().RomFile.GetFileName(), false));
		}

		if(stopRom) {
		#ifndef LIBRETRO
			_emuThread.reset(new thread(&Emulator::Run, this));
		#endif
		}
		result = true;
	} else {
		MessageManager::DisplayMessage("Error", "CouldNotLoadFile", romFile.GetFileName());
	}

	_settings->SetEmulationConfig(orgConfig);
	return result;
}

RomInfo Emulator::GetRomInfo()
{
	return _console->GetRomInfo();
}

string Emulator::GetHash(HashType type)
{
	//TODO
	return "";
}

PpuFrameInfo Emulator::GetPpuFrame()
{
	return _console->GetPpuFrame();
}

ConsoleRegion Emulator::GetRegion()
{
	//TODO
	return ConsoleRegion::Ntsc;
}

ConsoleType Emulator::GetConsoleType()
{
	//TODO
	return ConsoleType::Snes;
}

uint32_t Emulator::GetFrameCount()
{
	return _console->GetPpuFrame().FrameCount;
}

double Emulator::GetFps()
{
	return _console->GetFps();
}

void Emulator::PauseOnNextFrame()
{
	shared_ptr<Debugger> debugger = _debugger;
	if(debugger) {
		if(_settings->CheckFlag(EmulationFlags::GameboyMode)) {
			debugger->Step(CpuType::Gameboy, 144, StepType::SpecificScanline);
		} else {
			debugger->Step(CpuType::Cpu, 240, StepType::SpecificScanline);
		}
	} else {
		_pauseOnNextFrame = true;
		_paused = false;
	}
}

void Emulator::Pause()
{
	shared_ptr<Debugger> debugger = _debugger;
	if(debugger) {
		if(_settings->CheckFlag(EmulationFlags::GameboyMode)) {
			debugger->Step(CpuType::Gameboy, 1, StepType::Step);
		} else {
			debugger->Step(CpuType::Cpu, 1, StepType::Step);
		}
	} else {
		_paused = true;
	}
}

void Emulator::Resume()
{
	shared_ptr<Debugger> debugger = _debugger;
	if(debugger) {
		debugger->Run();
	} else {
		_paused = false;
	}
}

bool Emulator::IsPaused()
{
	shared_ptr<Debugger> debugger = _debugger;
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
	_runLock.Acquire();
	if(!_stopFlag) {
		_notificationManager->SendNotification(ConsoleNotificationType::GameResumed);
	}
}

EmulatorLock Emulator::AcquireLock()
{
	return EmulatorLock(this);
}

void Emulator::Lock()
{
	shared_ptr<Debugger> debugger = _debugger;
	if(debugger) {
		debugger->SuspendDebugger(false);
	}

	_lockCounter++;
	_runLock.Acquire();
}

void Emulator::Unlock()
{
	shared_ptr<Debugger> debugger = _debugger;
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
		while(_lockCounter > 0) {}

		shared_ptr<Debugger> debugger = _debugger;
		if(debugger) {
			while(debugger->HasBreakRequest()) {}
		}

		_threadPaused = false;

		_runLock.Acquire();
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

shared_ptr<SoundMixer> Emulator::GetSoundMixer()
{
	return _soundMixer;
}

shared_ptr<VideoRenderer> Emulator::GetVideoRenderer()
{
	return _videoRenderer;
}

shared_ptr<VideoDecoder> Emulator::GetVideoDecoder()
{
	return _videoDecoder;
}

shared_ptr<NotificationManager> Emulator::GetNotificationManager()
{
	return _notificationManager;
}

shared_ptr<EmuSettings> Emulator::GetSettings()
{
	return _settings;
}

shared_ptr<SaveStateManager> Emulator::GetSaveStateManager()
{
	return _saveStateManager;
}

shared_ptr<RewindManager> Emulator::GetRewindManager()
{
	return _rewindManager;
}

shared_ptr<DebugHud> Emulator::GetDebugHud()
{
	return _debugHud;
}

shared_ptr<BatteryManager> Emulator::GetBatteryManager()
{
	return _batteryManager;
}

shared_ptr<CheatManager> Emulator::GetCheatManager()
{
	return _cheatManager;
}

shared_ptr<MovieManager> Emulator::GetMovieManager()
{
	return _movieManager;
}

shared_ptr<ControlManager> Emulator::GetControlManager()
{
	return _console->GetControlManager();
}

shared_ptr<Debugger> Emulator::GetDebugger(bool autoStart)
{
	shared_ptr<Debugger> debugger = _debugger;
	if(!debugger && autoStart) {
		//Lock to make sure we don't try to start debuggers in 2 separate threads at once
		auto lock = _debuggerLock.AcquireSafe();
		debugger = _debugger;
		if(!debugger) {
			//debugger.reset(new Debugger(shared_from_this()));
			_debugger = debugger;
		}
	}
	return debugger;
}

void Emulator::StopDebugger()
{
	//Pause/unpause the regular emulation thread based on the debugger's pause state
	_paused = IsPaused();

	shared_ptr<Debugger> debugger = _debugger;
	debugger->SuspendDebugger(false);
	Lock();
	_debugger.reset();

	Unlock();
}

bool Emulator::IsDebugging()
{
	return _debugger != nullptr;
}

thread::id Emulator::GetEmulationThreadId()
{
	return _emulationThreadId;
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
	//TODO
	/*if(type == EventType::EndFrame && _spcHud) {
		_spcHud->Draw(GetFrameCount());
	}*/

	if(_debugger) {
		_debugger->ProcessEvent(type);
	}
}

void Emulator::BreakImmediately(BreakSource source)
{
	if(_debugger) {
		_debugger->BreakImmediately(source);
	}
}

template void Emulator::ProcessMemoryRead<CpuType::Cpu>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Emulator::ProcessMemoryRead<CpuType::Sa1>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Emulator::ProcessMemoryRead<CpuType::Spc>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Emulator::ProcessMemoryRead<CpuType::Gsu>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Emulator::ProcessMemoryRead<CpuType::NecDsp>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Emulator::ProcessMemoryRead<CpuType::Cx4>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Emulator::ProcessMemoryRead<CpuType::Gameboy>(uint32_t addr, uint8_t value, MemoryOperationType opType);

template void Emulator::ProcessMemoryWrite<CpuType::Cpu>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Emulator::ProcessMemoryWrite<CpuType::Sa1>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Emulator::ProcessMemoryWrite<CpuType::Spc>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Emulator::ProcessMemoryWrite<CpuType::Gsu>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Emulator::ProcessMemoryWrite<CpuType::NecDsp>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Emulator::ProcessMemoryWrite<CpuType::Cx4>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Emulator::ProcessMemoryWrite<CpuType::Gameboy>(uint32_t addr, uint8_t value, MemoryOperationType opType);

template void Emulator::ProcessInterrupt<CpuType::Cpu>(uint32_t originalPc, uint32_t currentPc, bool forNmi);
template void Emulator::ProcessInterrupt<CpuType::Sa1>(uint32_t originalPc, uint32_t currentPc, bool forNmi);
template void Emulator::ProcessInterrupt<CpuType::Gameboy>(uint32_t originalPc, uint32_t currentPc, bool forNmi);
