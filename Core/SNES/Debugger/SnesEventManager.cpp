#include "pch.h"
#include "SNES/Debugger/SnesEventManager.h"
#include "SNES/SnesCpu.h"
#include "SNES/SnesPpu.h"
#include "SNES/SnesDmaController.h"
#include "SNES/SnesMemoryManager.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/Debugger.h"
#include "Debugger/DebugBreakHelper.h"
#include "Debugger/BaseEventManager.h"
#include "Shared/ColorUtilities.h"

SnesEventManager::SnesEventManager(Debugger *debugger, SnesCpu *cpu, SnesPpu *ppu, SnesMemoryManager *memoryManager, SnesDmaController *dmaController)
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
	evt.Flags = (uint32_t)EventFlags::ReadWriteOp;
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

	evt.ProgramCounter = _debugger->GetProgramCounter(CpuType::Snes, true);

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
	
	evt.ProgramCounter = (_cpu->GetState().K << 16) | _cpu->GetState().PC;

	_debugEvents.push_back(evt);
}

DebugEventInfo SnesEventManager::GetEvent(uint16_t y, uint16_t x)
{
	auto lock = _lock.AcquireSafe();

	x *= 2; //convert to hclock value
	y /= 2; //convert to scanline value

	//Search without including larger background color first
	for(int i = (int)_sentEvents.size() - 1; i >= 0; i--) {
		DebugEventInfo& evt = _sentEvents[i];
		if((x >= evt.Cycle && x <= evt.Cycle + 2) && evt.Scanline == y) {
			return evt;
		}
	}

	//If no exact match, extend to the background color
	for(int i = (int)_sentEvents.size() - 1; i >= 0; i--) {
		DebugEventInfo& evt = _sentEvents[i];
		if((x >= evt.Cycle - 4 && x <= evt.Cycle + 6) && std::abs((int)evt.Scanline - (int)y) <= 1) {
			return evt;
		}
	}

	DebugEventInfo empty = {};
	empty.ProgramCounter = 0xFFFFFFFF;
	return empty;
}

bool SnesEventManager::ShowPreviousFrameEvents()
{
	return _config.ShowPreviousFrameEvents;
}

void SnesEventManager::SetConfiguration(BaseEventViewerConfig& config)
{
	_config = (SnesEventViewerConfig&)config;
}

EventViewerCategoryCfg SnesEventManager::GetEventConfig(DebugEventInfo& evt)
{
	bool isDma = evt.Operation.Type == MemoryOperationType::DmaWrite || evt.Operation.Type == MemoryOperationType::DmaRead;
	if(evt.Type == DebugEventType::Register && isDma && !_config.ShowDmaChannels[evt.DmaChannel & 0x07]) {
		return {};
	}

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

void SnesEventManager::ConvertScanlineCycleToRowColumn(int32_t& x, int32_t& y)
{
	y *= 2;
	x /= 2;
}

uint32_t SnesEventManager::TakeEventSnapshot(bool forAutoRefresh)
{
	DebugBreakHelper breakHelper(_debugger);
	auto lock = _lock.AcquireSafe();

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

	_snapshotCurrentFrame = _debugEvents;
	_snapshotPrevFrame = _prevDebugEvents;
	_snapshotScanline = scanline;
	_snapshotCycle = cycle;
	_forAutoRefresh = forAutoRefresh;
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

void SnesEventManager::DrawScreen(uint32_t *buffer)
{
	//Skip the first 7 blank lines in the buffer when overscan mode is off
	uint16_t *src = _ppuBuffer + (_overscanMode ? 0 : (_useHighResOutput ? (512 * 14) : (256 * 7)));

	for(uint32_t y = 0, len = _overscanMode ? 239*2 : 224*2; y < len; y++) {
		for(uint32_t x = 0; x < 512; x++) {
			int srcOffset = _useHighResOutput ? ((y << 9) | x) : (((y >> 1) << 8) | (x >> 1));
			buffer[(y + 2)*SnesEventManager::ScanlineWidth + x + 22*2] = ColorUtilities::Rgb555ToArgb(src[srcOffset]);
		}
	}
}
