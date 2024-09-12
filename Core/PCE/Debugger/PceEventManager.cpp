#include "pch.h"
#include "PCE/Debugger/PceEventManager.h"
#include "PCE/PceConsole.h"
#include "PCE/PceCpu.h"
#include "PCE/PceVdc.h"
#include "PCE/PceVce.h"
#include "PCE/PceVpc.h"
#include "PCE/PceMemoryManager.h"
#include "PCE/PceConstants.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/Debugger.h"
#include "Debugger/DebugBreakHelper.h"
#include "Debugger/BaseEventManager.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"

PceEventManager::PceEventManager(Debugger *debugger, PceConsole *console)
{
	_debugger = debugger;
	_emu = debugger->GetEmulator();
	_console = console;
	_cpu = console->GetCpu();
	_vdc = console->GetVdc();
	_vpc = console->GetVpc();
	_vce = console->GetVce();
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
	evt.Flags = (uint32_t)EventFlags::ReadWriteOp;
	evt.Operation = operation;
	evt.Scanline = (int16_t)_vdc->GetScanline();
	evt.Cycle = _vdc->GetHClock();
	evt.BreakpointId = breakpointId;
	evt.DmaChannel = -1;
	evt.ProgramCounter = _debugger->GetProgramCounter(CpuType::Pce, true);

	if(operation.Type == MemoryOperationType::Write && (operation.Address & 0x1FFF) < 0x400) {
		if(_console->IsSuperGrafx()) {
			if((operation.Address & 0x1A) == 2) {
				evt.RegisterId = _vdc->GetState().CurrentReg; //VDC reg
			} else {
				//TODOv2 - supergrafx VPC writes, not VDC
			}
		} else {
			if((operation.Address & 0x03) >= 2) {
				evt.RegisterId = _vdc->GetState().CurrentReg; //VDC reg
			}
		}
	}

	_debugEvents.push_back(evt);
}

void PceEventManager::AddEvent(DebugEventType type)
{
	DebugEventInfo evt = {};
	evt.Type = type;
	evt.Scanline = _vdc->GetScanline();
	evt.Cycle = _vdc->GetHClock();
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
		if((evt.Cycle == cycle || evt.Cycle == cycle + 1) && evt.Scanline == scanline) {
			return evt;
		}
	}

	//If no exact match, extend to the background color
	for(int i = (int)_sentEvents.size() - 1; i >= 0; i--) {
		DebugEventInfo& evt = _sentEvents[i];
		if(cycle >= evt.Cycle - 2 && cycle <= evt.Cycle + 3 && std::abs((int)evt.Scanline - scanline) <= 1) {
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
				if(isWrite) {
					switch(evt.RegisterId) {
						case -1: return _config.VdcRegSelectWrites;
						case 0: return _config.VdcVramWrites;
						case 1: return _config.VdcVramReads;
						case 2: return _config.VdcVramWrites;
						case 5: return _config.VdcControlWrites;
						case 6: return _config.VdcRcrWrites;
						
						case 7: case 8:
							return _config.VdcScrollWrites;

						case 9: return _config.VdcMemoryWidthWrites;
						
						case 0xA: case 0xB: case 0xC: case 0xD: case 0xE:
							return _config.VdcHvConfigWrites;

						case 0xF: case 0x10: case 0x11: case 0x12: case 0x13:
							return _config.VdcDmaWrites;
					}
				} else {
					if((reg & 0x03) == 0) {
						return _config.VdcStatusReads;
					} else if((reg & 0x03) >= 2) {
						return _config.VdcVramReads;
					}
				}
			} else if(reg <= 0x7FF) {
				return isWrite ? _config.VceWrites : _config.VceReads;
			} else if(reg <= 0xBFF) {
				return isWrite ? _config.PsgWrites : _config.PsgReads;
			} else if(reg <= 0xFFF) {
				return isWrite ? _config.TimerWrites : _config.TimerReads;
			} else if(reg <= 0x13FF) {
				return isWrite ? _config.IoWrites : _config.IoReads;
			} else if(reg <= 0x17FF) {
				return isWrite ? _config.IrqControlWrites : _config.IrqControlReads;
			} else if(reg <= 0x1BFF) {
				if(reg & 0x200) {
					return isWrite ? _config.ArcadeCardWrites : _config.ArcadeCardReads;
				} else {
					switch(reg & 0x0F) {
						case 8:
							return isWrite ? _config.AdpcmWrites : _config.CdRomReads;

						case 9: case 0xA: case 0xB: case 0xC: case 0xD: case 0xE:
							return isWrite ? _config.AdpcmWrites : _config.AdpcmReads;

						default:
							return isWrite ? _config.CdRomWrites : _config.CdRomReads;
					}
				}
			}

			return {};
	}
}

