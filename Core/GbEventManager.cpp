#include "stdafx.h"
#include "GbEventManager.h"
#include "DebugTypes.h"
#include "GbCpu.h"
#include "GbPpu.h"
#include "Debugger.h"
#include "DebugBreakHelper.h"
#include "DefaultVideoFilter.h"
#include "Gameboy.h"
#include "Console.h"
#include "BaseCartridge.h"
#include "BaseEventManager.h"

GbEventManager::GbEventManager(Debugger* debugger, GbCpu* cpu, GbPpu* ppu)
{
	_debugger = debugger;
	_cpu = cpu;
	_ppu = ppu;

	_ppuBuffer = new uint16_t[456*GbEventManager::ScreenHeight];
	memset(_ppuBuffer, 0, 456*GbEventManager::ScreenHeight * sizeof(uint16_t));
}

GbEventManager::~GbEventManager()
{
	delete[] _ppuBuffer;
}

void GbEventManager::AddEvent(DebugEventType type, MemoryOperationInfo& operation, int32_t breakpointId)
{
	DebugEventInfo evt = {};
	evt.Type = type;
	evt.Operation = operation;
	evt.Scanline = _ppu->GetState().Scanline;
	evt.Cycle = _ppu->GetState().Cycle;
	evt.BreakpointId = breakpointId;
	evt.DmaChannel = -1;
	evt.ProgramCounter = _cpu->GetState().PC;
	_debugEvents.push_back(evt);
}

void GbEventManager::AddEvent(DebugEventType type)
{
	DebugEventInfo evt = {};
	evt.Type = type;
	evt.Scanline = _ppu->GetState().Scanline;
	evt.Cycle = _ppu->GetState().Cycle;
	evt.BreakpointId = -1;
	evt.DmaChannel = -1;
	evt.ProgramCounter = _cpu->GetState().PC;
	_debugEvents.push_back(evt);
}

void GbEventManager::GetEvents(DebugEventInfo* eventArray, uint32_t& maxEventCount)
{
	auto lock = _lock.AcquireSafe();
	uint32_t eventCount = std::min(maxEventCount, (uint32_t)_sentEvents.size());
	memcpy(eventArray, _sentEvents.data(), eventCount * sizeof(DebugEventInfo));
	maxEventCount = eventCount;
}

DebugEventInfo GbEventManager::GetEvent(uint16_t scanline, uint16_t cycle, EventViewerDisplayOptions& options)
{
	auto lock = _lock.AcquireSafe();

	for(DebugEventInfo& evt : _sentEvents) {
		if(evt.Cycle == cycle && evt.Scanline == scanline) {
			return evt;
		}
	}

	DebugEventInfo empty = {};
	empty.ProgramCounter = 0xFFFFFFFF;
	return empty;
}

uint32_t GbEventManager::GetEventCount(EventViewerDisplayOptions options)
{
	auto lock = _lock.AcquireSafe();
	FilterEvents(options);
	return (uint32_t)_sentEvents.size();
}

void GbEventManager::ClearFrameEvents()
{
	_prevDebugEvents = _debugEvents;
	_debugEvents.clear();
}

