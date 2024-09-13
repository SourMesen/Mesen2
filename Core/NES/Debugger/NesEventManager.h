#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/BaseEventManager.h"
#include "Utilities/SimpleLock.h"

enum class DebugEventType;

class NesCpu;
class BaseNesPpu;
class BaseMapper;
class NesConsole;
class Debugger;

struct NesEventViewerConfig : public BaseEventViewerConfig
{
	EventViewerCategoryCfg Irq;
	EventViewerCategoryCfg Nmi;
	EventViewerCategoryCfg MarkedBreakpoints;

	EventViewerCategoryCfg MapperRegisterWrites;
	EventViewerCategoryCfg MapperRegisterReads;
	EventViewerCategoryCfg ApuRegisterWrites;
	EventViewerCategoryCfg ApuRegisterReads;
	EventViewerCategoryCfg ControlRegisterWrites;
	EventViewerCategoryCfg ControlRegisterReads;

	EventViewerCategoryCfg Ppu2000Write;
	EventViewerCategoryCfg Ppu2001Write;
	EventViewerCategoryCfg Ppu2003Write;
	EventViewerCategoryCfg Ppu2004Write;
	EventViewerCategoryCfg Ppu2005Write;
	EventViewerCategoryCfg Ppu2006Write;
	EventViewerCategoryCfg Ppu2007Write;
	
	EventViewerCategoryCfg Ppu2002Read;
	EventViewerCategoryCfg Ppu2004Read;
	EventViewerCategoryCfg Ppu2007Read;

	EventViewerCategoryCfg DmcDmaReads;
	EventViewerCategoryCfg OtherDmaReads;
	EventViewerCategoryCfg SpriteZeroHit;

	bool ShowPreviousFrameEvents;
	bool ShowNtscBorders;
};

class NesEventManager : public BaseEventManager
{
private:
	NesEventViewerConfig _config = {};

	NesCpu* _cpu = nullptr;
	NesConsole* _console = nullptr;
	BaseMapper* _mapper = nullptr;
	Debugger* _debugger = nullptr;

	uint32_t _palette[512] = {};

	uint32_t _scanlineCount = 262;
	uint16_t *_ppuBuffer = nullptr;

	void DrawNtscBorders(uint32_t *buffer);
	void DrawPixel(uint32_t *buffer, int32_t x, uint32_t y, uint32_t color);

	void ProcessNtscBorderColorEvents(vector<DebugEventInfo>& events, vector<uint16_t>& bgColor, uint32_t& currentPos, uint16_t& currentColor);

protected:
	void ConvertScanlineCycleToRowColumn(int32_t& x, int32_t& y) override;
	void DrawScreen(uint32_t* buffer) override;

	bool ShowPreviousFrameEvents() override;
	int GetScanlineOffset() override { return 1; }

public:
	NesEventManager(Debugger *debugger, NesConsole* console);
	~NesEventManager();

	void AddEvent(DebugEventType type, MemoryOperationInfo& operation, int32_t breakpointId = -1) override;
	void AddEvent(DebugEventType type) override;

	void ClearFrameEvents() override;

	EventViewerCategoryCfg GetEventConfig(DebugEventInfo& evt) override;

	uint32_t TakeEventSnapshot(bool forAutoRefresh) override;

	FrameInfo GetDisplayBufferSize() override;

	DebugEventInfo GetEvent(uint16_t y, uint16_t x) override;

	void SetConfiguration(BaseEventViewerConfig& config) override;
};
