#pragma once 
#include "stdafx.h"

#include "Shared/SettingTypes.h"
#include "Shared/Interfaces/IConsole.h"
#include "Debugger/DebugTypes.h"

class Emulator;
class NesCpu;
class NesPpu;
class NesApu;
class NesMemoryManager;
class NesControlManager;
class BaseMapper;
class EmuSettings;
class NesSoundMixer;

enum class DebugEventType;
enum class EventType;
enum class NesModel;

class NesConsole final : public IConsole, public std::enable_shared_from_this<NesConsole>
{
private:
	Emulator* _emu = nullptr;
	shared_ptr<NesCpu> _cpu;
	shared_ptr<NesPpu> _ppu;
	shared_ptr<NesApu> _apu;
	shared_ptr<NesMemoryManager> _memoryManager;
	shared_ptr<BaseMapper> _mapper;
	shared_ptr<NesControlManager> _controlManager;
	shared_ptr<NesSoundMixer> _mixer;

public:
	NesConsole(Emulator* emulator);

	NesCpu* GetCpu() { return _cpu.get(); }
	NesPpu* GetPpu() { return _ppu.get(); }
	NesApu* GetApu() { return _apu.get(); }
	NesMemoryManager* GetMemoryManager() { return _memoryManager.get(); }
	BaseMapper* GetMapper() { return _mapper.get(); }
	NesSoundMixer* GetSoundMixer() { return _mixer.get(); }
	Emulator* GetEmulator();
	NesConfig& GetNesConfig();

	void ProcessCpuClock();
	void InitializeRam(uint8_t* ram, uint32_t size) {}

	std::thread::id  GetEmulationThreadId() { return std::thread::id(); }

	bool IsNsf() { return false; }

	void DebugAddTrace(char* str) {}
	void DebugAddDebugEvent(DebugEventType eventType) {}
	void DebugProcessEvent(EventType eventType) {}
	void DebugProcessInterrupt(uint16_t originalPc, uint16_t pc, bool forNmi) {}
	void DebugProcessPpuCycle() {}
	void DebugSetLastFramePpuScroll(uint16_t addr, uint8_t xScroll, bool updateHorizontalScrollOnly) {}
	void SetNextFrameOverclockStatus(bool enabled) {}

	// Inherited via IConsole
	virtual void Serialize(Serializer& s) override;
	virtual void Stop() override;
	virtual void Reset() override;
	virtual void OnBeforeRun() override;
	virtual bool LoadRom(VirtualFile& romFile) override;
	virtual void Init() override;
	virtual void RunFrame() override;
	virtual shared_ptr<IControlManager> GetControlManager() override;
	virtual double GetFrameDelay() override;
	virtual double GetFps() override;
	virtual void RunSingleFrame() override;
	virtual PpuFrameInfo GetPpuFrame() override;
	virtual ConsoleType GetConsoleType() override;
	virtual vector<CpuType> GetCpuTypes() override;

	virtual AddressInfo GetAbsoluteAddress(AddressInfo relAddress) override;
	virtual AddressInfo GetRelativeAddress(AddressInfo absAddress, CpuType cpuType) override;

	uint64_t GetMasterClock() override;
	uint32_t GetMasterClockRate() override;
};
