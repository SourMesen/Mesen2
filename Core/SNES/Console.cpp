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
#include "Gameboy/Gameboy.h"
#include "Gameboy/GbPpu.h"
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
#include "MovieManager.h"
#include "BatteryManager.h"
#include "CheatManager.h"
#include "MovieManager.h"
#include "SystemActionManager.h"
#include "SpcHud.h"
#include "Msu1.h"
#include "Emulator.h"
#include "IControlManager.h"
#include "Utilities/Serializer.h"
#include "Utilities/Timer.h"
#include "Utilities/VirtualFile.h"
#include "Utilities/PlatformUtilities.h"
#include "Utilities/FolderUtilities.h"

Console::Console(Emulator* emu)
{
	_emu = emu;
	_settings = emu->GetSettings();
}

Console::~Console()
{
}

void Console::Initialize()
{
}

void Console::Release()
{
}

void Console::RunFrame()
{
	_frameRunning = true;

	while(_frameRunning) {
		_cpu->Exec();
	}
}

void Console::OnBeforeRun()
{
	_memoryManager->IncMasterClockStartup();
	_controlManager->UpdateInputState();
}

bool Console::ProcessSystemActions()
{
	if(_controlManager->GetSystemActionManager()->IsResetPressed()) {
		_emu->Reset();
		return true;
	} else if(_controlManager->GetSystemActionManager()->IsPowerCyclePressed()) {
		_emu->PowerCycle();
		return true;
	}
	return false;
}

void Console::ProcessEndOfFrame()
{
#ifndef LIBRETRO
	_cart->RunCoprocessors();
	if(_cart->GetCoprocessor()) {
		_cart->GetCoprocessor()->ProcessEndOfFrame();
	}

	_emu->ProcessEndOfFrame();

	_controlManager->UpdateInputState();
	_controlManager->UpdateControlDevices();
	_internalRegisters->ProcessAutoJoypadRead();
#endif
	_frameRunning = false;
}

void Console::RunSingleFrame()
{
	//Used by Libretro
	/*_emulationThreadId = std::this_thread::get_id();
	_isRunAheadFrame = false;

	_controlManager->UpdateInputState();
	_internalRegisters->ProcessAutoJoypadRead();

	RunFrame();

	_cart->RunCoprocessors();
	if(_cart->GetCoprocessor()) {
		_cart->GetCoprocessor()->ProcessEndOfFrame();
	}

	_controlManager->UpdateControlDevices();*/
}

void Console::Stop()
{
	_cpu.reset();
	_ppu.reset();
	_spc.reset();
	_cart.reset();
	_internalRegisters.reset();
	_controlManager.reset();
	_memoryManager.reset();
	_dmaController.reset();
	_msu1.reset();
}

void Console::Reset()
{
	_dmaController->Reset();
	_internalRegisters->Reset();
	_memoryManager->Reset();
	_spc->Reset();
	_ppu->Reset();
	_cart->Reset();
	//_controlManager->Reset();

	//Reset cart before CPU to ensure correct memory mappings when fetching reset vector
	_cpu->Reset();

	if(_cart->GetSpcData()) {
		_spc->LoadSpcFile(_cart->GetSpcData());
		_spcHud.reset(new SpcHud(_emu, _cart->GetSpcData()));
	} else {
		_spcHud.reset();
	}
}

bool Console::LoadRom(VirtualFile& romFile, VirtualFile& patchFile)
{
	bool result = false;
	EmulationConfig orgConfig = _settings->GetEmulationConfig(); //backup emulation config (can be temporarily overriden to control the power on RAM state)
	shared_ptr<BaseCartridge> cart = BaseCartridge::CreateCartridge(this, romFile, patchFile);
	if(cart) {
		_cart = cart;
		
		UpdateRegion();

		_internalRegisters.reset(new InternalRegisters());
		_memoryManager.reset(new MemoryManager());
		_ppu.reset(new Ppu(_emu, this));
		_controlManager.reset(new ControlManager(this));
		_dmaController.reset(new DmaController(_memoryManager.get()));
		_spc.reset(new Spc(this));

		_msu1.reset(Msu1::Init(romFile, _spc.get()));

		if(_cart->GetSpcData()) {
			_spc->LoadSpcFile(_cart->GetSpcData());
			_spcHud.reset(new SpcHud(_emu, _cart->GetSpcData()));
		} else {
			_spcHud.reset();
		}

		_cpu.reset(new Cpu(this));
		_memoryManager->Initialize(this);
		_internalRegisters->Initialize(this);

		_ppu->PowerOn();
		_cpu->PowerOn();

		_controlManager->UpdateControlDevices();
				
		UpdateRegion();

		result = true;
	}

	_settings->SetEmulationConfig(orgConfig);
	return result;
}

void Console::Init()
{
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

uint64_t Console::GetMasterClock()
{
	return _memoryManager->GetMasterClock();
}

uint32_t Console::GetMasterClockRate()
{
	return _masterClockRate;
}

ConsoleRegion Console::GetRegion()
{
	return _region;
}

ConsoleType Console::GetConsoleType()
{
	return ConsoleType::Snes;
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
		return _settings->GetVideoConfig().IntegerFpsMode ? 60.0 : 60.0988118623484;
	} else {
		return _settings->GetVideoConfig().IntegerFpsMode ? 50.0 : 50.00697796826829;
	}
}

PpuFrameInfo Console::GetPpuFrame()
{
	//TODO
	return PpuFrameInfo();
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

void Console::Serialize(Serializer& s)
{
	s.Stream(_cpu.get());
	s.Stream(_memoryManager.get());
	s.Stream(_ppu.get());
	s.Stream(_dmaController.get());
	s.Stream(_internalRegisters.get());
	s.Stream(_cart.get());
	s.Stream(_controlManager.get());
	s.Stream(_spc.get());
	if(_msu1) {
		s.Stream(_msu1.get());
	}
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

shared_ptr<IControlManager> Console::GetControlManager()
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

Emulator* Console::GetEmulator()
{
	return _emu;
}

bool Console::IsRunning()
{
	return _cpu != nullptr;
}

bool Console::IsRunAheadFrame()
{
	return _isRunAheadFrame;
}
