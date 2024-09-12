#include "pch.h"
#include "GBA/Debugger/GbaEventManager.h"
#include "GBA/GbaCpu.h"
#include "GBA/GbaPpu.h"
#include "GBA/GbaMemoryManager.h"
#include "GBA/GbaDmaController.h"
#include "GBA/GbaConsole.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/Debugger.h"
#include "Debugger/DebugBreakHelper.h"
#include "Debugger/BaseEventManager.h"
#include "Shared/ColorUtilities.h"

GbaEventManager::GbaEventManager(Debugger* debugger, GbaCpu* cpu, GbaPpu* ppu, GbaMemoryManager* memoryManager, GbaDmaController* dmaController)
{
	_debugger = debugger;
	_cpu = cpu;
	_ppu = ppu;
	_memoryManager = memoryManager;
	_dmaController = dmaController;

	_ppuBuffer = new uint16_t[GbaConstants::PixelCount];
	memset(_ppuBuffer, 0, GbaConstants::PixelCount * sizeof(uint16_t));
}

GbaEventManager::~GbaEventManager()
{
	delete[] _ppuBuffer;
}

void GbaEventManager::AddEvent(DebugEventType type, MemoryOperationInfo& operation, int32_t breakpointId)
{
	DebugEventInfo evt = {};

	uint16_t cycle = _ppu->GetCycle();
	uint16_t scanline = _ppu->GetScanline();
	evt.Type = type;
	evt.Flags = (uint32_t)EventFlags::ReadWriteOp;
	evt.Operation = operation;
	evt.Scanline = scanline;
	evt.Cycle = cycle;
	evt.BreakpointId = breakpointId;
	
	evt.DmaChannel = _dmaController->DebugGetActiveChannel();
	if(evt.DmaChannel > 0) {
		if(evt.Operation.Type == MemoryOperationType::Write) {
			evt.Operation.Type = MemoryOperationType::DmaWrite;
		} else {
			evt.Operation.Type = MemoryOperationType::DmaRead;
		}
	}

	evt.ProgramCounter = _debugger->GetProgramCounter(CpuType::Gba, true);
	_debugEvents.push_back(evt);
}

void GbaEventManager::AddEvent(DebugEventType type)
{
	DebugEventInfo evt = {};
	evt.Type = type;
	evt.Scanline = _ppu->GetScanline();
	evt.Cycle = _ppu->GetCycle();
	evt.BreakpointId = -1;
	evt.DmaChannel = -1;
	evt.ProgramCounter = _debugger->GetProgramCounter(CpuType::Gba, true);
	_debugEvents.push_back(evt);
}

