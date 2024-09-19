#include "pch.h"
#include "WS/Debugger/WsEventManager.h"
#include "WS/WsCpu.h"
#include "WS/WsPpu.h"
#include "WS/WsConsole.h"
#include "Shared/ColorUtilities.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/Debugger.h"
#include "Debugger/DebugBreakHelper.h"
#include "Debugger/BaseEventManager.h"

WsEventManager::WsEventManager(Debugger* debugger, WsConsole* console, WsCpu* cpu, WsPpu* ppu)
{
	_debugger = debugger;
	_console = console;
	_cpu = cpu;
	_ppu = ppu;

	_ppuBuffer = new uint16_t[WsConstants::MaxPixelCount];
	memset(_ppuBuffer, 0, WsConstants::MaxPixelCount * sizeof(uint16_t));
}

WsEventManager::~WsEventManager()
{
	delete[] _ppuBuffer;
}

void WsEventManager::AddEvent(DebugEventType type, MemoryOperationInfo& operation, int32_t breakpointId)
{
	DebugEventInfo evt = {};
	evt.Type = type;
	evt.Flags = (uint32_t)EventFlags::ReadWriteOp;
	evt.Operation = operation;
	evt.Scanline = _ppu->GetScanline();
	evt.Cycle = _ppu->GetCycle();
	evt.BreakpointId = breakpointId;
	evt.DmaChannel = -1;

	evt.ProgramCounter = _debugger->GetProgramCounter(CpuType::Ws, true);
	_debugEvents.push_back(evt);
}

void WsEventManager::AddEvent(DebugEventType type)
{
	DebugEventInfo evt = {};
	evt.Type = type;
	evt.Scanline = _ppu->GetScanline();
	evt.Cycle = _ppu->GetCycle();
	evt.BreakpointId = -1;
	evt.DmaChannel = -1;
	evt.ProgramCounter = _cpu->GetProgramCounter();
	_debugEvents.push_back(evt);
}

DebugEventInfo WsEventManager::GetEvent(uint16_t y, uint16_t x)
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

bool WsEventManager::ShowPreviousFrameEvents()
{
	return _config.ShowPreviousFrameEvents;
}

void WsEventManager::SetConfiguration(BaseEventViewerConfig& config)
{
	_config = (WsEventViewerConfig&)config;
}

EventViewerCategoryCfg WsEventManager::GetEventConfig(DebugEventInfo& evt)
{
	switch(evt.Type) {
		default: return {};
		case DebugEventType::Breakpoint: return _config.MarkedBreakpoints;
		case DebugEventType::Irq: return _config.Irq;
		case DebugEventType::Register:
			bool isWrite = evt.Operation.Type == MemoryOperationType::Write || evt.Operation.Type == MemoryOperationType::DmaWrite;
			uint32_t port = evt.Operation.Address;
			if(evt.Operation.MemType == MemoryType::WsMemory) {
				if(evt.Operation.Address >= 0xFE00) {
					return isWrite ? _config.PpuPaletteWrite : _config.PpuPaletteRead;
				} else {
					return isWrite ? _config.PpuVramWrite : _config.PpuVramRead;
				}
			} else {
				if(port <= 0x3F) {
					if(port >= 0x08 && port <= 0x0F) {
						return isWrite ? _config.PpuWindowWrite : _config.PpuWindowRead;
					} else if(port >= 0x10 && port <= 0x13) {
						return isWrite ? _config.PpuScrollWrite : _config.PpuScrollRead;
					} else if(port == 0x02 && !isWrite) {
						return _config.PpuVCounterRead;
					} else if(port >= 0x1C && port <= 0x3F) {
						return isWrite ? _config.PpuPaletteWrite : _config.PpuPaletteRead;
					} else {
						return isWrite ? _config.PpuOtherWrite : _config.PpuOtherRead;
					}

				} else if(port >= 0x40 && port <= 0x53) {
					return isWrite ? _config.DmaWrite : _config.DmaRead;
				} else if((port >= 0x80 && port <= 0x9F) || (port >= 0x64 && port <= 0x6B)) {
					return isWrite ? _config.AudioWrite : _config.AudioRead;
				} else if(port >= 0xA2 && port <= 0xAB) {
					return isWrite ? _config.TimerWrite : _config.TimerRead;
				} else if(port == 0xB0 || port == 0xB2 || port == 0xB4 || port == 0xB6 || port == 0xB7) {
					return isWrite ? _config.IrqWrite : _config.IrqRead;
				} else if(port == 0xB1 || port == 0xB3) {
					return isWrite ? _config.SerialWrite : _config.SerialRead;
				} else if(port == 0xB5) {
					return isWrite ? _config.InputWrite : _config.InputRead;
				} else if(port >= 0xBA && port <= 0xBF) {
					return isWrite ? _config.EepromWrite : _config.EepromRead;
				} else if(port >= 0xC0) {
					return isWrite ? _config.CartWrite : _config.CartRead;
				} else if(port == 0x60 || port == 0x62 || port == 0xA0 || (port >= 0x70 && port <= 0x77)) {
					//system control 1/2/3, tft control, etc.
					return isWrite ? _config.OtherWrite : _config.OtherRead;
				}
			}

			return {};
	}
}

void WsEventManager::ConvertScanlineCycleToRowColumn(int32_t& x, int32_t& y)
{
	y *= 2;
	x *= 2;
}

uint32_t WsEventManager::TakeEventSnapshot(bool forAutoRefresh)
{
	DebugBreakHelper breakHelper(_debugger);
	auto lock = _lock.AcquireSafe();

	uint16_t cycle = _ppu->GetCycle();
	uint16_t scanline = _ppu->GetScanline();

	if(scanline > _ppu->GetVisibleScanlineCount()) {
		memcpy(_ppuBuffer, _ppu->GetScreenBuffer(false), WsConstants::MaxPixelCount * sizeof(uint16_t));
	} else {
		uint32_t offset = _ppu->GetScreenWidth() * (std::max<int>(1, scanline) - 1);
		memcpy(_ppuBuffer, _ppu->GetScreenBuffer(false), offset * sizeof(uint16_t));
		memcpy(_ppuBuffer + offset, _ppu->GetScreenBuffer(true) + offset, (WsConstants::MaxPixelCount - offset) * sizeof(uint16_t));
	}

	_snapshotCurrentFrame = _debugEvents;
	_snapshotPrevFrame = _prevDebugEvents;
	_snapshotScanline = scanline;
	_snapshotCycle = cycle;
	_forAutoRefresh = forAutoRefresh;
	_scanlineCount = _ppu->GetScanlineCount();
	return _scanlineCount;
}

FrameInfo WsEventManager::GetDisplayBufferSize()
{
	FrameInfo size;
	size.Width = WsEventManager::ScanlineWidth;
	size.Height = _scanlineCount * 2;
	return size;
}

void WsEventManager::DrawScreen(uint32_t* buffer)
{
	uint16_t *src = _ppuBuffer;
	uint16_t screenWidth = _ppu->GetScreenWidth();
	for(uint32_t y = 0, len = WsConstants::ScreenHeight * 2; y < len; y++) {
		for(uint32_t x = 0; x < WsConstants::ScreenWidth * 2; x++) {
			int srcOffset = (y >> 1) * screenWidth + (x >> 1);
			buffer[(y + 2)*WsEventManager::ScanlineWidth + x] = ColorUtilities::Bgr444ToArgb(src[srcOffset]);
		}
	}
}
