#pragma once 
#include "../stdafx.h"

#include "../SettingTypes.h"

class NesCpu;
class NesPpu;
class NesApu;
class NesMemoryManager;
class BaseMapper;
class EmuSettings;
class NesSoundMixer;

enum class DebugEventType;
enum class EventType;
enum class NesModel;

class NesConsole
{
private:
	shared_ptr<NesCpu> _cpu;
	shared_ptr<NesPpu> _ppu;
	shared_ptr<NesApu> _apu;
	shared_ptr<NesMemoryManager> _memoryManager;
	shared_ptr<BaseMapper> _mapper;
	//shared_ptr<ControlManager> _controlManager;

public:
	NesCpu* GetCpu() { return _cpu.get(); }
	NesPpu* GetPpu() { return _ppu.get(); }
	NesApu* GetApu() { return _apu.get(); }
	NesMemoryManager* GetMemoryManager() { return _memoryManager.get(); }
	BaseMapper* GetMapper() { return _mapper.get(); }
	EmuSettings* GetSettings() { return nullptr; }
	NesSoundMixer* GetSoundMixer() { return nullptr; }
	NesConfig GetNesConfig() { return {}; }

	//TODO
	void ProcessCpuClock() {}
	void InitializeRam(uint8_t* ram, uint32_t size) {}

	std::thread::id  GetEmulationThreadId() { return std::thread::id(); }

	void DebugAddTrace(char* str) {}
	void DebugAddDebugEvent(DebugEventType eventType) {}
	void DebugProcessEvent(EventType eventType) {}
	void DebugProcessInterrupt(uint16_t originalPc, uint16_t pc, bool forNmi) {}
	void DebugProcessPpuCycle() {}
	void DebugSetLastFramePpuScroll(uint16_t addr, uint8_t xScroll, bool updateHorizontalScrollOnly) {}
	void SetNextFrameOverclockStatus(bool enabled) {}
};
