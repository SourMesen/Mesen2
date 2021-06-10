#include "stdafx.h"
#include "SNES/Debugger/SnesEventManager.h"
#include "SNES/Cpu.h"
#include "SNES/Ppu.h"
#include "SNES/DmaController.h"
#include "SNES/MemoryManager.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/Debugger.h"
#include "Debugger/DebugBreakHelper.h"
#include "Debugger/IEventManager.h"
#include "SNES/SnesDefaultVideoFilter.h"

SnesEventManager::SnesEventManager(Debugger *debugger, Cpu *cpu, Ppu *ppu, MemoryManager *memoryManager, DmaController *dmaController)
{
	_debugger = debugger;
	_cpu = cpu;
	_ppu = ppu;
	_memoryManager = memoryManager;
	_dmaController = dmaController;

	_ppuBuffer = new uint16_t[512 * 478];
	memset(_ppuBuffer, 0, 512 * 478 * sizeof(uint16_t));
}

SnesEventManager::~SnesEventManager()
{
	delete[] _ppuBuffer;
}

void SnesEventManager::AddEvent(DebugEventType type, MemoryOperationInfo &operation, int32_t breakpointId)
{
	DebugEventInfo evt = {};
	evt.Type = type;
	evt.Operation = operation;
	evt.Scanline = (int16_t)_ppu->GetScanline();
	evt.Cycle = _memoryManager->GetHClock();
	evt.BreakpointId = breakpointId;

	if(operation.Type == MemoryOperationType::DmaRead || operation.Type == MemoryOperationType::DmaWrite) {
		evt.DmaChannel = _dmaController->GetActiveChannel();
		evt.DmaChannelInfo = _dmaController->GetChannelConfig(evt.DmaChannel & 0x07);
	} else {
		evt.DmaChannel = -1;
	}

	CpuState state = _cpu->GetState();
	evt.ProgramCounter = (state.K << 16) | state.PC;

	_debugEvents.push_back(evt);
}

void SnesEventManager::AddEvent(DebugEventType type)
{
	DebugEventInfo evt = {};
	evt.Type = type;
	evt.Scanline = _ppu->GetScanline();
	evt.Cycle = _memoryManager->GetHClock();
	evt.BreakpointId = -1;
	evt.DmaChannel = -1;
	
	CpuState state = _cpu->GetState();
	evt.ProgramCounter = (state.K << 16) | state.PC;

	_debugEvents.push_back(evt);
}

void SnesEventManager::GetEvents(DebugEventInfo *eventArray, uint32_t &maxEventCount)
{
	auto lock = _lock.AcquireSafe();
	uint32_t eventCount = std::min(maxEventCount, (uint32_t)_sentEvents.size());
	memcpy(eventArray, _sentEvents.data(), eventCount * sizeof(DebugEventInfo));
	maxEventCount = eventCount;
}

DebugEventInfo SnesEventManager::GetEvent(uint16_t scanline, uint16_t cycle)
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

void SnesEventManager::SetConfiguration(BaseEventViewerConfig& config)
{
	_config = (SnesEventViewerConfig&)config;
}

uint32_t SnesEventManager::GetEventCount()
{
	auto lock = _lock.AcquireSafe();
	FilterEvents();
	return (uint32_t)_sentEvents.size();
}

void SnesEventManager::ClearFrameEvents()
{
	_prevDebugEvents = _debugEvents;
	_debugEvents.clear();
}

EventViewerCategoryCfg SnesEventManager::GetEventConfig(DebugEventInfo& evt)
{
	switch(evt.Type) {
		default: return {};
		case DebugEventType::Breakpoint: return _config.MarkedBreakpoints;
		case DebugEventType::Irq: return _config.Irq;
		case DebugEventType::Nmi: return _config.Nmi;
		case DebugEventType::Register:
			uint16_t reg = evt.Operation.Address & 0xFFFF;
			bool isWrite = evt.Operation.Type == MemoryOperationType::Write || evt.Operation.Type == MemoryOperationType::DmaWrite;
			if(reg <= 0x213F) {
				if(isWrite) {
					if(reg >= 0x2101 && reg <= 0x2104) {
						return _config.PpuRegisterOamWrites;
					} else if(reg >= 0x2105 && reg <= 0x210C) {
						return _config.PpuRegisterBgOptionWrites;
					} else if(reg >= 0x210D && reg <= 0x2114) {
						return _config.PpuRegisterBgScrollWrites;
					} else if(reg >= 0x2115 && reg <= 0x2119) {
						return _config.PpuRegisterVramWrites;
					} else if(reg >= 0x211A && reg <= 0x2120) {
						return _config.PpuRegisterMode7Writes;
					} else if(reg >= 0x2121 && reg <= 0x2122) {
						return _config.PpuRegisterCgramWrites;
					} else if(reg >= 0x2123 && reg <= 0x212B) {
						return _config.PpuRegisterWindowWrites;
					} else {
						return _config.PpuRegisterOtherWrites;
					}
				} else {
					return _config.PpuRegisterReads;
				}
			} else if(reg <= 0x217F) {
				return isWrite ? _config.ApuRegisterWrites : _config.ApuRegisterReads;
			} else if(reg <= 0x2183) {
				return isWrite ? _config.WorkRamRegisterWrites : _config.WorkRamRegisterReads;
			} else if(reg >= 0x4000) {
				return isWrite ? _config.CpuRegisterWrites : _config.CpuRegisterReads;
			}

			return {};
	}
}

