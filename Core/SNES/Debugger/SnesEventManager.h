#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/BaseEventManager.h"
#include "Utilities/SimpleLock.h"

enum class DebugEventType;
struct DebugEventInfo;
struct EventViewerDisplayOptions;
class SnesCpu;
class SnesPpu;
class Debugger;
class SnesDmaController;
class SnesMemoryManager;

struct SnesEventViewerConfig : public BaseEventViewerConfig
{
	EventViewerCategoryCfg Irq;
	EventViewerCategoryCfg Nmi;
	EventViewerCategoryCfg MarkedBreakpoints;

	EventViewerCategoryCfg PpuRegisterReads;
	EventViewerCategoryCfg PpuRegisterCgramWrites;
	EventViewerCategoryCfg PpuRegisterVramWrites;
	EventViewerCategoryCfg PpuRegisterOamWrites;
	EventViewerCategoryCfg PpuRegisterMode7Writes;
	EventViewerCategoryCfg PpuRegisterBgOptionWrites;
	EventViewerCategoryCfg PpuRegisterBgScrollWrites;
	EventViewerCategoryCfg PpuRegisterWindowWrites;
	EventViewerCategoryCfg PpuRegisterOtherWrites;

	EventViewerCategoryCfg ApuRegisterReads;
	EventViewerCategoryCfg ApuRegisterWrites;
	EventViewerCategoryCfg CpuRegisterReads;
	EventViewerCategoryCfg CpuRegisterWrites;
	EventViewerCategoryCfg WorkRamRegisterReads;
	EventViewerCategoryCfg WorkRamRegisterWrites;

	bool ShowPreviousFrameEvents;
	uint8_t ShowDmaChannels[8];
};

class SnesEventManager final : public BaseEventManager
{
private:
	static constexpr int ScanlineWidth = 1364 / 2;

	SnesEventViewerConfig _config;

	SnesCpu * _cpu;
	SnesPpu *_ppu;
	SnesMemoryManager* _memoryManager;
	SnesDmaController *_dmaController;
	Debugger *_debugger;

	bool _overscanMode = false;
	bool _useHighResOutput = false;
	uint32_t _scanlineCount = 262;
	uint16_t *_ppuBuffer = nullptr;

protected:
	void ConvertScanlineCycleToRowColumn(int32_t& x, int32_t& y) override;
	void DrawScreen(uint32_t* buffer) override;
	bool ShowPreviousFrameEvents() override;

public:
	SnesEventManager(Debugger *debugger, SnesCpu *cpu, SnesPpu *ppu, SnesMemoryManager *memoryManager, SnesDmaController *dmaController);
	~SnesEventManager();

	void AddEvent(DebugEventType type, MemoryOperationInfo &operation, int32_t breakpointId = -1) override;
	void AddEvent(DebugEventType type) override;
	
	EventViewerCategoryCfg GetEventConfig(DebugEventInfo& evt) override;

	uint32_t TakeEventSnapshot(bool forAutoRefresh) override;
	FrameInfo GetDisplayBufferSize() override;
	DebugEventInfo GetEvent(uint16_t y, uint16_t x) override;

	void SetConfiguration(BaseEventViewerConfig& config) override;
};
