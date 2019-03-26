#include "stdafx.h"
#include "EventManager.h"
#include "DebugTypes.h"
#include "Cpu.h"
#include "Ppu.h"
#include "Debugger.h"
#include "DebugBreakHelper.h"
#include "DefaultVideoFilter.h"

EventManager::EventManager(Debugger *debugger, Cpu *cpu, Ppu *ppu)
{
	_debugger = debugger;
	_cpu = cpu;
	_ppu = ppu;
}

void EventManager::AddEvent(DebugEventType type, MemoryOperationInfo &operation, int32_t breakpointId)
{
	DebugEventInfo evt;
	evt.Type = type;
	evt.Operation = operation;
	evt.Scanline = _ppu->GetScanline();
	evt.Cycle = _ppu->GetCycle();
	evt.BreakpointId = breakpointId;

	CpuState state = _cpu->GetState();
	evt.ProgramCounter = (state.K << 16) | state.PC;

	_debugEvents.push_back(evt);
}

void EventManager::AddEvent(DebugEventType type)
{
	DebugEventInfo evt = {};
	evt.Type = type;
	evt.Scanline = _ppu->GetScanline();
	evt.Cycle = _ppu->GetCycle();
	
	CpuState state = _cpu->GetState();
	evt.ProgramCounter = (state.K << 16) | state.PC;

	_debugEvents.push_back(evt);
}

void EventManager::GetEvents(DebugEventInfo *eventArray, uint32_t &maxEventCount, bool getPreviousFrameData)
{
	DebugBreakHelper breakHelper(_debugger);

	vector<DebugEventInfo> &events = getPreviousFrameData ? _prevDebugEvents : _debugEvents;
	uint32_t eventCount = std::min(maxEventCount, (uint32_t)events.size());
	memcpy(eventArray, events.data(), eventCount * sizeof(DebugEventInfo));
	maxEventCount = eventCount;
}

DebugEventInfo EventManager::GetEvent(uint16_t scanline, uint16_t cycle, EventViewerDisplayOptions &options)
{
	auto lock = _lock.AcquireSafe();

	for(DebugEventInfo &evt : _sentEvents) {
		if(evt.Cycle == cycle && evt.Scanline == scanline) {
			return evt;
		}
	}

	DebugEventInfo empty = {};
	empty.ProgramCounter = 0xFFFFFFFF;
	return empty;
}

uint32_t EventManager::GetEventCount(bool getPreviousFrameData)
{
	DebugBreakHelper breakHelper(_debugger);
	return (uint32_t)(getPreviousFrameData ? _prevDebugEvents.size() : _debugEvents.size());
}

void EventManager::ClearFrameEvents()
{
	_prevDebugEvents = _debugEvents;
	_debugEvents.clear();
}

