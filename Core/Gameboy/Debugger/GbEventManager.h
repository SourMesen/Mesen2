#pragma once
#include "stdafx.h"
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

	EventViewerCategoryCfg PpuRegisterReads;
	EventViewerCategoryCfg PpuRegisterCgramWrites;
	EventViewerCategoryCfg PpuRegisterVramWrites;
	EventViewerCategoryCfg PpuRegisterOamWrites;
	EventViewerCategoryCfg PpuRegisterBgScrollWrites;
	EventViewerCategoryCfg PpuRegisterWindowWrites;
	EventViewerCategoryCfg PpuRegisterOtherWrites;

	EventViewerCategoryCfg ApuRegisterReads;
	EventViewerCategoryCfg ApuRegisterWrites;
	EventViewerCategoryCfg CpuRegisterReads;
	EventViewerCategoryCfg CpuRegisterWrites;

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

	void DrawEvent(DebugEventInfo& evt, bool drawBackground, uint32_t* buffer);

protected:
	bool ShowPreviousFrameEvents() override;

public:
	GbEventManager(Debugger* debugger, GbCpu* cpu, GbPpu* ppu);
	~GbEventManager();

	void AddEvent(DebugEventType type, MemoryOperationInfo& operation, int32_t breakpointId = -1);
	void AddEvent(DebugEventType type);

	EventViewerCategoryCfg GetEventConfig(DebugEventInfo& evt);

	uint32_t TakeEventSnapshot();
	void GetDisplayBuffer(uint32_t* buffer, uint32_t bufferSize);
	DebugEventInfo GetEvent(uint16_t y, uint16_t x);

	FrameInfo GetDisplayBufferSize() override;
	void SetConfiguration(BaseEventViewerConfig& config) override;
};
