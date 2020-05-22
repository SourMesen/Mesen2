#pragma once
#include "stdafx.h"
#include "DebugTypes.h"
#include "BaseEventManager.h"
#include "../Utilities/SimpleLock.h"

enum class DebugEventType;
struct DebugEventInfo;
struct EventViewerDisplayOptions;
class GbCpu;
class GbPpu;
class Debugger;

class GbEventManager final : public IEventManager
{
private:
	static constexpr int ScanlineWidth = 456*2;
	static constexpr int ScreenHeight = 154;
	static constexpr int VBlankScanline = 144;

	GbPpu* _ppu;
	GbCpu* _cpu;
	Debugger* _debugger;

	vector<DebugEventInfo> _debugEvents;
	vector<DebugEventInfo> _prevDebugEvents;
	vector<DebugEventInfo> _sentEvents;

	vector<DebugEventInfo> _snapshot;
	uint16_t _snapshotScanline = 0;
	uint16_t _snapshotCycle = 0;
	SimpleLock _lock;

	uint32_t _scanlineCount = 262;
	uint16_t* _ppuBuffer = nullptr;

	void DrawEvent(DebugEventInfo& evt, bool drawBackground, uint32_t* buffer, EventViewerDisplayOptions& options);
	void FilterEvents(EventViewerDisplayOptions& options);

public:
	GbEventManager(Debugger* debugger, GbCpu* cpu, GbPpu* ppu);
	~GbEventManager();

	void AddEvent(DebugEventType type, MemoryOperationInfo& operation, int32_t breakpointId = -1);
	void AddEvent(DebugEventType type);

	void GetEvents(DebugEventInfo* eventArray, uint32_t& maxEventCount);
	uint32_t GetEventCount(EventViewerDisplayOptions options);
	void ClearFrameEvents();

	uint32_t TakeEventSnapshot(EventViewerDisplayOptions options);
	void GetDisplayBuffer(uint32_t* buffer, uint32_t bufferSize, EventViewerDisplayOptions options);
	DebugEventInfo GetEvent(uint16_t scanline, uint16_t cycle, EventViewerDisplayOptions& options);
};
