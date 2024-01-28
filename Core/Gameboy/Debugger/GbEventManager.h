#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/BaseEventManager.h"
#include "Utilities/SimpleLock.h"

enum class DebugEventType;
struct DebugEventInfo;
class GbCpu;
class GbPpu;
class Debugger;

struct GbEventViewerConfig : public BaseEventViewerConfig
{
	EventViewerCategoryCfg Irq;
	EventViewerCategoryCfg MarkedBreakpoints;

	EventViewerCategoryCfg PpuRegisterCgramWrites;
	EventViewerCategoryCfg PpuRegisterVramWrites;
	EventViewerCategoryCfg PpuRegisterOamWrites;
	EventViewerCategoryCfg PpuRegisterBgScrollWrites;
	EventViewerCategoryCfg PpuRegisterWindowWrites;
	EventViewerCategoryCfg PpuRegisterOtherWrites;

	EventViewerCategoryCfg PpuRegisterCgramReads;
	EventViewerCategoryCfg PpuRegisterVramReads;
	EventViewerCategoryCfg PpuRegisterOamReads;
	EventViewerCategoryCfg PpuRegisterBgScrollReads;
	EventViewerCategoryCfg PpuRegisterWindowReads;
	EventViewerCategoryCfg PpuRegisterOtherReads;

	EventViewerCategoryCfg ApuRegisterReads;
	EventViewerCategoryCfg ApuRegisterWrites;

	EventViewerCategoryCfg SerialReads;
	EventViewerCategoryCfg SerialWrites;

	EventViewerCategoryCfg TimerReads;
	EventViewerCategoryCfg TimerWrites;

	EventViewerCategoryCfg InputReads;
	EventViewerCategoryCfg InputWrites;

	EventViewerCategoryCfg OtherRegisterReads;
	EventViewerCategoryCfg OtherRegisterWrites;

	bool ShowPreviousFrameEvents;
};

class GbEventManager final : public BaseEventManager
{
private:
	static constexpr int ScanlineWidth = 456*2;
	static constexpr int ScreenHeight = 154;
	static constexpr int VBlankScanline = 144;

	GbEventViewerConfig _config;

	GbPpu* _ppu;
	GbCpu* _cpu;
	Debugger* _debugger;

	uint32_t _scanlineCount = GbEventManager::ScreenHeight;
	uint16_t* _ppuBuffer = nullptr;

protected:
	bool ShowPreviousFrameEvents() override;
	void ConvertScanlineCycleToRowColumn(int32_t& x, int32_t& y) override;
	void DrawScreen(uint32_t* buffer) override;

public:
	GbEventManager(Debugger* debugger, GbCpu* cpu, GbPpu* ppu);
	~GbEventManager();

	void AddEvent(DebugEventType type, MemoryOperationInfo& operation, int32_t breakpointId = -1) override;
	void AddEvent(DebugEventType type) override;

	EventViewerCategoryCfg GetEventConfig(DebugEventInfo& evt) override;

	uint32_t TakeEventSnapshot(bool forAutoRefresh) override;
	DebugEventInfo GetEvent(uint16_t y, uint16_t x) override;

	FrameInfo GetDisplayBufferSize() override;
	void SetConfiguration(BaseEventViewerConfig& config) override;
};
