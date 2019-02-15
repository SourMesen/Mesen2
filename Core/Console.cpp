#include "stdafx.h"
#include "Console.h"
#include "Cpu.h"
#include "MemoryManager.h"
#include "Debugger.h"
#include "VideoDecoder.h"
#include "VideoRenderer.h"
#include "DebugHud.h"
#include "../Utilities/Timer.h"
#include "../Utilities/VirtualFile.h"

void Console::Initialize()
{
	_videoDecoder.reset(new VideoDecoder(shared_from_this()));
	_videoRenderer.reset(new VideoRenderer(shared_from_this()));
	_debugHud.reset(new DebugHud());

	_videoDecoder->StartThread();
	_videoRenderer->StartThread();
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
	_runLock.WaitForRelease();

	_cpu.reset();
	_ppu.reset();
	_memoryManager.reset();
	_debugger.reset();
}

void Console::LoadRom(VirtualFile romFile, VirtualFile patchFile)
{
	Stop();

	shared_ptr<BaseCartridge> cart = BaseCartridge::CreateCartridge(romFile, patchFile);
	if(cart) {
		_ppu.reset(new Ppu(shared_from_this()));
		
		_memoryManager.reset(new MemoryManager());
		_memoryManager->Initialize(cart, shared_from_this());

		_cpu.reset(new Cpu(_memoryManager));
		_debugger.reset(new Debugger(_cpu, _ppu, _memoryManager));
	}
}

shared_ptr<VideoRenderer> Console::GetVideoRenderer()
{
	return _videoRenderer;
}

shared_ptr<VideoDecoder> Console::GetVideoDecoder()
{
	return _videoDecoder;
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

shared_ptr<MemoryManager> Console::GetMemoryManager()
{
	return _memoryManager;
}

shared_ptr<Debugger> Console::GetDebugger(bool allowStart)
{
	return _debugger;
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
