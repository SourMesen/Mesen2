#pragma once
#include "stdafx.h"
#include "../Utilities/VirtualFile.h"
#include "../Utilities/SimpleLock.h"

class Cpu;
class Ppu;
class MemoryManager;
class Debugger;
enum class MemoryOperationType;

class Console : public std::enable_shared_from_this<Console>
{
private:
	shared_ptr<Cpu> _cpu;
	shared_ptr<Ppu> _ppu;
	shared_ptr<MemoryManager> _memoryManager;
	shared_ptr<Debugger> _debugger;
	
	SimpleLock _runLock;
	atomic<bool> _stopFlag;

public:
	void Run();
	void Stop();

	void LoadRom(VirtualFile romFile, VirtualFile patchFile);

	shared_ptr<Ppu> GetPpu();
	shared_ptr<Debugger> GetDebugger(bool allowStart = true);

	void ProcessCpuRead(uint32_t addr, uint8_t value, MemoryOperationType type);
	void ProcessCpuWrite(uint32_t addr, uint8_t value, MemoryOperationType type);
};