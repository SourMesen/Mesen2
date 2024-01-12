#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/BaseEventManager.h"
#include "Utilities/SimpleLock.h"

enum class DebugEventType;
struct DebugEventInfo;
class SmsCpu;
class SmsVdp;
class SmsConsole;
class Debugger;

struct SmsEventViewerConfig : public BaseEventViewerConfig
{
	EventViewerCategoryCfg Irq;
	EventViewerCategoryCfg Nmi;
	EventViewerCategoryCfg MarkedBreakpoints;

	EventViewerCategoryCfg VdpPaletteWrite;
	EventViewerCategoryCfg VdpVramWrite;

	EventViewerCategoryCfg VdpVCounterRead;
	EventViewerCategoryCfg VdpHCounterRead;
	EventViewerCategoryCfg VdpVramRead;
	EventViewerCategoryCfg VdpControlPortRead;
	EventViewerCategoryCfg VdpControlPortWrite;

	EventViewerCategoryCfg PsgWrite;
	EventViewerCategoryCfg IoWrite;
	EventViewerCategoryCfg IoRead;
	
	EventViewerCategoryCfg MemoryControlWrite;

	EventViewerCategoryCfg GameGearPortWrite;
	EventViewerCategoryCfg GameGearPortRead;

	bool ShowPreviousFrameEvents;
};

class SmsEventManager final : public BaseEventManager
{
private:
	static constexpr int ScanlineWidth = 342*2;
	static constexpr int ScreenHeight = 262;

	SmsEventViewerConfig _config;

	SmsVdp* _vdp;
	SmsCpu* _cpu;
	SmsConsole* _console;
	Debugger* _debugger;

	uint32_t _scanlineCount = 262;
	uint32_t _visibleScanlineCount = 192;
	uint16_t* _ppuBuffer = nullptr;

protected:
	bool ShowPreviousFrameEvents() override;
	void ConvertScanlineCycleToRowColumn(int32_t& x, int32_t& y) override;
	void DrawScreen(uint32_t* buffer) override;

public:
	SmsEventManager(Debugger* debugger, SmsConsole* console, SmsCpu* cpu, SmsVdp* vdp);
	~SmsEventManager();

	void AddEvent(DebugEventType type, MemoryOperationInfo& operation, int32_t breakpointId = -1) override;
	void AddEvent(DebugEventType type) override;

	EventViewerCategoryCfg GetEventConfig(DebugEventInfo& evt) override;

	uint32_t TakeEventSnapshot(bool forAutoRefresh) override;
	DebugEventInfo GetEvent(uint16_t y, uint16_t x) override;

	FrameInfo GetDisplayBufferSize() override;
	void SetConfiguration(BaseEventViewerConfig& config) override;
};
