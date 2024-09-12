#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"
#include "Shared/SettingTypes.h"
#include "Utilities/SimpleLock.h"
#include "SNES/DmaControllerTypes.h"

enum class EventFlags
{
	PreviousFrame = 1 << 0,
	RegFirstWrite = 1 << 1,
	RegSecondWrite = 1 << 2,
	WithTargetMemory = 1 << 3,
	SmsVdpPaletteWrite = 1 << 4,
	ReadWriteOp = 1 << 5,
};

struct DebugEventInfo
{
	MemoryOperationInfo Operation;
	DebugEventType Type;
	uint32_t ProgramCounter;
	int16_t Scanline;
	uint16_t Cycle;
	int16_t BreakpointId = -1;
	int8_t DmaChannel = -1;
	DmaChannelConfig DmaChannelInfo;
	uint32_t Flags;
	int32_t RegisterId = -1;
	MemoryOperationInfo TargetMemory;
	uint32_t Color = 0;
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
	int16_t _snapshotScanlineOffset = 0;
	uint16_t _snapshotCycle = 0;
	bool _forAutoRefresh = false;
	SimpleLock _lock;

	virtual bool ShowPreviousFrameEvents() = 0;

	void FilterEvents();
	void DrawDot(uint32_t x, uint32_t y, uint32_t color, bool drawBackground, uint32_t* buffer);
	virtual int GetScanlineOffset() { return 0; }

	void DrawLine(uint32_t* buffer, FrameInfo size, uint32_t color, uint32_t row);
	void DrawEvents(uint32_t* buffer, FrameInfo size);

	virtual void ConvertScanlineCycleToRowColumn(int32_t& x, int32_t& y) = 0;
	virtual void DrawScreen(uint32_t* buffer) = 0;
	void DrawEvent(DebugEventInfo& evt, bool drawBackground, uint32_t* buffer);

public:
	virtual ~BaseEventManager() {}

	virtual void SetConfiguration(BaseEventViewerConfig& config) = 0;

	virtual void AddEvent(DebugEventType type, MemoryOperationInfo& operation, int32_t breakpointId = -1) = 0;
	virtual void AddEvent(DebugEventType type) = 0;

	void GetEvents(DebugEventInfo* eventArray, uint32_t& maxEventCount);
	uint32_t GetEventCount();
	virtual void ClearFrameEvents();

	virtual EventViewerCategoryCfg GetEventConfig(DebugEventInfo& evt) = 0;

	virtual uint32_t TakeEventSnapshot(bool forAutoRefresh) = 0;
	virtual FrameInfo GetDisplayBufferSize() = 0;
	virtual DebugEventInfo GetEvent(uint16_t scanline, uint16_t cycle) = 0;
	
	void GetDisplayBuffer(uint32_t* buffer, uint32_t bufferSize);
};
