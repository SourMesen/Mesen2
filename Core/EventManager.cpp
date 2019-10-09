#include "stdafx.h"
#include "EventManager.h"
#include "DebugTypes.h"
#include "Cpu.h"
#include "Ppu.h"
#include "DmaController.h"
#include "Debugger.h"
#include "DebugBreakHelper.h"
#include "DefaultVideoFilter.h"

EventManager::EventManager(Debugger *debugger, Cpu *cpu, Ppu *ppu, DmaController *dmaController)
{
	_debugger = debugger;
	_cpu = cpu;
	_ppu = ppu;
	_dmaController = dmaController;

	_ppuBuffer = new uint16_t[512 * 478];
	memset(_ppuBuffer, 0, 512 * 478 * sizeof(uint16_t));
}

EventManager::~EventManager()
{
	delete[] _ppuBuffer;
}

void EventManager::AddEvent(DebugEventType type, MemoryOperationInfo &operation, int32_t breakpointId)
{
	DebugEventInfo evt;
	evt.Type = type;
	evt.Operation = operation;
	evt.Scanline = _ppu->GetScanline();
	evt.Cycle = _ppu->GetCycle();
	evt.BreakpointId = breakpointId;

	if(operation.Type == MemoryOperationType::DmaRead || operation.Type == MemoryOperationType::DmaWrite) {
		evt.DmaChannel = _dmaController->GetActiveChannel();
		evt.DmaChannelInfo = _dmaController->GetChannelConfig(evt.DmaChannel);
	}

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
	bool isDma = evt.Operation.Type == MemoryOperationType::DmaWrite || evt.Operation.Type == MemoryOperationType::DmaRead;
	bool showEvent = false;
	uint32_t color = 0;
	switch(evt.Type) {
		case DebugEventType::Breakpoint: showEvent = options.ShowMarkedBreakpoints; color = options.BreakpointColor; break;
		case DebugEventType::Irq: showEvent = options.ShowIrq; color = options.IrqColor; break;
		case DebugEventType::Nmi: showEvent = options.ShowNmi; color = options.NmiColor; break;
		case DebugEventType::Register:
			if(isDma && !options.ShowDmaChannels[evt.DmaChannel]) {
				showEvent = false;
				break;
			}

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
	uint32_t y = std::min<uint32_t>(evt.Scanline * 2, _scanlineCount * 2);
	uint32_t x = evt.Cycle * 2;

	for(int i = iMin; i <= iMax; i++) {
		for(int j = jMin; j <= jMax; j++) {
			int32_t pos = (y + i) * 340 * 2 + x + j;
			if(pos < 0 || pos >= 340 * 2 * _scanlineCount * 2) {
				continue;
			}
			buffer[pos] = color;
		}
	}
}

uint32_t EventManager::TakeEventSnapshot(EventViewerDisplayOptions options)
{
	DebugBreakHelper breakHelper(_debugger);
	auto lock = _lock.AcquireSafe();
	_snapshot.clear();

	uint16_t cycle = _ppu->GetCycle();
	uint16_t scanline = _ppu->GetScanline();
	uint32_t key = (scanline << 9) + cycle;

	_overscanMode = _ppu->GetState().OverscanMode;
	_useHighResOutput = _ppu->IsHighResOutput();
	memcpy(_ppuBuffer, _ppu->GetScreenBuffer(), (_useHighResOutput ? (512 * 478) : (256*239)) * sizeof(uint16_t));

	_snapshot = _debugEvents;
	_snapshotScanline = _ppu->GetRealScanline();
	if(options.ShowPreviousFrameEvents && scanline != 0) {
		for(DebugEventInfo &evt : _prevDebugEvents) {
			uint32_t evtKey = (evt.Scanline << 9) + evt.Cycle;
			if(evtKey > key) {
				_snapshot.push_back(evt);
			}
		}
	}

	_scanlineCount = _ppu->GetVblankEndScanline() + 1;
	return _scanlineCount;
}

void EventManager::GetDisplayBuffer(uint32_t *buffer, EventViewerDisplayOptions options)
{
	auto lock = _lock.AcquireSafe();
	_sentEvents.clear();

	for(int i = 0; i < 340 * 2 * _scanlineCount * 2; i++) {
		buffer[i] = 0xFF555555;
	}

	//Skip the first 7 blank lines in the buffer when overscan mode is off
	uint16_t *src = _ppuBuffer + (_overscanMode ? 0 : (_useHighResOutput ? (512 * 14) : (256 * 7)));

	for(uint32_t y = 0, len = _overscanMode ? 239*2 : 224*2; y < len; y++) {
		for(uint32_t x = 0; x < 512; x++) {
			int srcOffset = _useHighResOutput ? ((y << 9) | x) : (((y >> 1) << 8) | (x >> 1));
			buffer[(y + 2)*340*2 + x + 22*2] = DefaultVideoFilter::ToArgb(src[srcOffset]);
		}
	}

	constexpr uint32_t nmiColor = 0xFF55FFFF;
	constexpr uint32_t currentScanlineColor = 0xFFFFFF55;
	int nmiScanline = (_overscanMode ? 240 : 225) * 2 * 340 * 2;
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
