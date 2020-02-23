#include "stdafx.h"
#include "Console.h"
#include "Cpu.h"
#include "Ppu.h"
#include "Spc.h"
#include "NecDsp.h"
#include "InternalRegisters.h"
#include "ControlManager.h"
#include "MemoryManager.h"
#include "DmaController.h"
#include "BaseCartridge.h"
#include "RamHandler.h"
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
#include "CartTypes.h"
#include "RewindManager.h"
#include "ConsoleLock.h"
#include "MovieManager.h"
#include "BatteryManager.h"
#include "CheatManager.h"
#include "MovieManager.h"
#include "SystemActionManager.h"
#include "SpcHud.h"
#include "Msu1.h"
#include "../Utilities/Serializer.h"
#include "../Utilities/Timer.h"
#include "../Utilities/VirtualFile.h"
#include "../Utilities/PlatformUtilities.h"
#include "../Utilities/FolderUtilities.h"

Console::Console()
{
	_settings.reset(new EmuSettings(this));

	_paused = false;
	_pauseOnNextFrame = false;
	_stopFlag = false;
	_lockCounter = 0;
}

Console::~Console()
{
}

void Console::Initialize()
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

void Console::Release()
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

void Console::RunFrame()
{
	uint32_t frameCount = _ppu->GetFrameCount();
	while(frameCount == _ppu->GetFrameCount()) {
		_cpu->Exec();
	}
}

