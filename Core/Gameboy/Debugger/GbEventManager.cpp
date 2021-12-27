#include "stdafx.h"
#include "Gameboy/Debugger/GbEventManager.h"
#include "Gameboy/GbCpu.h"
#include "Gameboy/GbPpu.h"
#include "Gameboy/Gameboy.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/Debugger.h"
#include "Debugger/DebugBreakHelper.h"
#include "Debugger/IEventManager.h"
#include "SNES/SnesDefaultVideoFilter.h"

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

DebugEventInfo GbEventManager::GetEvent(uint16_t y, uint16_t x)
{
	auto lock = _lock.AcquireSafe();

	x /= 2; //convert to cycle value
	y /= 2; //convert to scanline value

	//Search without including larger background color first
	for(DebugEventInfo& evt : _sentEvents) {
		if(evt.Cycle == x && evt.Scanline == y) {
			return evt;
		}
	}

	//If no exact match, extend to the background color
	for(int i = (int)_sentEvents.size() - 1; i >= 0; i--){
		DebugEventInfo& evt = _sentEvents[i];
		if(std::abs((int)evt.Cycle - (int)x) <= 1 && std::abs((int)evt.Scanline - (int)y) <= 1) {
			return evt;
		}
	}

	DebugEventInfo empty = {};
	empty.ProgramCounter = 0xFFFFFFFF;
	return empty;
}

void GbEventManager::SetConfiguration(BaseEventViewerConfig& config)
{
	_config = (GbEventViewerConfig&)config;
}

uint32_t GbEventManager::GetEventCount()
{
	auto lock = _lock.AcquireSafe();
	FilterEvents();
	return (uint32_t)_sentEvents.size();
}

void GbEventManager::ClearFrameEvents()
{
	_prevDebugEvents = _debugEvents;
	_debugEvents.clear();
}

EventViewerCategoryCfg GbEventManager::GetEventConfig(DebugEventInfo& evt)
{
	switch(evt.Type) {
		default: return {};
		case DebugEventType::Breakpoint: return _config.MarkedBreakpoints;
		case DebugEventType::Irq: return _config.Irq;
		case DebugEventType::Register:
			uint16_t reg = evt.Operation.Address & 0xFFFF;
			bool isWrite = evt.Operation.Type == MemoryOperationType::Write || evt.Operation.Type == MemoryOperationType::DmaWrite;
			if(reg >= 0xFE00 && reg <= 0xFE9F) {
				return isWrite ? _config.PpuRegisterOamWrites : _config.PpuRegisterReads;
			} else if(reg >= 0xFF42 && reg <= 0xFF43) {
				return isWrite ? _config.PpuRegisterBgScrollWrites : _config.PpuRegisterReads;
			} else if(reg >= 0x8000 && reg <= 0x9FFF) {
				return isWrite ? _config.PpuRegisterVramWrites : _config.PpuRegisterReads;
			} else if((reg >= 0xFF47 && reg <= 0xFF49) || (reg >= 0xFF68 && reg <= 0xFF6B)) {
				return isWrite ? _config.PpuRegisterCgramWrites : _config.PpuRegisterReads;
			} else if(reg >= 0xFF4A && reg <= 0xFF4B) {
				return isWrite ? _config.PpuRegisterWindowWrites : _config.PpuRegisterReads;
			} else if(reg >= 0xFF40 && reg <= 0xFF70) {
				return isWrite ? _config.PpuRegisterOtherWrites : _config.PpuRegisterReads;
			} else if(reg >= 0xFF10 && reg <= 0xFF3F) {
				return isWrite ? _config.ApuRegisterWrites : _config.ApuRegisterReads;
			} else {
				return isWrite ? _config.CpuRegisterWrites : _config.CpuRegisterReads;
			}

			return {};
	}
}

void GbEventManager::FilterEvents()
{
	auto lock = _lock.AcquireSafe();
	_sentEvents.clear();

	vector<DebugEventInfo> events = _snapshot;
	if(_config.ShowPreviousFrameEvents && _snapshotScanline != 0) {
		uint32_t key = (_snapshotScanline << 16) + _snapshotCycle;
		for(DebugEventInfo& evt : _prevDebugEvents) {
			uint32_t evtKey = (evt.Scanline << 16) + evt.Cycle;
			if(evtKey > key) {
				events.push_back(evt);
			}
		}
	}

	for(DebugEventInfo& evt : events) {
		if(GetEventConfig(evt).Visible) {
			_sentEvents.push_back(evt);
		}
	}
}

void GbEventManager::DrawEvent(DebugEventInfo& evt, bool drawBackground, uint32_t* buffer)
{
	EventViewerCategoryCfg evtCfg = GetEventConfig(evt);
	uint32_t color = evtCfg.Color;

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
			if(j + x >= GbEventManager::ScanlineWidth) {
				continue;
			}

			int32_t pos = (y + i) * GbEventManager::ScanlineWidth + x + j;
			if(pos < 0 || pos >= GbEventManager::ScanlineWidth * (int)_scanlineCount * 2) {
				continue;
			}
			buffer[pos] = color;
		}
	}
}

uint32_t GbEventManager::TakeEventSnapshot()
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

FrameInfo GbEventManager::GetDisplayBufferSize()
{
	FrameInfo size;
	size.Width = GbEventManager::ScanlineWidth;
	size.Height = _scanlineCount * 2;
	return size;
}

void GbEventManager::GetDisplayBuffer(uint32_t* buffer, uint32_t bufferSize)
{
	auto lock = _lock.AcquireSafe();
	FrameInfo size = GetDisplayBufferSize();
	if(_snapshotScanline < 0 || bufferSize < size.Width * size.Height * sizeof(uint32_t)) {
		return;
	}

	uint16_t *src = _ppuBuffer;
	for(uint32_t y = 0, len = GbEventManager::ScreenHeight*2; y < len; y++) {
		for(uint32_t x = 0; x < GbEventManager::ScanlineWidth; x++) {
			int srcOffset = (y >> 1) * 456 + (x >> 1);
			buffer[y*GbEventManager::ScanlineWidth + x] = SnesDefaultVideoFilter::ToArgb(src[srcOffset]);
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

	FilterEvents();
	for(DebugEventInfo &evt : _sentEvents) {
		DrawEvent(evt, true, buffer);
	}
	for(DebugEventInfo &evt : _sentEvents) {
		DrawEvent(evt, false, buffer);
	}
}