DebugEventInfo GbaEventManager::GetEvent(uint16_t y, uint16_t x)
{
	auto lock = _lock.AcquireSafe();

	x /= 1; //convert to cycle value
	y /= 4; //convert to scanline value

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

bool GbaEventManager::ShowPreviousFrameEvents()
{
	return _config.ShowPreviousFrameEvents;
}

void GbaEventManager::SetConfiguration(BaseEventViewerConfig& config)
{
	_config = (GbaEventViewerConfig&)config;
}

EventViewerCategoryCfg GbaEventManager::GetEventConfig(DebugEventInfo& evt)
{
	switch(evt.Type) {
		default: return {};
		case DebugEventType::Breakpoint: return _config.MarkedBreakpoints;
		case DebugEventType::Irq: return _config.Irq;
		case DebugEventType::Register:
			uint32_t addr = evt.Operation.Address;
			bool isWrite = evt.Operation.Type == MemoryOperationType::Write || evt.Operation.Type == MemoryOperationType::DmaWrite;
			switch(addr >> 24) {
				case 4:
					addr &= 0xFFFF;
					if(addr < 0x10 || (addr >= 0x20 && addr <= 0x3F) || (addr >= 0x4C && addr <= 0x55)) {
						return isWrite ? _config.PpuRegisterOtherWrites : _config.PpuRegisterOtherReads;
					} else if(addr < 0x20) {
						return isWrite ? _config.PpuRegisterBgScrollWrites : _config.PpuRegisterBgScrollReads;
					} else if(addr >= 0x40 && addr <= 0x4B) {
						return isWrite ? _config.PpuRegisterWindowWrites : _config.PpuRegisterWindowReads;
					} else if(addr >= 0x60 && addr <= 0xA7) {
						return isWrite ? _config.ApuRegisterWrites : _config.ApuRegisterReads;
					} else if(addr >= 0xB0 && addr <= 0xDF) {
						return isWrite ? _config.DmaRegisterWrites : _config.DmaRegisterReads;
					} else if(addr >= 0x100 && addr <= 0x10F) {
						return isWrite ? _config.TimerWrites : _config.TimerReads;
					} else if((addr >= 0x120 && addr <= 0x12B) || (addr >= 0x134 && addr <= 0x159)) {
						return isWrite ? _config.SerialWrites : _config.SerialReads;
					} else if(addr >= 0x130 && addr <= 0x131) {
						return isWrite ? _config.InputWrites : _config.InputReads;
					} else {
						return isWrite ? _config.OtherRegisterWrites : _config.OtherRegisterReads;
					}
					break;
				case 5: return isWrite ? _config.PaletteWrites : _config.PaletteReads;
				case 6: return isWrite ? _config.VramWrites : _config.VramReads;
				case 7: return isWrite ? _config.OamWrites : _config.OamReads;
				default: return {};
			}

	}
}

void GbaEventManager::ConvertScanlineCycleToRowColumn(int32_t& x, int32_t& y)
{
	y *= 4;
	x *= 1;
}

uint32_t GbaEventManager::TakeEventSnapshot(bool forAutoRefresh)
{
	DebugBreakHelper breakHelper(_debugger);
	auto lock = _lock.AcquireSafe();

	uint16_t cycle = _ppu->GetCycle();
	uint16_t scanline = _ppu->GetScanline();

	if(scanline >= 160 || scanline == 0) {
		memcpy(_ppuBuffer, _ppu->GetScreenBuffer(), GbaConstants::PixelCount * sizeof(uint16_t));
	} else {
		uint32_t offset = GbaConstants::ScreenWidth * scanline;
		memcpy(_ppuBuffer, _ppu->GetScreenBuffer(), offset * sizeof(uint16_t));
		memcpy(_ppuBuffer + offset, _ppu->GetPreviousScreenBuffer() + offset, (GbaConstants::PixelCount - offset) * sizeof(uint16_t));
	}

	_snapshotCurrentFrame = _debugEvents;
	_snapshotPrevFrame = _prevDebugEvents;
	_snapshotScanline = scanline;
	_snapshotCycle = cycle;
	_forAutoRefresh = forAutoRefresh;
	_scanlineCount = GbaEventManager::ScreenHeight;
	return _scanlineCount;
}

FrameInfo GbaEventManager::GetDisplayBufferSize()
{
	FrameInfo size;
	size.Width = GbaEventManager::ScanlineWidth;
	size.Height = _scanlineCount * 4;
	return size;
}

void GbaEventManager::DrawScreen(uint32_t* buffer)
{
	uint16_t *src = _ppuBuffer;
	for(uint32_t y = 0, len = GbaConstants::ScreenHeight*4; y < len; y++) {
		for(uint32_t x = 0; x < 240*4; x+=4) {
			int srcOffset = (y >> 2) * 240 + (x >> 2);
			uint32_t color = ColorUtilities::Rgb555ToArgb(src[srcOffset]);
			int dstOffset = y * GbaEventManager::ScanlineWidth;
			buffer[dstOffset + x + 46] = color;
			buffer[dstOffset + x + 46 + 1] = color;
			buffer[dstOffset + x + 46 + 2] = color;
			buffer[dstOffset + x + 46 + 3] = color;
		}
	}
}