void Console::Run()
{
	if(!_cpu) {
		return;
	}
	
	auto emulationLock = _emulationLock.AcquireSafe();
	auto lock = _runLock.AcquireSafe();

	_stopFlag = false;
	_isRunAheadFrame = false;

	PlatformUtilities::EnableHighResolutionTimer();

	_videoDecoder->StartThread();
	_emulationThreadId = std::this_thread::get_id();

	_memoryManager->IncMasterClockStartup();
	_controlManager->UpdateInputState();

	_frameDelay = GetFrameDelay();
	_stats.reset(new DebugStats());
	_frameLimiter.reset(new FrameLimiter(_frameDelay));
	_lastFrameTimer.Reset();

	while(!_stopFlag) {
		bool useRunAhead = _settings->GetEmulationConfig().RunAheadFrames > 0 && !_debugger && !_rewindManager->IsRewinding() && _settings->GetEmulationSpeed() > 0 && _settings->GetEmulationSpeed() <= 100;
		if(useRunAhead) {
			RunFrameWithRunAhead();
		} else {
			RunFrame();
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
	}

	_movieManager->Stop();

	_emulationThreadId = thread::id();

	PlatformUtilities::RestoreTimerResolution();
}

bool Console::ProcessSystemActions()
{
	if(_controlManager->GetSystemActionManager()->IsResetPressed()) {
		Reset();
		return true;
	} else if(_controlManager->GetSystemActionManager()->IsPowerCyclePressed()) {
		PowerCycle();
		return true;
	}
	return false;
}

void Console::RunFrameWithRunAhead()
{
	stringstream runAheadState;
	uint32_t frameCount = _settings->GetEmulationConfig().RunAheadFrames;

	//Run a single frame and save the state (no audio/video)
	_isRunAheadFrame = true;
	RunFrame();
	Serialize(runAheadState, 0);

	while(frameCount > 1) {
		//Run extra frames if the requested run ahead frame count is higher than 1
		frameCount--;
		RunFrame();
	}
	_isRunAheadFrame = false;

	//Run one frame normally (with audio/video output)
	RunFrame();
	_rewindManager->ProcessEndOfFrame();
	
	bool wasReset = ProcessSystemActions();
	if(!wasReset) {
		//Load the state we saved earlier
		_isRunAheadFrame = true;
		Deserialize(runAheadState, SaveStateManager::FileFormatVersion, false);
		_isRunAheadFrame = false;
	}
}

void Console::ProcessEndOfFrame()
{
#ifndef LIBRETRO
	_cart->RunCoprocessors();
	if(_cart->GetCoprocessor()) {
		_cart->GetCoprocessor()->ProcessEndOfFrame();
	}

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

	_controlManager->UpdateInputState();
	_controlManager->UpdateControlDevices();
	_internalRegisters->ProcessAutoJoypadRead();
#endif
}

void Console::RunSingleFrame()
{
	//Used by Libretro
	uint32_t lastFrameNumber = _ppu->GetFrameCount();
	_emulationThreadId = std::this_thread::get_id();

	_controlManager->UpdateInputState();
	_internalRegisters->ProcessAutoJoypadRead();

	while(_ppu->GetFrameCount() == lastFrameNumber) {
		_cpu->Exec();
	}

	_cart->RunCoprocessors();
	if(_cart->GetCoprocessor()) {
		_cart->GetCoprocessor()->ProcessEndOfFrame();
	}

	_controlManager->UpdateControlDevices();
}

void Console::Stop(bool sendNotification)
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

	if(_cart && !_settings->GetPreferences().DisableGameSelectionScreen) {
		RomInfo romInfo = _cart->GetRomInfo();
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

	_cpu.reset();
	_ppu.reset();
	_spc.reset();
	_cart.reset();
	_internalRegisters.reset();
	_controlManager.reset();
	_memoryManager.reset();
	_dmaController.reset();
	_msu1.reset();

	_soundMixer->StopAudio(true);

	if(sendNotification) {
		_notificationManager->SendNotification(ConsoleNotificationType::EmulationStopped);
	}
}

void Console::Reset()
{
	shared_ptr<Debugger> debugger = _debugger;

	_lockCounter++;
	_runLock.Acquire();

	_dmaController->Reset();
	_internalRegisters->Reset();
	_memoryManager->Reset();
	_spc->Reset();
	_ppu->Reset();
	_cart->Reset();
	//_controlManager->Reset();

	//Reset cart before CPU to ensure correct memory mappings when fetching reset vector
	_cpu->Reset();

	_notificationManager->SendNotification(ConsoleNotificationType::GameReset);
	ProcessEvent(EventType::Reset);

	if(_cart->GetSpcData()) {
		_spc->LoadSpcFile(_cart->GetSpcData());
		_spcHud.reset(new SpcHud(this, _cart->GetSpcData()));
	} else {
		_spcHud.reset();
	}

	if(debugger) {
		//Debugger was suspended in SystemActionManager::Reset(), resume debugger here
		debugger->SuspendDebugger(true);
	}

	_memoryManager->IncMasterClockStartup();

	_runLock.Release(); 
	_lockCounter--;
}

void Console::ReloadRom(bool forPowerCycle)
{
	shared_ptr<BaseCartridge> cart = _cart;
	if(cart) {
		shared_ptr<Debugger> debugger = _debugger;
		if(debugger) {
			debugger->Run();
		}

		RomInfo info = cart->GetRomInfo();
		Lock();
		LoadRom(info.RomFile, info.PatchFile, false, forPowerCycle);

		_memoryManager->IncMasterClockStartup();
		Unlock();
	}
}

void Console::PowerCycle()
{
	ReloadRom(true);
}

bool Console::LoadRom(VirtualFile romFile, VirtualFile patchFile, bool stopRom, bool forPowerCycle)
{
	if(_cart) {
		//Make sure the battery is saved to disk before we load another game (or reload the same game)
		_cart->SaveBattery();
	}

	bool result = false;
	EmulationConfig orgConfig = _settings->GetEmulationConfig(); //backup emulation config (can be temporarily overriden to control the power on RAM state)
	shared_ptr<BaseCartridge> cart = forPowerCycle ? _cart : BaseCartridge::CreateCartridge(this, romFile, patchFile);
	if(cart) {
		if(stopRom) {
			KeyManager::UpdateDevices();
			Stop(false);
		}

		_cheatManager->ClearCheats(false);
		
		_cart = cart;
		_batteryManager->Initialize(FolderUtilities::GetFilename(romFile.GetFileName(), false));

		UpdateRegion();

		_internalRegisters.reset(new InternalRegisters());
		_memoryManager.reset(new MemoryManager());
		_ppu.reset(new Ppu(this));
		_controlManager.reset(new ControlManager(this));
		_dmaController.reset(new DmaController(_memoryManager.get()));
		_spc.reset(new Spc(this));

		_msu1.reset(Msu1::Init(romFile, _spc.get()));

		if(_cart->GetSpcData()) {
			_spc->LoadSpcFile(_cart->GetSpcData());
			_spcHud.reset(new SpcHud(this, _cart->GetSpcData()));
		} else {
			_spcHud.reset();
		}

		_cpu.reset(new Cpu(this));
		_memoryManager->Initialize(this);
		_internalRegisters->Initialize(this);

		if(_debugger) {
			//Reset debugger if it was running before
			auto lock = _debuggerLock.AcquireSafe();
			_debugger->Release();
			_debugger.reset();
			GetDebugger();
		}

		_ppu->PowerOn();
		_cpu->PowerOn();

		_rewindManager.reset(new RewindManager(shared_from_this()));
		_notificationManager->RegisterNotificationListener(_rewindManager);

		_controlManager->UpdateControlDevices();
				
		UpdateRegion();

		_notificationManager->SendNotification(ConsoleNotificationType::GameLoaded, (void*)forPowerCycle);

		_paused = false;

		if(!forPowerCycle) {
			string modelName = _region == ConsoleRegion::Pal ? "PAL" : "NTSC";
			string messageTitle = MessageManager::Localize("GameLoaded") + " (" + modelName + ")";
			MessageManager::DisplayMessage(messageTitle, FolderUtilities::GetFilename(GetRomInfo().RomFile.GetFileName(), false));
		}

		if(stopRom) {
			#ifndef LIBRETRO
			_emuThread.reset(new thread(&Console::Run, this));
			#endif
		}
		result = true;
	} else {
		MessageManager::DisplayMessage("Error", "CouldNotLoadFile", romFile.GetFileName());
	}

	_settings->SetEmulationConfig(orgConfig);
	return result;
}

RomInfo Console::GetRomInfo()
{
	shared_ptr<BaseCartridge> cart = _cart;
	if(cart) {
		return cart->GetRomInfo();
	} else {
		return {};
	}
}

uint32_t Console::GetMasterClockRate()
{
	return _masterClockRate;
}

ConsoleRegion Console::GetRegion()
{
	return _region;
}

void Console::UpdateRegion()
{
	switch(_settings->GetEmulationConfig().Region) {
		case ConsoleRegion::Auto: _region = _cart->GetRegion(); break;

		default:
		case ConsoleRegion::Ntsc: _region = ConsoleRegion::Ntsc; break;
		case ConsoleRegion::Pal: _region = ConsoleRegion::Pal; break;
	}

	_masterClockRate = _region == ConsoleRegion::Pal ? 21281370 : 21477270;
}

double Console::GetFps()
{
	if(_region == ConsoleRegion::Ntsc) {
		return _settings->GetVideoConfig().IntegerFpsMode ? 60.0 : 60.098812;
	} else {
		return _settings->GetVideoConfig().IntegerFpsMode ? 50.0 : 50.006978;
	}
}

double Console::GetFrameDelay()
{
	uint32_t emulationSpeed = _settings->GetEmulationSpeed();
	double frameDelay;
	if(emulationSpeed == 0) {
		frameDelay = 0;
	} else {
		UpdateRegion();
		switch(_region) {
			default:
			case ConsoleRegion::Ntsc: frameDelay = _settings->GetVideoConfig().IntegerFpsMode ? 16.6666666666666666667 : 16.63926405550947; break;
			case ConsoleRegion::Pal: frameDelay = _settings->GetVideoConfig().IntegerFpsMode ? 20 : 19.99720882631146; break;
		}
		frameDelay /= (emulationSpeed / 100.0);
	}
	return frameDelay;
}

void Console::PauseOnNextFrame()
{
	shared_ptr<Debugger> debugger = _debugger;
	if(debugger) {
		debugger->Step(CpuType::Cpu, 240, StepType::SpecificScanline);
	} else {
		_pauseOnNextFrame = true;
		_paused = false;
	}
}

void Console::Pause()
{
	shared_ptr<Debugger> debugger = _debugger;
	if(debugger) {
		debugger->Step(CpuType::Cpu, 1, StepType::Step);
	} else {
		_paused = true;
	}
}

void Console::Resume()
{
	shared_ptr<Debugger> debugger = _debugger;
	if(debugger) {
		debugger->Run();
	} else {
		_paused = false;
	}
}

bool Console::IsPaused()
{
	shared_ptr<Debugger> debugger = _debugger;
	if(debugger) {
		return debugger->IsExecutionStopped();
	} else {
		return _paused;
	}
}

void Console::WaitForPauseEnd()
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
	if(!_stopFlag) {
		_runLock.Acquire();
		_notificationManager->SendNotification(ConsoleNotificationType::GameResumed);
	}
}

