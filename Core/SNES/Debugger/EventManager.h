#pragma once
#include "stdafx.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/BaseEventManager.h"
#include "Utilities/SimpleLock.h"

enum class DebugEventType;
struct DebugEventInfo;
struct EventViewerDisplayOptions;
class Cpu;
class Ppu;
class Debugger;
class DmaController;
class MemoryManager;

class EventManager final : public IEventManager
{
private:
	static constexpr int ScanlineWidth = 1364 / 2;

	Cpu * _cpu;
	Ppu *_ppu;
	MemoryManager* _memoryManager;
	DmaController *_dmaController;
	Debugger *_debugger;
	vector<DebugEventInfo> _debugEvents;
	vector<DebugEventInfo> _prevDebugEvents;
	vector<DebugEventInfo> _sentEvents;
	
	vector<DebugEventInfo> _snapshot;
	int16_t _snapshotScanline = -1;
	uint16_t _snapshotCycle = 0;
	SimpleLock _lock;

	bool _overscanMode = false;
	bool _useHighResOutput = false;
	uint32_t _scanlineCount = 262;
	uint16_t *_ppuBuffer = nullptr;

	void DrawEvent(DebugEventInfo &evt, bool drawBackground, uint32_t *buffer, EventViewerDisplayOptions &options);
	void FilterEvents(EventViewerDisplayOptions &options);

public:
	EventManager(Debugger *debugger, Cpu *cpu, Ppu *ppu, MemoryManager *memoryManager, DmaController *dmaController);
	~EventManager();

	void AddEvent(DebugEventType type, MemoryOperationInfo &operation, int32_t breakpointId = -1);
	void AddEvent(DebugEventType type);
	
	void GetEvents(DebugEventInfo *eventArray, uint32_t &maxEventCount);
	uint32_t GetEventCount(EventViewerDisplayOptions options);
	void ClearFrameEvents();

	uint32_t TakeEventSnapshot(EventViewerDisplayOptions options);
	void GetDisplayBuffer(uint32_t *buffer, uint32_t bufferSize, EventViewerDisplayOptions options);
	DebugEventInfo GetEvent(uint16_t scanline, uint16_t cycle, EventViewerDisplayOptions &options);
};
