#pragma once
#include "stdafx.h"
#include "../Utilities/VirtualFile.h"
#include "../Utilities/SimpleLock.h"

class Cpu;
class Ppu;
class BaseCartridge;
class MemoryManager;
class Debugger;
class DebugHud;
class VideoRenderer;
class VideoDecoder;
class NotificationManager;
enum class MemoryOperationType;

class Console : public std::enable_shared_from_this<Console>
{
private:
	shared_ptr<Cpu> _cpu;
	shared_ptr<Ppu> _ppu;
	shared_ptr<MemoryManager> _memoryManager;
	shared_ptr<BaseCartridge> _cart;
	shared_ptr<Debugger> _debugger;

	shared_ptr<NotificationManager> _notificationManager;
	shared_ptr<VideoRenderer> _videoRenderer;
	shared_ptr<VideoDecoder> _videoDecoder;
	shared_ptr<DebugHud> _debugHud;
	
	SimpleLock _runLock;
	atomic<bool> _stopFlag;

public:
	void Initialize();

	void Run();
	void Stop();

	void LoadRom(VirtualFile romFile, VirtualFile patchFile);

	shared_ptr<VideoRenderer> GetVideoRenderer();
	shared_ptr<VideoDecoder> GetVideoDecoder();
	shared_ptr<NotificationManager> GetNotificationManager();

	shared_ptr<DebugHud> GetDebugHud();

	shared_ptr<Cpu> GetCpu();
	shared_ptr<Ppu> GetPpu();
	shared_ptr<BaseCartridge> GetCartridge();
	shared_ptr<MemoryManager> GetMemoryManager();
	shared_ptr<Debugger> GetDebugger(bool allowStart = true);

	void ProcessCpuRead(uint32_t addr, uint8_t value, MemoryOperationType type);
	void ProcessCpuWrite(uint32_t addr, uint8_t value, MemoryOperationType type);
};