ConsoleLock Console::AcquireLock()
{
	return ConsoleLock(this);
}

void Console::Lock()
{
	shared_ptr<Debugger> debugger = _debugger;
	if(debugger) {
		debugger->SuspendDebugger(false);
	}

	_lockCounter++;
	_runLock.Acquire();
}

void Console::Unlock()
{
	shared_ptr<Debugger> debugger = _debugger;
	if(debugger) {
		debugger->SuspendDebugger(true);
	}

	_runLock.Release();
	_lockCounter--;
}

void Console::WaitForLock()
{
	if(_lockCounter > 0) {
		//Need to temporarely pause the emu (to save/load a state, etc.)
		_runLock.Release();

		//Spin wait until we are allowed to start again
		while(_lockCounter > 0) {}

		_runLock.Acquire();
	}
}

void Console::Serialize(ostream &out, int compressionLevel)
{
	Serializer serializer(SaveStateManager::FileFormatVersion);
	serializer.Stream(_cpu.get());
	serializer.Stream(_memoryManager.get());
	serializer.Stream(_ppu.get());
	serializer.Stream(_dmaController.get());
	serializer.Stream(_internalRegisters.get());
	serializer.Stream(_cart.get());
	serializer.Stream(_controlManager.get());
	serializer.Stream(_spc.get());
	if(_msu1) {
		serializer.Stream(_msu1.get());
	}
	serializer.Save(out, compressionLevel);
}

