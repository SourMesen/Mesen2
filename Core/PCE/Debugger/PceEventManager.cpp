#include "stdafx.h"
#include "PCE/Debugger/PceEventManager.h"
#include "PCE/PceConsole.h"
#include "PCE/PceCpu.h"
#include "PCE/PcePpu.h"
#include "PCE/PceMemoryManager.h"
#include "PCE/PceConstants.h"
#include "PCE/PceDefaultVideoFilter.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/Debugger.h"
#include "Debugger/DebugBreakHelper.h"
#include "Debugger/BaseEventManager.h"

PceEventManager::PceEventManager(Debugger *debugger, PceConsole *console)
{
	_debugger = debugger;
	_cpu = console->GetCpu();
	_ppu = console->GetPpu();
	_memoryManager = console->GetMemoryManager();

	_ppuBuffer = new uint16_t[PceConstants::MaxScreenWidth * PceConstants::ScreenHeight];
	memset(_ppuBuffer, 0, PceConstants::MaxScreenWidth * PceConstants::ScreenHeight * sizeof(uint16_t));
}

PceEventManager::~PceEventManager()
{
	delete[] _ppuBuffer;
}

void PceEventManager::AddEvent(DebugEventType type, MemoryOperationInfo &operation, int32_t breakpointId)
{
	DebugEventInfo evt = {};
	evt.Type = type;
	evt.Operation = operation;
	evt.Scanline = (int16_t)_ppu->GetScanline();
	evt.Cycle = _ppu->GetHClock();
	evt.BreakpointId = breakpointId;
	evt.DmaChannel = -1;
	evt.ProgramCounter = _cpu->GetState().PC;
	_debugEvents.push_back(evt);
}

void PceEventManager::AddEvent(DebugEventType type)
{
	DebugEventInfo evt = {};
	evt.Type = type;
	evt.Scanline = _ppu->GetScanline();
	evt.Cycle = _ppu->GetHClock();
	evt.BreakpointId = -1;
	evt.DmaChannel = -1;
	evt.ProgramCounter = _cpu->GetState().PC;
	_debugEvents.push_back(evt);
}

DebugEventInfo PceEventManager::GetEvent(uint16_t y, uint16_t x)
{
	auto lock = _lock.AcquireSafe();

	uint16_t cycle = x; //convert to hclock value
	uint16_t scanline = y / 2; //convert to scanline value

	//Search without including larger background color first
	for(int i = (int)_sentEvents.size() - 1; i >= 0; i--) {
		DebugEventInfo& evt = _sentEvents[i];
		if(evt.Cycle == cycle && evt.Scanline == scanline) {
			return evt;
		}
	}

	//If no exact match, extend to the background color
	for(int i = (int)_sentEvents.size() - 1; i >= 0; i--) {
		DebugEventInfo& evt = _sentEvents[i];
		if(std::abs((int)evt.Cycle - cycle) <= 1 && std::abs((int)evt.Scanline - scanline) <= 1) {
			return evt;
		}
	}

	DebugEventInfo empty = {};
	empty.ProgramCounter = 0xFFFFFFFF;
	return empty;
}

bool PceEventManager::ShowPreviousFrameEvents()
{
	return _config.ShowPreviousFrameEvents;
}

void PceEventManager::SetConfiguration(BaseEventViewerConfig& config)
{
	_config = (PceEventViewerConfig&)config;
}

EventViewerCategoryCfg PceEventManager::GetEventConfig(DebugEventInfo& evt)
{
	switch(evt.Type) {
		default: return {};
		case DebugEventType::Breakpoint: return _config.MarkedBreakpoints;
		case DebugEventType::Irq: return _config.Irq;
		case DebugEventType::Register:
			uint16_t reg = evt.Operation.Address & 0x1FFF;
			bool isWrite = evt.Operation.Type == MemoryOperationType::Write;

			if(reg <= 0x3FF) {
				return isWrite ? _config.VdcWrites : _config.VdcReads;
			} else if(reg <= 0x7FF) {
				return isWrite ? _config.VceWrites : _config.VceReads;
			} else if(reg <= 0xBFF) {
				return isWrite ? _config.PsgWrites : _config.PsgReads;
			} else if(reg <= 0xFFF) {
				return isWrite ? _config.TimerWrites : _config.TimerReads;
			} else if(reg <= 0x13FF) {
				return isWrite ? _config.IoWrites : _config.IoReads;
			} else if(reg <= 0x17FF) {
				//TODO
			}

			return {};
	}
}

void PceEventManager::ConvertScanlineCycleToRowColumn(int32_t& x, int32_t& y)
{
	y *= 2;
}

uint32_t PceEventManager::TakeEventSnapshot()
{
	DebugBreakHelper breakHelper(_debugger);
	auto lock = _lock.AcquireSafe();

	uint16_t cycle = _ppu->GetHClock();
	uint16_t scanline = _ppu->GetScanline();

	constexpr uint32_t size = PceConstants::MaxScreenWidth * PceConstants::ScreenHeight;
	if(scanline < 14 || scanline >= 256) {
		memcpy(_ppuBuffer, _ppu->GetScreenBuffer(), size * sizeof(uint16_t));
		memcpy(_rowClockDividers, _ppu->GetRowClockDividers(), PceConstants::ScreenHeight);
	} else {
		uint32_t scanlineOffset = (scanline - 14);
		uint32_t offset = PceConstants::MaxScreenWidth * scanlineOffset;
		memcpy(_ppuBuffer, _ppu->GetScreenBuffer(), offset * sizeof(uint16_t));
		memcpy(_ppuBuffer + offset, _ppu->GetPreviousScreenBuffer() + offset, (size - offset) * sizeof(uint16_t));
		
		memcpy(_rowClockDividers, _ppu->GetRowClockDividers(), scanlineOffset);
		memcpy(_rowClockDividers + scanlineOffset, _ppu->GetPreviousRowClockDividers() + scanlineOffset, (PceConstants::ScreenHeight - scanlineOffset));
	}

	_snapshotCurrentFrame = _debugEvents;
	_snapshotPrevFrame = _prevDebugEvents;
	_snapshotScanline = scanline;
	_snapshotCycle = cycle;
	return PceConstants::ScanlineCount;
}

FrameInfo PceEventManager::GetDisplayBufferSize()
{
	FrameInfo size;
	size.Width = PceConstants::ClockPerScanline;
	size.Height = PceConstants::ScanlineCount * 2;
	return size;
}

void PceEventManager::DrawScreen(uint32_t *buffer)
{
	uint16_t *src = _ppuBuffer;

	for(uint32_t y = 0, len = PceConstants::ScreenHeight * 2; y < len; y++) {
		uint16_t scanline = y >> 1;
		uint32_t divider = _rowClockDividers[scanline];
		if(divider == 0) {
			break;
		}

		for(uint32_t x = 0; x < PceConstants::ClockPerScanline; x++) {
			int srcOffset = (scanline * PceConstants::MaxScreenWidth) + (x / divider);
			buffer[(y + 14*2) * PceConstants::ClockPerScanline + x] = PceDefaultVideoFilter::ToArgb(src[srcOffset]);
		}
	}
}
