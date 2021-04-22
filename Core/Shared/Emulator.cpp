#include "stdafx.h"
#include "Shared/Emulator.h"
#include "Shared/NotificationManager.h"
#include "Shared/Audio/SoundMixer.h"
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
#include "Shared/EmulatorLock.h"
#include "Shared/Movies/MovieManager.h"
#include "Shared/BatteryManager.h"
#include "Shared/CheatManager.h"
#include "Shared/SystemActionManager.h"
#include "Shared/Movies/MovieManager.h"
#include "Shared/Interfaces/IConsole.h"
#include "Shared/Interfaces/IControlManager.h"
#include "SNES/Console.h"
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
	_systemActionManager.reset(new SystemActionManager(this));

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
	GetControlManager()->UpdateInputState();

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
	if(!romFile.IsValid()) {
		return false;
	}

	if(patchFile.IsValid()) {
		_patchFile = patchFile;
		if(romFile.ApplyPatch(patchFile)) {
			MessageManager::DisplayMessage("Patch", "ApplyingPatch", patchFile.GetFileName());
		}
	}

	if(_console) {
		//Make sure the battery is saved to disk before we load another game (or reload the same game)
		_console->SaveBattery();
	}
	
	_batteryManager->Initialize(FolderUtilities::GetFilename(romFile.GetFileName(), false));

	bool result = false;
	EmulationConfig orgConfig = _settings->GetEmulationConfig(); //backup emulation config (can be temporarily overriden to control the power on RAM state)
	
	memset(_consoleMemory, 0, sizeof(_consoleMemory));
	shared_ptr<IConsole> console = shared_ptr<IConsole>(new NesConsole(this));
	if(!console->LoadRom(romFile)) {
		memset(_consoleMemory, 0, sizeof(_consoleMemory));
		console.reset(new Console(this));
		if(!console->LoadRom(romFile)) {
			memset(_consoleMemory, 0, sizeof(_consoleMemory));
			console.reset(new Gameboy(this, false));
			if(!console->LoadRom(romFile)) {
				memset(_consoleMemory, 0, sizeof(_consoleMemory));
				MessageManager::DisplayMessage("Error", "CouldNotLoadFile", romFile.GetFileName());
				
				//TODO
				_settings->SetEmulationConfig(orgConfig);
				return false;
			}
		}
	}

	_romFile = romFile;
	_patchFile = patchFile;

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
	GetControlManager()->UpdateControlDevices();
	GetControlManager()->UpdateInputState();
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
		
	return result;
}

RomInfo Emulator::GetRomInfo()
{
	RomInfo romInfo = {};
	romInfo.RomFile = _romFile;
	romInfo.PatchFile = _patchFile;
	return romInfo;
}

string Emulator::GetHash(HashType type)
{
	//TODO
	return "";
}

uint32_t Emulator::GetCrc32()
{
	//TODO
	return 0;
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
	return _console->GetPpuFrame().FrameCount;
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
		frameDelay = _console->GetFrameDelay();
		frameDelay /= (emulationSpeed / 100.0);
	}
	return frameDelay;
}

void Emulator::PauseOnNextFrame()
{
	shared_ptr<Debugger> debugger = _debugger;
	if(debugger) {
		switch(GetConsoleType()) {
			case ConsoleType::Snes:
				debugger->Step(CpuType::Cpu, 240, StepType::SpecificScanline);
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
	shared_ptr<Debugger> debugger = _debugger;
	if(debugger) {
		switch(GetConsoleType()) {
			case ConsoleType::Snes:
				debugger->Step(CpuType::Cpu, 1, StepType::Step);
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

EmuSettings* Emulator::GetSettings()
{
	return _settings.get();
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

shared_ptr<SystemActionManager> Emulator::GetSystemActionManager()
{
	return _systemActionManager;
}

shared_ptr<IControlManager> Emulator::GetControlManager()
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
			debugger.reset(new Debugger(this, _console.get()));
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

void Emulator::RegisterMemory(SnesMemoryType type, void* memory, uint32_t size)
{
	_consoleMemory[(int)type] = { memory, size };
}

ConsoleMemoryInfo Emulator::GetMemory(SnesMemoryType type)
{
	return _consoleMemory[(int)type];
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

template<CpuType cpuType>
void Emulator::AddDebugEvent(DebugEventType evtType)
{
	if(_debugger) {
		_debugger->GetEventManager(cpuType)->AddEvent(evtType);
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

template void Emulator::AddDebugEvent<CpuType::Cpu>(DebugEventType evtType);
template void Emulator::AddDebugEvent<CpuType::Gameboy>(DebugEventType evtType);
template void Emulator::AddDebugEvent<CpuType::Nes>(DebugEventType evtType);