void Console::Deserialize(istream &in, uint32_t fileFormatVersion, bool compressed)
{
	Serializer serializer(in, fileFormatVersion, compressed);
	serializer.Stream(_cpu.get());
	serializer.Stream(_memoryManager.get());
	serializer.Stream(_ppu.get());
	serializer.Stream(_dmaController.get());
	serializer.Stream(_internalRegisters.get());
	serializer.Stream(_cart.get());
	serializer.Stream(_controlManager.get());
	serializer.Stream(_spc.get());
	if(_msu1) {
		serializer.Stream(_msu1.get());
	}
	_notificationManager->SendNotification(ConsoleNotificationType::StateLoaded);
}

shared_ptr<SoundMixer> Console::GetSoundMixer()
{
	return _soundMixer;
}

shared_ptr<VideoRenderer> Console::GetVideoRenderer()
{
	return _videoRenderer;
}

shared_ptr<VideoDecoder> Console::GetVideoDecoder()
{
	return _videoDecoder;
}

shared_ptr<NotificationManager> Console::GetNotificationManager()
{
	return _notificationManager;
}

shared_ptr<EmuSettings> Console::GetSettings()
{
	return _settings;
}

shared_ptr<SaveStateManager> Console::GetSaveStateManager()
{
	return _saveStateManager;
}

shared_ptr<RewindManager> Console::GetRewindManager()
{
	return _rewindManager;
}

shared_ptr<DebugHud> Console::GetDebugHud()
{
	return _debugHud;
}

shared_ptr<BatteryManager> Console::GetBatteryManager()
{
	return _batteryManager;
}

shared_ptr<CheatManager> Console::GetCheatManager()
{
	return _cheatManager;
}

shared_ptr<MovieManager> Console::GetMovieManager()
{
	return _movieManager;
}

shared_ptr<Cpu> Console::GetCpu()
{
	return _cpu;
}

shared_ptr<Ppu> Console::GetPpu()
{
	return _ppu;
}

shared_ptr<Spc> Console::GetSpc()
{
	return _spc;
}

shared_ptr<BaseCartridge> Console::GetCartridge()
{
	return _cart;
}

shared_ptr<MemoryManager> Console::GetMemoryManager()
{
	return _memoryManager;
}

shared_ptr<InternalRegisters> Console::GetInternalRegisters()
{
	return _internalRegisters;
}

shared_ptr<ControlManager> Console::GetControlManager()
{
	return _controlManager;
}

shared_ptr<DmaController> Console::GetDmaController()
{
	return _dmaController;
}

