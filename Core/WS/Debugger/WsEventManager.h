#pragma once
#include "pch.h"
#include "WS/WsTypes.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/BaseEventManager.h"
#include "Utilities/SimpleLock.h"

enum class DebugEventType;
struct DebugEventInfo;
class WsCpu;
class WsPpu;
class WsConsole;
class Debugger;

struct WsEventViewerConfig : public BaseEventViewerConfig
{
	EventViewerCategoryCfg PpuPaletteRead;
	EventViewerCategoryCfg PpuPaletteWrite;
	EventViewerCategoryCfg PpuVramRead;
	EventViewerCategoryCfg PpuVramWrite;
	EventViewerCategoryCfg PpuVCounterRead;
	EventViewerCategoryCfg PpuScrollRead;
	EventViewerCategoryCfg PpuScrollWrite;
	EventViewerCategoryCfg PpuWindowRead;
	EventViewerCategoryCfg PpuWindowWrite;
	EventViewerCategoryCfg PpuOtherRead;
	EventViewerCategoryCfg PpuOtherWrite;
	EventViewerCategoryCfg AudioRead;
	EventViewerCategoryCfg AudioWrite;
	EventViewerCategoryCfg SerialRead;
	EventViewerCategoryCfg SerialWrite;
	EventViewerCategoryCfg DmaRead;
	EventViewerCategoryCfg DmaWrite;
	EventViewerCategoryCfg InputRead;
	EventViewerCategoryCfg InputWrite;
	EventViewerCategoryCfg IrqRead;
	EventViewerCategoryCfg IrqWrite;
	EventViewerCategoryCfg TimerRead;
	EventViewerCategoryCfg TimerWrite;
	EventViewerCategoryCfg EepromRead;
	EventViewerCategoryCfg EepromWrite;
	EventViewerCategoryCfg CartRead;
	EventViewerCategoryCfg CartWrite;
	EventViewerCategoryCfg OtherRead;
	EventViewerCategoryCfg OtherWrite;

	EventViewerCategoryCfg Irq;
	EventViewerCategoryCfg MarkedBreakpoints;

	bool ShowPreviousFrameEvents;
};

class WsEventManager final : public BaseEventManager
{
private:
	static constexpr int ScanlineWidth = WsConstants::ClocksPerScanline*2;
	static constexpr int ScreenHeight = WsConstants::ScanlineCount;

	WsEventViewerConfig _config;

	WsPpu* _ppu;
	WsCpu* _cpu;
	WsConsole* _console;
	Debugger* _debugger;

	uint32_t _scanlineCount = 159;
	uint16_t* _ppuBuffer = nullptr;

protected:
	bool ShowPreviousFrameEvents() override;
	void ConvertScanlineCycleToRowColumn(int32_t& x, int32_t& y) override;
	void DrawScreen(uint32_t* buffer) override;

public:
	WsEventManager(Debugger* debugger, WsConsole* console, WsCpu* cpu, WsPpu* ppu);
	~WsEventManager();

	void AddEvent(DebugEventType type, MemoryOperationInfo& operation, int32_t breakpointId = -1) override;
	void AddEvent(DebugEventType type) override;

	EventViewerCategoryCfg GetEventConfig(DebugEventInfo& evt) override;

	uint32_t TakeEventSnapshot(bool forAutoRefresh) override;
	DebugEventInfo GetEvent(uint16_t y, uint16_t x) override;

	FrameInfo GetDisplayBufferSize() override;
	void SetConfiguration(BaseEventViewerConfig& config) override;
};
