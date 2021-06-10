#pragma once
#include "stdafx.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/IEventManager.h"
#include "Utilities/SimpleLock.h"

enum class DebugEventType;
struct DebugEventInfo;
struct EventViewerDisplayOptions;
class Cpu;
class Ppu;
class Debugger;
class DmaController;
class MemoryManager;

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

class SnesEventManager final : public IEventManager
{
private:
	static constexpr int ScanlineWidth = 1364 / 2;

	SnesEventViewerConfig _config;

	Cpu * _cpu;
	Ppu *_ppu;
	MemoryManager* _memoryManager;
	DmaController *_dmaController;
	Debugger *_debugger;
	vector<DebugEventInfo> _debugEvents;
	vector<DebugEventInfo> _prevDebugEvents;
	vector<DebugEventInfo> _sentEvents;
	
	vector<DebugEventInfo> _snapshot;
	int16_t _snapshotScanline = -1;
	uint16_t _snapshotCycle = 0;
	SimpleLock _lock;

	bool _overscanMode = false;
	bool _useHighResOutput = false;
	uint32_t _scanlineCount = 262;
	uint16_t *_ppuBuffer = nullptr;

	void DrawEvent(DebugEventInfo &evt, bool drawBackground, uint32_t *buffer);
	void FilterEvents();

public:
	SnesEventManager(Debugger *debugger, Cpu *cpu, Ppu *ppu, MemoryManager *memoryManager, DmaController *dmaController);
	~SnesEventManager();

	void AddEvent(DebugEventType type, MemoryOperationInfo &operation, int32_t breakpointId = -1);
	void AddEvent(DebugEventType type);
	
	void GetEvents(DebugEventInfo *eventArray, uint32_t &maxEventCount);
	uint32_t GetEventCount();
	void ClearFrameEvents();

	EventViewerCategoryCfg GetEventConfig(DebugEventInfo& evt);

	uint32_t TakeEventSnapshot();
	FrameInfo GetDisplayBufferSize();
	void GetDisplayBuffer(uint32_t *buffer, uint32_t bufferSize);
	DebugEventInfo GetEvent(uint16_t scanline, uint16_t cycle);

	void SetConfiguration(BaseEventViewerConfig& config) override;
};