shared_ptr<Msu1> Console::GetMsu1()
{
	return _msu1;
}

shared_ptr<Debugger> Console::GetDebugger(bool autoStart)
{
	shared_ptr<Debugger> debugger = _debugger;
	if(!debugger && autoStart) {
		//Lock to make sure we don't try to start debuggers in 2 separate threads at once
		auto lock = _debuggerLock.AcquireSafe();
		debugger = _debugger;
		if(!debugger) {
			debugger.reset(new Debugger(shared_from_this()));
			_debugger = debugger;
		}
	}
	return debugger;
}

void Console::StopDebugger()
{
	//Pause/unpause the regular emulation thread based on the debugger's pause state
	_paused = IsPaused();

	shared_ptr<Debugger> debugger = _debugger;
	debugger->SuspendDebugger(false);
	Lock();
	_debugger.reset();

	Unlock();
}

bool Console::IsDebugging()
{
	return _debugger != nullptr;
}

thread::id Console::GetEmulationThreadId()
{
	return _emulationThreadId;
}

bool Console::IsRunning()
{
	return _cpu != nullptr;
}

bool Console::IsRunAheadFrame()
{
	return _isRunAheadFrame;
}

template<CpuType type>
void Console::ProcessMemoryRead(uint32_t addr, uint8_t value, MemoryOperationType opType)
{
	if(_debugger) {
		_debugger->ProcessMemoryRead<type>(addr, value, opType);
	}
}

template<CpuType type>
void Console::ProcessMemoryWrite(uint32_t addr, uint8_t value, MemoryOperationType opType)
{
	if(_debugger) {
		_debugger->ProcessMemoryWrite<type>(addr, value, opType);
	}
}

void Console::ProcessPpuRead(uint32_t addr, uint8_t value, SnesMemoryType memoryType)
{
	if(_debugger) {
		_debugger->ProcessPpuRead(addr, value, memoryType);
	}
}

void Console::ProcessPpuWrite(uint32_t addr, uint8_t value, SnesMemoryType memoryType)
{
	if(_debugger) {
		_debugger->ProcessPpuWrite(addr, value, memoryType);
	}
}

void Console::ProcessWorkRamRead(uint32_t addr, uint8_t value)
{
	if(_debugger) {
		_debugger->ProcessWorkRamRead(addr, value);
	}
}

void Console::ProcessWorkRamWrite(uint32_t addr, uint8_t value)
{
	if(_debugger) {
		_debugger->ProcessWorkRamWrite(addr, value);
	}
}

void Console::ProcessNecDspExec(uint32_t addr, uint32_t value)
{
	if(_debugger) {
		_debugger->ProcessNecDspExec(addr, value);
	}
}

void Console::ProcessCx4Exec()
{
	if(_debugger) {
		_debugger->ProcessCx4Exec();
	}
}

void Console::ProcessPpuCycle()
{
	if(_debugger) {
		_debugger->ProcessPpuCycle();
	}
}

template<CpuType type>
void Console::ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi)
{
	if(_debugger) {
		_debugger->ProcessInterrupt<type>(originalPc, currentPc, forNmi);
	}
}

void Console::ProcessEvent(EventType type)
{
	if(type == EventType::EndFrame && _spcHud) {
		_spcHud->Draw(_ppu->GetFrameCount());
	}

	if(_debugger) {
		_debugger->ProcessEvent(type);
	}
}

template void Console::ProcessMemoryRead<CpuType::Cpu>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Console::ProcessMemoryRead<CpuType::Sa1>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Console::ProcessMemoryRead<CpuType::Spc>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Console::ProcessMemoryRead<CpuType::Gsu>(uint32_t addr, uint8_t value, MemoryOperationType opType);

template void Console::ProcessMemoryWrite<CpuType::Cpu>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Console::ProcessMemoryWrite<CpuType::Sa1>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Console::ProcessMemoryWrite<CpuType::Spc>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Console::ProcessMemoryWrite<CpuType::Gsu>(uint32_t addr, uint8_t value, MemoryOperationType opType);

template void Console::ProcessInterrupt<CpuType::Cpu>(uint32_t originalPc, uint32_t currentPc, bool forNmi);
template void Console::ProcessInterrupt<CpuType::Sa1>(uint32_t originalPc, uint32_t currentPc, bool forNmi);