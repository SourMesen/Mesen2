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
	Stop();

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

			frameLimiter.ProcessFrame();
			frameLimiter.WaitForNextFrame();
			
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

void Console::Stop()
{
	_stopFlag = true;

	shared_ptr<Debugger> debugger = _debugger;
	if(debugger) {
		debugger->Run();
	}

	_runLock.WaitForRelease();

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
}

void Console::LoadRom(VirtualFile romFile, VirtualFile patchFile)
{
	Stop();

	shared_ptr<BaseCartridge> cart = BaseCartridge::CreateCartridge(romFile, patchFile);
	if(cart) {
		vector<uint8_t> spcRomData;
		VirtualFile spcBios(FolderUtilities::CombinePath(FolderUtilities::GetHomeFolder(), "spc700.rom"));
		if(spcBios.IsValid()) {
			spcBios.ReadFile(spcRomData);
		} else {
			MessageManager::Log("[SPC] spc700.rom not found, cannot launch game.");
			return;
		}

		_internalRegisters.reset(new InternalRegisters(shared_from_this()));
		_ppu.reset(new Ppu(shared_from_this()));
		_spc.reset(new Spc(shared_from_this(), spcRomData));
		_cart = cart;
		_controlManager.reset(new ControlManager(shared_from_this()));
		_memoryManager.reset(new MemoryManager());
		_dmaController.reset(new DmaController(_memoryManager.get()));

		_memoryManager->Initialize(shared_from_this());

		_cpu.reset(new Cpu(this));
		_memoryManager->IncrementMasterClockValue<170>();

		_rewindManager.reset(new RewindManager(shared_from_this()));
		_notificationManager->RegisterNotificationListener(_rewindManager);

		//if(_debugger) {
			//Reset debugger if it was running before
			//auto lock = _debuggerLock.AcquireSafe();
			//_debugger.reset();
			//GetDebugger();
		//}

		_notificationManager->SendNotification(ConsoleNotificationType::GameLoaded);
	}
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

double Console::GetFrameDelay()
{
	uint32_t emulationSpeed = _settings->GetEmulationSpeed();
	double frameDelay;
	if(emulationSpeed == 0) {
		frameDelay = 0;
	} else {
		frameDelay = 16.63926405550947 / (emulationSpeed / 100.0);
	}
	return frameDelay;
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

void Console::ProcessEvent(EventType type)
{
	if(_debugger) {
		_debugger->ProcessEvent(type);
	}
}