void SnesEventManager::FilterEvents()
{
	auto lock = _lock.AcquireSafe();
	_sentEvents.clear();

	vector<DebugEventInfo> events = _snapshot;
	if(_config.ShowPreviousFrameEvents && _snapshotScanline != 0) {
		uint32_t key = (_snapshotScanline << 16) + _snapshotCycle;
		for(DebugEventInfo &evt : _prevDebugEvents) {
			uint32_t evtKey = (evt.Scanline << 16) + evt.Cycle;
			if(evtKey > key) {
				events.push_back(evt);
			}
		}
	}

	for(DebugEventInfo &evt : events) {
		bool isDma = evt.Operation.Type == MemoryOperationType::DmaWrite || evt.Operation.Type == MemoryOperationType::DmaRead;
		EventViewerCategoryCfg eventCfg = GetEventConfig(evt);
		if(evt.Type == DebugEventType::Register && isDma && !_config.ShowDmaChannels[evt.DmaChannel & 0x07]) {
			continue;
		}

		if(eventCfg.Visible) {
			_sentEvents.push_back(evt);
		}
	}
}

void SnesEventManager::DrawEvent(DebugEventInfo &evt, bool drawBackground, uint32_t *buffer)
{
	EventViewerCategoryCfg eventCfg = GetEventConfig(evt);
	uint32_t color = eventCfg.Color;

	if(drawBackground){
		color = 0xFF000000 | ((color >> 1) & 0x7F7F7F);
	} else {
		color |= 0xFF000000;
	}

	int iMin = drawBackground ? -2 : 0;
	int iMax = drawBackground ? 3 : 1;
	int jMin = drawBackground ? -2 : 0;
	int jMax = drawBackground ? 3 : 1;
	uint32_t y = std::min<uint32_t>(evt.Scanline * 2, _scanlineCount * 2);
	uint32_t x = evt.Cycle / 2;

	for(int i = iMin; i <= iMax; i++) {
		for(int j = jMin; j <= jMax; j++) {
			int32_t pos = (y + i) * SnesEventManager::ScanlineWidth + x + j;
			if(pos < 0 || pos >= SnesEventManager::ScanlineWidth * (int)_scanlineCount * 2) {
				continue;
			}
			buffer[pos] = color;
		}
	}
}

uint32_t SnesEventManager::TakeEventSnapshot()
{
	DebugBreakHelper breakHelper(_debugger);
	auto lock = _lock.AcquireSafe();
	_snapshot.clear();

	uint16_t cycle = _memoryManager->GetHClock();
	uint16_t scanline = _ppu->GetScanline();

	_overscanMode = _ppu->GetState().OverscanMode;
	_useHighResOutput = _ppu->IsHighResOutput();

	if(scanline >= _ppu->GetNmiScanline() || scanline == 0) {
		memcpy(_ppuBuffer, _ppu->GetScreenBuffer(), (_useHighResOutput ? (512 * 478) : (256*239)) * sizeof(uint16_t));
	} else {
		uint16_t adjustedScanline = scanline + (_overscanMode ? 0 : 7);
		uint32_t size = _useHighResOutput ? (512 * 478) : (256 * 239);
		uint32_t offset = _useHighResOutput ? (512 * adjustedScanline * 2) : (256 * adjustedScanline);
		memcpy(_ppuBuffer, _ppu->GetScreenBuffer(), offset * sizeof(uint16_t));
		memcpy(_ppuBuffer+offset, _ppu->GetPreviousScreenBuffer()+offset, (size - offset) * sizeof(uint16_t));
	}

	_snapshot = _debugEvents;
	_snapshotScanline = scanline;
	_snapshotCycle = cycle;
	_scanlineCount = _ppu->GetVblankEndScanline() + 1;
	return _scanlineCount;
}

FrameInfo SnesEventManager::GetDisplayBufferSize()
{
	FrameInfo size;
	size.Width = SnesEventManager::ScanlineWidth;
	size.Height = _scanlineCount * 2;
	return size;
}

void SnesEventManager::GetDisplayBuffer(uint32_t *buffer, uint32_t bufferSize)
{
	auto lock = _lock.AcquireSafe();

	FrameInfo size = GetDisplayBufferSize();
	if(_snapshotScanline < 0 || bufferSize < size.Width * size.Height * sizeof(uint32_t)) {
		return;
	}

	for(uint32_t i = 0; i < size.Width*size.Height; i++) {
		buffer[i] = 0xFF555555;
	}

	//Skip the first 7 blank lines in the buffer when overscan mode is off
	uint16_t *src = _ppuBuffer + (_overscanMode ? 0 : (_useHighResOutput ? (512 * 14) : (256 * 7)));

	for(uint32_t y = 0, len = _overscanMode ? 239*2 : 224*2; y < len; y++) {
		for(uint32_t x = 0; x < 512; x++) {
			int srcOffset = _useHighResOutput ? ((y << 9) | x) : (((y >> 1) << 8) | (x >> 1));
			buffer[(y + 2)*SnesEventManager::ScanlineWidth + x + 22*2] = SnesDefaultVideoFilter::ToArgb(src[srcOffset]);
		}
	}

	constexpr uint32_t nmiColor = 0xFF55FFFF;
	constexpr uint32_t currentScanlineColor = 0xFFFFFF55;
	int nmiScanline = (_overscanMode ? 240 : 225) * 2 * SnesEventManager::ScanlineWidth;
	uint32_t scanlineOffset = _snapshotScanline * 2 * SnesEventManager::ScanlineWidth;
	for(int i = 0; i < SnesEventManager::ScanlineWidth; i++) {
		buffer[nmiScanline + i] = nmiColor;
		buffer[nmiScanline + SnesEventManager::ScanlineWidth + i] = nmiColor;
		if(_snapshotScanline != 0) {
			buffer[scanlineOffset + i] = currentScanlineColor;
			buffer[scanlineOffset + SnesEventManager::ScanlineWidth + i] = currentScanlineColor;
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