void GbEventManager::FilterEvents(EventViewerDisplayOptions& options)
{
	auto lock = _lock.AcquireSafe();
	_sentEvents.clear();

	vector<DebugEventInfo> events = _snapshot;
	if(options.ShowPreviousFrameEvents && _snapshotScanline != 0) {
		uint32_t key = (_snapshotScanline << 16) + _snapshotCycle;
		for(DebugEventInfo& evt : _prevDebugEvents) {
			uint32_t evtKey = (evt.Scanline << 16) + evt.Cycle;
			if(evtKey > key) {
				events.push_back(evt);
			}
		}
	}

	for(DebugEventInfo& evt : events) {
		bool isWrite = evt.Operation.Type == MemoryOperationType::Write || evt.Operation.Type == MemoryOperationType::DmaWrite;
		bool showEvent = false;
		switch(evt.Type) {
			default: break;
			case DebugEventType::Breakpoint: showEvent = options.ShowMarkedBreakpoints; break;
			case DebugEventType::Irq: showEvent = options.ShowIrq; break;
			case DebugEventType::Register:
				uint16_t reg = evt.Operation.Address & 0xFFFF;
				if(reg >= 0xFE00 && reg <= 0xFE9F) {
					showEvent = isWrite ? options.ShowPpuRegisterOamWrites : options.ShowPpuRegisterReads;
				} else if(reg >= 0xFF42 && reg <= 0xFF43) {
					showEvent = isWrite ? options.ShowPpuRegisterBgScrollWrites : options.ShowPpuRegisterReads;
				} else if(reg >= 0x8000 && reg <= 0x9FFF) {
					showEvent = isWrite ? options.ShowPpuRegisterVramWrites : options.ShowPpuRegisterReads;
				} else if(reg >= 0xFF47 && reg <= 0xFF49 || (reg >= 0xFF68 && reg <= 0xFF6B)) {
					showEvent = isWrite ? options.ShowPpuRegisterCgramWrites : options.ShowPpuRegisterReads;
				} else if(reg >= 0xFF4A && reg <= 0xFF4B) {
					showEvent = isWrite ? options.ShowPpuRegisterWindowWrites : options.ShowPpuRegisterReads;
				} else if(reg >= 0xFF40 && reg <= 0xFF70) {
					showEvent = isWrite ? options.ShowPpuRegisterOtherWrites : options.ShowPpuRegisterReads;
				} else if(reg >= 0xFF10 && reg <= 0xFF3F) {
					showEvent = isWrite ? options.ShowApuRegisterWrites : options.ShowApuRegisterReads;
				} else {
					showEvent = isWrite ? options.ShowCpuRegisterWrites : options.ShowCpuRegisterReads;
				}
				break;
		}

		if(showEvent) {
			_sentEvents.push_back(evt);
		}
	}
}

void GbEventManager::DrawEvent(DebugEventInfo& evt, bool drawBackground, uint32_t* buffer, EventViewerDisplayOptions& options)
{
	bool isWrite = evt.Operation.Type == MemoryOperationType::Write || evt.Operation.Type == MemoryOperationType::DmaWrite;
	uint32_t color = 0;
	uint32_t ppuReadColor = options.PpuRegisterReadColor;
	switch(evt.Type) {
		default: break;
		case DebugEventType::Breakpoint: color = options.BreakpointColor; break;
		case DebugEventType::Irq: color = options.IrqColor; break;
		case DebugEventType::Register:
			uint16_t reg = evt.Operation.Address & 0xFFFF;
			if(reg >= 0xFE00 && reg <= 0xFE9F) {
				color = isWrite ? options.PpuRegisterWriteOamColor : ppuReadColor;
			} else if(reg >= 0xFF42 && reg <= 0xFF43) {
				color = isWrite ? options.PpuRegisterWriteBgScrollColor : ppuReadColor;
			} else if(reg >= 0x8000 && reg <= 0x9FFF) {
				color = isWrite ? options.PpuRegisterWriteVramColor : ppuReadColor;
			} else if(reg >= 0xFF47 && reg <= 0xFF49 || (reg >= 0xFF68 && reg <= 0xFF6B)) {
				color = isWrite ? options.PpuRegisterWriteCgramColor : ppuReadColor;
			} else if(reg >= 0xFF4A && reg <= 0xFF4B) {
				color = isWrite ? options.PpuRegisterWriteWindowColor : ppuReadColor;
			} else if(reg >= 0xFF40 && reg <= 0xFF70) {
				color = isWrite ? options.PpuRegisterWriteOtherColor : ppuReadColor;
			} else if(reg >= 0xFF10 && reg <= 0xFF3F) {
				color = isWrite ? options.ApuRegisterWriteColor : options.ApuRegisterReadColor;
			} else {
				color = isWrite ? options.CpuRegisterWriteColor : options.CpuRegisterReadColor;
			}
			break;
	}

	if(drawBackground) {
		color = 0xFF000000 | ((color >> 1) & 0x7F7F7F);
	} else {
		color |= 0xFF000000;
	}

	int iMin = drawBackground ? -2 : 0;
	int iMax = drawBackground ? 3 : 1;
	int jMin = drawBackground ? -2 : 0;
	int jMax = drawBackground ? 3 : 1;
	uint32_t y = std::min<uint32_t>(evt.Scanline * 2, _scanlineCount * 2);
	uint32_t x = evt.Cycle * 2;

	for(int i = iMin; i <= iMax; i++) {
		for(int j = jMin; j <= jMax; j++) {
			int32_t pos = (y + i) * GbEventManager::ScanlineWidth + x + j;
			if(pos < 0 || pos >= GbEventManager::ScanlineWidth * (int)_scanlineCount * 2) {
				continue;
			}
			buffer[pos] = color;
		}
	}
}

