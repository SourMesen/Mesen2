#include "stdafx.h"
#include "Console.h"
#include "Cpu.h"
#include "Ppu.h"
#include "Spc.h"
#include "InternalRegisters.h"
#include "MemoryManager.h"
#include "Debugger.h"
#include "NotificationManager.h"
#include "SoundMixer.h"
#include "VideoDecoder.h"
#include "VideoRenderer.h"
#include "DebugHud.h"
#include "MessageManager.h"
#include "../Utilities/Timer.h"
#include "../Utilities/VirtualFile.h"

void Console::Initialize()
{
	_notificationManager.reset(new NotificationManager());
	_videoDecoder.reset(new VideoDecoder(shared_from_this()));
	_videoRenderer.reset(new VideoRenderer(shared_from_this()));
	_soundMixer.reset(new SoundMixer());
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
}

void Console::Run()
{
	if(!_cpu) {
		return;
	}

	_stopFlag = false;

	auto lock = _runLock.AcquireSafe();
	while(!_stopFlag) {
		_cpu->Exec();
	}

	//Timer timer;
	/*std::cout << "Time: " << std::to_string(timer.GetElapsedMS()) << std::endl;
	std::cout << "OP Count: " << std::to_string(_cpu->opCount) << std::endl;
	std::cout << "OP/sec: " << std::to_string(_cpu->opCount * 1000 / timer.GetElapsedMS()) << std::endl;
	while(true);*/
}

void Console::Stop()
{
	_stopFlag = true;

	shared_ptr<Debugger> debugger = _debugger;
	if(debugger) {
		debugger->Run();
	}

	_runLock.WaitForRelease();

	_cpu.reset();
	_ppu.reset();
	_spc.reset();
	_cart.reset();
	_internalRegisters.reset();
	_memoryManager.reset();
}

void Console::LoadRom(VirtualFile romFile, VirtualFile patchFile)
{
	Stop();

	shared_ptr<BaseCartridge> cart = BaseCartridge::CreateCartridge(romFile, patchFile);
	if(cart) {
		MessageManager::ClearLog();
		_internalRegisters.reset(new InternalRegisters());
		_ppu.reset(new Ppu(shared_from_this()));
		_spc.reset(new Spc(shared_from_this()));
		_cart = cart;
		_memoryManager.reset(new MemoryManager());
		_memoryManager->Initialize(shared_from_this());

		_cpu.reset(new Cpu(_memoryManager));

		//if(_debugger) {
			//Reset debugger if it was running before
			auto lock = _debuggerLock.AcquireSafe();
			_debugger.reset();
			GetDebugger();
		//}
	}
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
