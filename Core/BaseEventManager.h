#pragma once
#include "stdafx.h"
#include "DebugTypes.h"

struct DebugEventInfo
{
	MemoryOperationInfo Operation;
	DebugEventType Type;
	uint32_t ProgramCounter;
	uint16_t Scanline;
	uint16_t Cycle;
	int16_t BreakpointId;
	int8_t DmaChannel;
	DmaChannelConfig DmaChannelInfo;
};

struct EventViewerDisplayOptions
{
	uint32_t IrqColor;
	uint32_t NmiColor;
	uint32_t BreakpointColor;

	uint32_t PpuRegisterReadColor;
	uint32_t PpuRegisterWriteCgramColor;
	uint32_t PpuRegisterWriteVramColor;
	uint32_t PpuRegisterWriteOamColor;
	uint32_t PpuRegisterWriteMode7Color;
	uint32_t PpuRegisterWriteBgOptionColor;
	uint32_t PpuRegisterWriteBgScrollColor;
	uint32_t PpuRegisterWriteWindowColor;
	uint32_t PpuRegisterWriteOtherColor;

	uint32_t ApuRegisterReadColor;
	uint32_t ApuRegisterWriteColor;
	uint32_t CpuRegisterReadColor;
	uint32_t CpuRegisterWriteColor;
	uint32_t WorkRamRegisterReadColor;
	uint32_t WorkRamRegisterWriteColor;

	bool ShowPpuRegisterCgramWrites;
	bool ShowPpuRegisterVramWrites;
	bool ShowPpuRegisterOamWrites;
	bool ShowPpuRegisterMode7Writes;
	bool ShowPpuRegisterBgOptionWrites;
	bool ShowPpuRegisterBgScrollWrites;
	bool ShowPpuRegisterWindowWrites;
	bool ShowPpuRegisterOtherWrites;

	bool ShowPpuRegisterReads;
	bool ShowCpuRegisterWrites;
	bool ShowCpuRegisterReads;

	bool ShowApuRegisterWrites;
	bool ShowApuRegisterReads;
	bool ShowWorkRamRegisterWrites;
	bool ShowWorkRamRegisterReads;

	bool ShowNmi;
	bool ShowIrq;

	bool ShowMarkedBreakpoints;
	bool ShowPreviousFrameEvents;

	bool ShowDmaChannels[8];
};

class IEventManager
{
public:
	virtual void AddEvent(DebugEventType type, MemoryOperationInfo& operation, int32_t breakpointId = -1) = 0;
	virtual void AddEvent(DebugEventType type) = 0;

	virtual void GetEvents(DebugEventInfo* eventArray, uint32_t& maxEventCount) = 0;
	virtual uint32_t GetEventCount(EventViewerDisplayOptions options) = 0;
	virtual void ClearFrameEvents() = 0;

	virtual uint32_t TakeEventSnapshot(EventViewerDisplayOptions options) = 0;
	virtual void GetDisplayBuffer(uint32_t* buffer, uint32_t bufferSize, EventViewerDisplayOptions options) = 0;
	virtual DebugEventInfo GetEvent(uint16_t scanline, uint16_t cycle, EventViewerDisplayOptions& options) = 0;
};
