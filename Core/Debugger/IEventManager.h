#pragma once
#include "stdafx.h"
#include "Debugger/DebugTypes.h"
#include "Shared/SettingTypes.h"
#include "SNES/DmaControllerTypes.h"

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
};

struct EventViewerCategoryCfg
{
	bool Visible;
	uint32_t Color;
};

struct BaseEventViewerConfig
{
};

class IEventManager
{
public:
	virtual void SetConfiguration(BaseEventViewerConfig& config) = 0;

	virtual void AddEvent(DebugEventType type, MemoryOperationInfo& operation, int32_t breakpointId = -1) = 0;
	virtual void AddEvent(DebugEventType type) = 0;

	virtual void GetEvents(DebugEventInfo* eventArray, uint32_t& maxEventCount) = 0;
	virtual uint32_t GetEventCount() = 0;
	virtual void ClearFrameEvents() = 0;

	virtual uint32_t TakeEventSnapshot() = 0;
	virtual FrameInfo GetDisplayBufferSize() = 0;
	virtual void GetDisplayBuffer(uint32_t* buffer, uint32_t bufferSize) = 0;
	virtual DebugEventInfo GetEvent(uint16_t scanline, uint16_t cycle) = 0;
};
