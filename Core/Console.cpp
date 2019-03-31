#include "stdafx.h"
#include "Console.h"
#include "Cpu.h"
#include "Ppu.h"
#include "Spc.h"
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
#include "../Utilities/Serializer.h"
#include "../Utilities/Timer.h"
#include "../Utilities/VirtualFile.h"
#include "../Utilities/PlatformUtilities.h"
#include "../Utilities/FolderUtilities.h"

Console::~Console()
{
}

void Console::Initialize()
{
	_lockCounter = 0;

	_settings.reset(new EmuSettings());
	_notificationManager.reset(new NotificationManager());
	_videoDecoder.reset(new VideoDecoder(shared_from_this()));
	_videoRenderer.reset(new VideoRenderer(shared_from_this()));
	_saveStateManager.reset(new SaveStateManager(shared_from_this()));
	_soundMixer.reset(new SoundMixer(this));
	_debugHud.reset(new DebugHud());

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
}

void Console::Run()
{
	if(!_cpu) {
		return;
	}

	DebugStats stats(this);
	Timer lastFrameTimer;
	_stopFlag = false;
	uint32_t previousFrameCount = 0;
	
	double frameDelay = GetFrameDelay();
	FrameLimiter frameLimiter(frameDelay);

	PlatformUtilities::EnableHighResolutionTimer();

	_videoDecoder->StartThread();
	_emulationThreadId = std::this_thread::get_id();

	auto lock = _runLock.AcquireSafe();
	while(!_stopFlag) {
		_cpu->Exec();

		if(previousFrameCount != _ppu->GetFrameCount()) {
			_rewindManager->ProcessEndOfFrame();

			WaitForLock();

			if(_paused && !_stopFlag && !_debugger) {
				WaitForPauseEnd();
				if(_stopFlag) {
					break;
				}
			}

			frameLimiter.ProcessFrame();
			frameLimiter.WaitForNextFrame();

			_controlManager->UpdateControlDevices();

			double newFrameDelay = GetFrameDelay();
			if(newFrameDelay != frameDelay) {
				frameDelay = newFrameDelay;
				frameLimiter.SetDelay(frameDelay);
			}
			
			PreferencesConfig cfg = _settings->GetPreferences();
			if(cfg.ShowDebugInfo) {
				double lastFrameTime = lastFrameTimer.GetElapsedMS();
				lastFrameTimer.Reset();
				stats.DisplayStats(lastFrameTime);
			}

			previousFrameCount = _ppu->GetFrameCount();
		}
	}

	_emulationThreadId = thread::id();

	PlatformUtilities::RestoreTimerResolution();
}