uint32_t GbEventManager::TakeEventSnapshot(EventViewerDisplayOptions options)
{
	DebugBreakHelper breakHelper(_debugger);
	auto lock = _lock.AcquireSafe();
	_snapshot.clear();

	uint16_t cycle = _ppu->GetState().Cycle;
	uint16_t scanline = _ppu->GetState().Scanline;

	if(scanline >= GbEventManager::VBlankScanline || scanline == 0) {
		memcpy(_ppuBuffer, _ppu->GetEventViewerBuffer(), 456 * GbEventManager::ScreenHeight * sizeof(uint16_t));
	} else {
		uint32_t size = 456 * GbEventManager::ScreenHeight;
		uint32_t offset = 456 * scanline;
		memcpy(_ppuBuffer, _ppu->GetEventViewerBuffer(), offset * sizeof(uint16_t));
		memcpy(_ppuBuffer + offset, _ppu->GetPreviousEventViewerBuffer() + offset, (size - offset) * sizeof(uint16_t));
	}

	_snapshot = _debugEvents;
	_snapshotScanline = scanline;
	_snapshotCycle = cycle;
	_scanlineCount = GbEventManager::ScreenHeight;
	return _scanlineCount;
}

void GbEventManager::GetDisplayBuffer(uint32_t* buffer, uint32_t bufferSize, EventViewerDisplayOptions options)
{
	auto lock = _lock.AcquireSafe();

	if(bufferSize < _scanlineCount * 2 * GbEventManager::ScanlineWidth * 4) {
		return;
	}

	uint16_t *src = _ppuBuffer;
	for(uint32_t y = 0, len = GbEventManager::ScreenHeight*2; y < len; y++) {
		for(uint32_t x = 0; x < GbEventManager::ScanlineWidth; x++) {
			int srcOffset = (y >> 1) * 456 + (x >> 1);
			buffer[y*GbEventManager::ScanlineWidth + x] = DefaultVideoFilter::ToArgb(src[srcOffset]);
		}
	}

	constexpr uint32_t currentScanlineColor = 0xFFFFFF55;
	uint32_t scanlineOffset = _snapshotScanline * 2 * GbEventManager::ScanlineWidth;
	if(_snapshotScanline != 0) {
		for(int i = 0; i < GbEventManager::ScanlineWidth; i++) {
			buffer[scanlineOffset + i] = currentScanlineColor;
			buffer[scanlineOffset + GbEventManager::ScanlineWidth + i] = currentScanlineColor;
		}
	}

	FilterEvents(options);
	for(DebugEventInfo &evt : _sentEvents) {
		DrawEvent(evt, true, buffer, options);
	}
	for(DebugEventInfo &evt : _sentEvents) {
		DrawEvent(evt, false, buffer, options);
	}
}