void EventManager::DrawEvent(DebugEventInfo &evt, bool drawBackground, uint32_t *buffer, EventViewerDisplayOptions &options)
{
	bool isWrite = evt.Operation.Type == MemoryOperationType::Write || evt.Operation.Type == MemoryOperationType::DmaWrite;
	bool showEvent = false;
	uint32_t color = 0;
	switch(evt.Type) {
		case DebugEventType::Breakpoint: showEvent = options.ShowMarkedBreakpoints; color = options.BreakpointColor; break;
		case DebugEventType::Irq: showEvent = options.ShowIrq; color = options.IrqColor; break;
		case DebugEventType::Nmi: showEvent = options.ShowNmi; color = options.NmiColor; break;
		case DebugEventType::Register:
			uint16_t reg = evt.Operation.Address & 0xFFFF;
			if(reg <= 0x213F) {
				showEvent = isWrite ? options.ShowPpuRegisterWrites : options.ShowPpuRegisterReads;
				color = isWrite ? options.PpuRegisterWriteColor : options.PpuRegisterReadColor;
			} else if(reg <= 0x217F) {
				showEvent = isWrite ? options.ShowApuRegisterWrites : options.ShowApuRegisterReads;
				color = isWrite ? options.ApuRegisterWriteColor : options.ApuRegisterReadColor;
			} else if(reg <= 0x2183) {
				showEvent = isWrite ? options.ShowWorkRamRegisterWrites : options.ShowWorkRamRegisterReads;
				color = isWrite ? options.WorkRamRegisterWriteColor : options.WorkRamRegisterReadColor;
			} else if(reg >= 0x4000) {
				showEvent = isWrite ? options.ShowCpuRegisterWrites : options.ShowCpuRegisterReads;
				color = isWrite ? options.CpuRegisterWriteColor : options.CpuRegisterReadColor;
			}
			break;
	}

	if(!showEvent) {
		return;
	}

	if(drawBackground){
		color = 0xFF000000 | ((color >> 1) & 0x7F7F7F);
	} else {
		_sentEvents.push_back(evt);
		color |= 0xFF000000;
	}

	int iMin = drawBackground ? -2 : 0;
	int iMax = drawBackground ? 3 : 1;
	int jMin = drawBackground ? -2 : 0;
	int jMax = drawBackground ? 3 : 1;
	uint32_t y = evt.Scanline * 2;
	uint32_t x = evt.Cycle * 2;

	for(int i = iMin; i <= iMax; i++) {
		for(int j = jMin; j <= jMax; j++) {
			int32_t pos = (y + i) * 340 * 2 + x + j;
			if(pos < 0 || pos > 340 * 2 * 262 * 2) {
				continue;
			}
			buffer[pos] = color;
		}
	}
}

void EventManager::TakeEventSnapshot(EventViewerDisplayOptions options)
{
	DebugBreakHelper breakHelper(_debugger);
	auto lock = _lock.AcquireSafe();
	_snapshot.clear();

	uint16_t cycle = _ppu->GetState().Cycle;
	uint16_t scanline = _ppu->GetState().Scanline;
	uint32_t key = (scanline << 9) + cycle;

	_snapshot = _debugEvents;
	_snapshotScanline = scanline;
	if(options.ShowPreviousFrameEvents && scanline != 0) {
		for(DebugEventInfo &evt : _prevDebugEvents) {
			uint32_t evtKey = (evt.Scanline << 9) + evt.Cycle;
			if(evtKey > key) {
				_snapshot.push_back(evt);
			}
		}
	}
}

void EventManager::GetDisplayBuffer(uint32_t *buffer, EventViewerDisplayOptions options)
{
	auto lock = _lock.AcquireSafe();
	_sentEvents.clear();

	for(int i = 0; i < 340 * 2 * 262 * 2; i++) {
		buffer[i] = 0xFF555555;
	}
	bool overscanMode = _ppu->GetState().OverscanMode;
	uint16_t *ppuBuffer = _ppu->GetScreenBuffer();
	uint32_t pixelCount = 256*2*239*2;
	for(uint32_t y = 0, len = overscanMode ? 239*2 : 224*2; y < len; y++) {
		for(uint32_t x = 0; x < 512; x++) {
			buffer[(y + 2)*340*2 + x + 22*2] = DefaultVideoFilter::ToArgb(ppuBuffer[(y << 9) | x]);
		}
	}

	constexpr uint32_t nmiColor = 0xFF55FFFF;
	constexpr uint32_t currentScanlineColor = 0xFFFFFF55;
	int nmiScanline = (overscanMode ? 240 : 225) * 2 * 340 * 2;
	uint32_t scanlineOffset = _snapshotScanline * 2 * 340 * 2;
	for(int i = 0; i < 340 * 2; i++) {
		buffer[nmiScanline + i] = nmiColor;
		buffer[nmiScanline + 340 * 2 + i] = nmiColor;
		if(_snapshotScanline != 0) {
			buffer[scanlineOffset + i] = currentScanlineColor;
			buffer[scanlineOffset + 340 * 2 + i] = currentScanlineColor;
		}
	}

	for(DebugEventInfo &evt : _snapshot) {
		DrawEvent(evt, true, buffer, options);
	}
	for(DebugEventInfo &evt : _snapshot) {
		DrawEvent(evt, false, buffer, options);
	}
}
