#pragma once
#include "stdafx.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/BaseEventManager.h"
#include "PCE/PceConstants.h"

enum class DebugEventType;
struct DebugEventInfo;
struct EventViewerDisplayOptions;
class PceConsole;
class PceCpu;
class PcePpu;
class Debugger;
class PceMemoryManager;

struct PceEventViewerConfig : public BaseEventViewerConfig
{
	EventViewerCategoryCfg Irq;
	EventViewerCategoryCfg MarkedBreakpoints;

	EventViewerCategoryCfg VdcWrites;
	EventViewerCategoryCfg VdcReads;
	EventViewerCategoryCfg VceWrites;
	EventViewerCategoryCfg VceReads;
	EventViewerCategoryCfg PsgWrites;
	EventViewerCategoryCfg PsgReads;
	EventViewerCategoryCfg TimerWrites;
	EventViewerCategoryCfg TimerReads;
	EventViewerCategoryCfg IoWrites;
	EventViewerCategoryCfg IoReads;

	bool ShowPreviousFrameEvents;
};

class PceEventManager final : public BaseEventManager
{
private:
	PceEventViewerConfig _config;

	PceCpu * _cpu;
	PcePpu *_ppu;
	PceMemoryManager* _memoryManager;
	Debugger *_debugger;

	uint16_t* _ppuBuffer = nullptr;
	uint8_t _rowClockDividers[PceConstants::ScreenHeight] = {};

protected:
	void ConvertScanlineCycleToRowColumn(int32_t& x, int32_t& y) override;
	void DrawScreen(uint32_t* buffer) override;
	bool ShowPreviousFrameEvents() override;

public:
	PceEventManager(Debugger *debugger, PceConsole *console);
	~PceEventManager();

	void AddEvent(DebugEventType type, MemoryOperationInfo &operation, int32_t breakpointId = -1);
	void AddEvent(DebugEventType type);
	
	EventViewerCategoryCfg GetEventConfig(DebugEventInfo& evt);

	uint32_t TakeEventSnapshot();
	FrameInfo GetDisplayBufferSize() override;
	DebugEventInfo GetEvent(uint16_t y, uint16_t x);

	void SetConfiguration(BaseEventViewerConfig& config) override;
};
