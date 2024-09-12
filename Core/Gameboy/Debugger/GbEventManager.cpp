#include "pch.h"
#include "Gameboy/Debugger/GbEventManager.h"
#include "Gameboy/GbCpu.h"
#include "Gameboy/GbPpu.h"
#include "Gameboy/Gameboy.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/Debugger.h"
#include "Debugger/DebugBreakHelper.h"
#include "Debugger/BaseEventManager.h"
#include "Shared/ColorUtilities.h"

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
	evt.Flags = (uint32_t)EventFlags::ReadWriteOp;
	evt.Operation = operation;
	evt.Scanline = _ppu->GetState().Scanline;
	evt.Cycle = _ppu->GetState().Cycle;
	evt.BreakpointId = breakpointId;
	evt.DmaChannel = -1;
	evt.ProgramCounter = _debugger->GetProgramCounter(CpuType::Gameboy, true);
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

bool GbEventManager::ShowPreviousFrameEvents()
{
	return _config.ShowPreviousFrameEvents;
}

void GbEventManager::SetConfiguration(BaseEventViewerConfig& config)
{
	_config = (GbEventViewerConfig&)config;
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
				return isWrite ? _config.PpuRegisterOamWrites : _config.PpuRegisterOamReads;
			} else if(reg >= 0xFF42 && reg <= 0xFF43) {
				return isWrite ? _config.PpuRegisterBgScrollWrites : _config.PpuRegisterBgScrollReads;
			} else if(reg >= 0x8000 && reg <= 0x9FFF) {
				return isWrite ? _config.PpuRegisterVramWrites : _config.PpuRegisterVramReads;
			} else if((reg >= 0xFF47 && reg <= 0xFF49) || (reg >= 0xFF68 && reg <= 0xFF6B)) {
				return isWrite ? _config.PpuRegisterCgramWrites : _config.PpuRegisterCgramReads;
			} else if(reg >= 0xFF4A && reg <= 0xFF4B) {
				return isWrite ? _config.PpuRegisterWindowWrites : _config.PpuRegisterWindowReads;
			} else if(reg >= 0xFF40 && reg <= 0xFF70) {
				return isWrite ? _config.PpuRegisterOtherWrites : _config.PpuRegisterOtherReads;
			} else if(reg >= 0xFF10 && reg <= 0xFF3F) {
				return isWrite ? _config.ApuRegisterWrites : _config.ApuRegisterReads;
			} else if(reg == 0xFF00) {
				return isWrite ? _config.InputWrites : _config.InputReads;
			} else if(reg == 0xFF01 || reg == 0xFF02) {
				return isWrite ? _config.SerialWrites : _config.SerialReads;
			} else if(reg >= 0xFF03 && reg <= 0xFF07) {
				return isWrite ? _config.TimerWrites : _config.TimerReads;
			}

			return isWrite ? _config.OtherRegisterWrites : _config.OtherRegisterReads;
	}
}

void GbEventManager::ConvertScanlineCycleToRowColumn(int32_t& x, int32_t& y)
{
	y *= 2;
	x *= 2;
}

uint32_t GbEventManager::TakeEventSnapshot(bool forAutoRefresh)
{
	DebugBreakHelper breakHelper(_debugger);
	auto lock = _lock.AcquireSafe();

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

	_snapshotCurrentFrame = _debugEvents;
	_snapshotPrevFrame = _prevDebugEvents;
	_snapshotScanline = scanline;
	_snapshotCycle = cycle;
	_forAutoRefresh = forAutoRefresh;
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

void GbEventManager::DrawScreen(uint32_t* buffer)
{
	uint16_t *src = _ppuBuffer;
	for(uint32_t y = 0, len = GbEventManager::ScreenHeight*2; y < len; y++) {
		for(uint32_t x = 0; x < GbEventManager::ScanlineWidth; x++) {
			int srcOffset = (y >> 1) * 456 + (x >> 1);
			buffer[y*GbEventManager::ScanlineWidth + x] = ColorUtilities::Rgb555ToArgb(src[srcOffset]);
		}
	}
}
