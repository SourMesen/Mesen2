#pragma once
#include "stdafx.h"
#include "DmaController.h"
#include "DebugTypes.h"
#include "../Utilities/SimpleLock.h"

enum class DebugEventType;
struct DebugEventInfo;
struct EventViewerDisplayOptions;
class Cpu;
class Ppu;
class Debugger;
class DmaController;

class EventManager
{
private:
	Cpu * _cpu;
	Ppu *_ppu;
	DmaController *_dmaController;
	Debugger *_debugger;
	vector<DebugEventInfo> _debugEvents;
	vector<DebugEventInfo> _prevDebugEvents;
	vector<DebugEventInfo> _sentEvents;
	
	vector<DebugEventInfo> _snapshot;
	uint16_t _snapshotScanline;
	SimpleLock _lock;

	bool _overscanMode = false;
	bool _useHighResOutput = false;
	uint32_t _scanlineCount = 262;
	uint16_t *_ppuBuffer;

	void DrawEvent(DebugEventInfo &evt, bool drawBackground, uint32_t *buffer, EventViewerDisplayOptions &options);

public:
	EventManager(Debugger *debugger, Cpu *cpu, Ppu *ppu, DmaController *dmaController);
	~EventManager();

	void AddEvent(DebugEventType type, MemoryOperationInfo &operation, int32_t breakpointId = -1);
	void AddEvent(DebugEventType type);
	
	void GetEvents(DebugEventInfo *eventArray, uint32_t &maxEventCount, bool getPreviousFrameData);
	uint32_t GetEventCount(bool getPreviousFrameData);
	void ClearFrameEvents();

	uint32_t TakeEventSnapshot(EventViewerDisplayOptions options);
	void GetDisplayBuffer(uint32_t *buffer, EventViewerDisplayOptions options);
	DebugEventInfo GetEvent(uint16_t scanline, uint16_t cycle, EventViewerDisplayOptions &options);
};

enum class DebugEventType
{
	Register,
	Nmi,
	Irq,
	Breakpoint
};

struct DebugEventInfo
{
	MemoryOperationInfo Operation;
	DebugEventType Type;
	uint32_t ProgramCounter;
	uint16_t Scanline;
	uint16_t Cycle;
	int16_t BreakpointId;
	uint8_t DmaChannel;
	DmaChannelConfig DmaChannelInfo;
};

struct EventViewerDisplayOptions
{
	uint32_t IrqColor;
	uint32_t NmiColor;
	uint32_t BreakpointColor;
	uint32_t PpuRegisterReadColor;
	uint32_t PpuRegisterWriteColor;
	uint32_t ApuRegisterReadColor;
	uint32_t ApuRegisterWriteColor;
	uint32_t CpuRegisterReadColor;
	uint32_t CpuRegisterWriteColor;
	uint32_t WorkRamRegisterReadColor;
	uint32_t WorkRamRegisterWriteColor;

	bool ShowPpuRegisterWrites;
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