void Console::Stop(bool sendNotification)
{
	_stopFlag = true;

	shared_ptr<Debugger> debugger = _debugger;
	if(debugger) {
		debugger->Run();
	}

	_runLock.WaitForRelease();

	if(_cart) {
		//TODO IPS/BPS patch support
		_saveStateManager->SaveRecentGame(GetRomInfo().RomFile.GetFileName(), _cart->GetRomInfo().RomFile, "");
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

	_soundMixer->StopAudio();

	if(sendNotification) {
		_notificationManager->SendNotification(ConsoleNotificationType::EmulationStopped);
	}
}

void Console::Reset()
{
	shared_ptr<Debugger> debugger = _debugger;
	if(debugger) {
		debugger->Run();
	}

	Lock();

	_dmaController->Reset();
	_internalRegisters->Reset();
	_memoryManager->Reset();
	_spc->Reset();
	_cpu->Reset();
	_ppu->Reset();
	//_cart->Reset();
	//_controlManager->Reset();

	if(debugger) {
		debugger->Step(1);
	}
	Unlock();
}

void Console::PowerCycle()
{
	shared_ptr<BaseCartridge> cart = _cart;
	if(cart) {
		shared_ptr<Debugger> debugger = _debugger;
		if(debugger) {
			debugger->Run();
		}

		RomInfo info = cart->GetRomInfo();
		Lock();
		LoadRom(info.RomFile, info.PatchFile, false);
		Unlock();
	}
}

bool Console::LoadRom(VirtualFile romFile, VirtualFile patchFile, bool stopRom)
{
	if(_cart) {
		//Make sure the battery is saved to disk before we load another game (or reload the same game)
		_cart->SaveBattery();
	}

	shared_ptr<BaseCartridge> cart = BaseCartridge::CreateCartridge(romFile, patchFile);
	if(cart) {
		if(stopRom) {
			Stop(false);
		}

		vector<uint8_t> spcRomData;
		VirtualFile spcBios(FolderUtilities::CombinePath(FolderUtilities::GetHomeFolder(), "spc700.rom"));
		if(spcBios.IsValid()) {
			spcBios.ReadFile(spcRomData);
		} else {
			MessageManager::DisplayMessage("SPC", "spc700.rom not found, cannot launch game.");
			return false;
		}
		
		_cart = cart;
		UpdateRegion();

		_internalRegisters.reset(new InternalRegisters(shared_from_this()));
		_ppu.reset(new Ppu(shared_from_this()));
		_controlManager.reset(new ControlManager(shared_from_this()));
		_memoryManager.reset(new MemoryManager());
		_dmaController.reset(new DmaController(_memoryManager.get()));
		_spc.reset(new Spc(shared_from_this(), spcRomData));

		_memoryManager->Initialize(shared_from_this());

		_cpu.reset(new Cpu(this));
		
		if(_debugger) {
			//Reset debugger if it was running before
			auto lock = _debuggerLock.AcquireSafe();
			_debugger.reset();
			GetDebugger();
			_debugger->Step(1);
		}

		_cpu->PowerOn();
		_memoryManager->IncrementMasterClockValue<170>();

		_rewindManager.reset(new RewindManager(shared_from_this()));
		_notificationManager->RegisterNotificationListener(_rewindManager);

		_controlManager->UpdateControlDevices();
				
		UpdateRegion();

		_paused = false;
		_notificationManager->SendNotification(ConsoleNotificationType::GameLoaded);

		string modelName = _region == ConsoleRegion::Pal ? "PAL" : "NTSC";
		string messageTitle = MessageManager::Localize("GameLoaded") + " (" + modelName + ")";
		MessageManager::DisplayMessage(messageTitle, FolderUtilities::GetFilename(GetRomInfo().RomFile.GetFileName(), false));

		return true;
	}

	return false;
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
		case ConsoleRegion::Auto:
		{
			uint8_t destCode = _cart->GetRomInfo().Header.DestinationCode;
			if((destCode >= 0x02 && destCode <= 0x0C) || destCode == 0x11) {
				_region = ConsoleRegion::Pal;
			} else {
				_region = ConsoleRegion::Ntsc;
			}
			break;
		}

		default:
		case ConsoleRegion::Ntsc: _region = ConsoleRegion::Ntsc; break;
		case ConsoleRegion::Pal: _region = ConsoleRegion::Pal; break;
	}

	_masterClockRate = _region == ConsoleRegion::Pal ? 21281370 : 21477270;
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

void Console::Pause()
{
	shared_ptr<Debugger> debugger = _debugger;
	if(debugger) {
		debugger->Step(1);
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
	_runLock.Acquire();
	_notificationManager->SendNotification(ConsoleNotificationType::GameResumed);
}

ConsoleLock Console::AcquireLock()
{
	return ConsoleLock(this);
}

void Console::Lock()
{
	_lockCounter++;
	_runLock.Acquire();
}

void Console::Unlock()
{
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

void Console::Serialize(ostream &out)
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
	serializer.Save(out);
}

void Console::Deserialize(istream &in, uint32_t fileFormatVersion)
{
	Serializer serializer(in, fileFormatVersion);
	serializer.Stream(_cpu.get());
	serializer.Stream(_memoryManager.get());
	serializer.Stream(_ppu.get());
	serializer.Stream(_dmaController.get());
	serializer.Stream(_internalRegisters.get());
	serializer.Stream(_cart.get());
	serializer.Stream(_controlManager.get());
	serializer.Stream(_spc.get());

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
	_debugger.reset();
}

thread::id Console::GetEmulationThreadId()
{
	return _emulationThreadId;
}

bool Console::IsRunning()
{
	return _cpu != nullptr;
}

void Console::ProcessCpuRead(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	if(_debugger) {
		_debugger->ProcessCpuRead(addr, value, type);
	}
}

void Console::ProcessCpuWrite(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	if(_debugger) {
		_debugger->ProcessCpuWrite(addr, value, type);
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

void Console::ProcessSpcRead(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	if(_debugger) {
		_debugger->ProcessSpcRead(addr, value, type);
	}
}

void Console::ProcessSpcWrite(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	if(_debugger) {
		_debugger->ProcessSpcWrite(addr, value, type);
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

void Console::ProcessPpuCycle()
{
	if(_debugger) {
		_debugger->ProcessPpuCycle();
	}
}

void Console::ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi)
{
	if(_debugger) {
		_debugger->ProcessInterrupt(originalPc, currentPc, forNmi);
	}
}

void Console::ProcessEvent(EventType type)
{
	if(_debugger) {
		_debugger->ProcessEvent(type);
	}
}