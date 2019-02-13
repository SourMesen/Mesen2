#include "stdafx.h"
#include "Console.h"
#include "Cpu.h"
#include "MemoryManager.h"
#include "Debugger.h"
#include "../Utilities/Timer.h"
#include "../Utilities/VirtualFile.h"

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
	_memoryManager.reset();
	_debugger.reset();
}

void Console::LoadRom(VirtualFile romFile, VirtualFile patchFile)
{
	Stop();

	shared_ptr<BaseCartridge> cart = BaseCartridge::CreateCartridge(romFile, patchFile);
	if(cart) {
		_memoryManager.reset(new MemoryManager(cart, shared_from_this()));
		_cpu.reset(new Cpu(_memoryManager));
		_debugger.reset(new Debugger(_cpu, _memoryManager));
	}
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
