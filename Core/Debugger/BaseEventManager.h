#pragma once
#include "stdafx.h"
#include "Debugger/DebugTypes.h"
#include "Shared/SettingTypes.h"
#include "Utilities/SimpleLock.h"
#include "SNES/DmaControllerTypes.h"

enum class EventFlags
{
	PreviousFrame = 1,
	NesPpuSecondWrite = 2,
};

struct DebugEventInfo
{
	MemoryOperationInfo Operation;
	DebugEventType Type;
	uint32_t ProgramCounter;
	int16_t Scanline;
	uint16_t Cycle;
	int16_t BreakpointId;
	int8_t DmaChannel;
	DmaChannelConfig DmaChannelInfo;
	uint32_t Flags;
};

struct EventViewerCategoryCfg
{
	bool Visible;
	uint32_t Color;
};

struct BaseEventViewerConfig
{
};

class BaseEventManager
{
protected:
	vector<DebugEventInfo> _debugEvents;
	vector<DebugEventInfo> _prevDebugEvents;
	vector<DebugEventInfo> _sentEvents;

	vector<DebugEventInfo> _snapshotCurrentFrame;
	vector<DebugEventInfo> _snapshotPrevFrame;
	int16_t _snapshotScanline = -1;
	uint16_t _snapshotCycle = 0;
	SimpleLock _lock;

	virtual bool ShowPreviousFrameEvents() = 0;

	void FilterEvents();
	void DrawDot(uint32_t x, uint32_t y, uint32_t color, bool drawBackground, uint32_t* buffer);
	virtual int GetScanlineOffset() { return 0; }

public:
	virtual ~BaseEventManager() {}

	virtual void SetConfiguration(BaseEventViewerConfig& config) = 0;

	virtual void AddEvent(DebugEventType type, MemoryOperationInfo& operation, int32_t breakpointId = -1) = 0;
	virtual void AddEvent(DebugEventType type) = 0;

	void GetEvents(DebugEventInfo* eventArray, uint32_t& maxEventCount);
	uint32_t GetEventCount();
	virtual void ClearFrameEvents();

	virtual EventViewerCategoryCfg GetEventConfig(DebugEventInfo& evt) = 0;

	virtual uint32_t TakeEventSnapshot() = 0;
	virtual FrameInfo GetDisplayBufferSize() = 0;
	virtual void GetDisplayBuffer(uint32_t* buffer, uint32_t bufferSize) = 0;
	virtual DebugEventInfo GetEvent(uint16_t scanline, uint16_t cycle) = 0;
};