void PceEventManager::ConvertScanlineCycleToRowColumn(int32_t& x, int32_t& y)
{
	y *= 2;
}

uint32_t PceEventManager::TakeEventSnapshot(bool forAutoRefresh)
{
	DebugBreakHelper breakHelper(_debugger);
	auto lock = _lock.AcquireSafe();

	uint16_t cycle = _vdc->GetHClock();
	uint16_t scanline = _vdc->GetScanline();

	constexpr uint32_t size = PceConstants::MaxScreenWidth * PceConstants::ScreenHeight;
	if(scanline < 14 || scanline >= 256) {
		memcpy(_ppuBuffer, _vpc->GetScreenBuffer(), size * sizeof(uint16_t));
		memcpy(_rowClockDividers, _vpc->GetScreenBuffer()+size, PceConstants::ScreenHeight * sizeof(uint16_t));
	} else {
		uint32_t scanlineOffset = (scanline - 14);
		uint32_t offset = PceConstants::MaxScreenWidth * scanlineOffset;
		memcpy(_ppuBuffer, _vpc->GetScreenBuffer(), offset * sizeof(uint16_t));
		memcpy(_ppuBuffer + offset, _vpc->GetPreviousScreenBuffer() + offset, (size - offset) * sizeof(uint16_t));
		
		memcpy(_rowClockDividers, _vpc->GetScreenBuffer()+size, scanlineOffset * sizeof(uint16_t));
		memcpy(_rowClockDividers + scanlineOffset, _vpc->GetPreviousScreenBuffer() + size + scanlineOffset, (PceConstants::ScreenHeight - scanlineOffset) * sizeof(uint16_t));
	}

	_snapshotCurrentFrame = _debugEvents;
	_snapshotPrevFrame = _prevDebugEvents;
	_snapshotScanline = scanline;
	_snapshotCycle = cycle;
	_forAutoRefresh = forAutoRefresh;
	_scanlineCount = _vce->GetScanlineCount();
	return _scanlineCount;
}

FrameInfo PceEventManager::GetDisplayBufferSize()
{
	FrameInfo size;
	size.Width = PceConstants::ClockPerScanline;
	size.Height = _scanlineCount * 2;
	return size;
}

void PceEventManager::DrawScreen(uint32_t *buffer)
{
	uint16_t *src = _ppuBuffer;
	uint32_t* palette = _emu->GetSettings()->GetPcEngineConfig().Palette;

	for(uint32_t y = 0, len = PceConstants::ScreenHeight * 2; y < len; y++) {
		uint16_t scanline = y >> 1;
		uint32_t divider = _rowClockDividers[scanline];
		if(divider == 0) {
			break;
		}

		for(uint32_t x = 0; x < PceConstants::ClockPerScanline; x++) {
			int srcOffset = (scanline * PceConstants::MaxScreenWidth) + (x / divider);
			buffer[(y + 14*2) * PceConstants::ClockPerScanline + x] = palette[src[srcOffset] & 0x3FF];
		}
	}
}
