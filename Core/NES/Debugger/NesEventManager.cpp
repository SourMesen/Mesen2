#include "pch.h"
#include "NES/Debugger/NesEventManager.h"
#include "NES/NesConsole.h"
#include "NES/BaseMapper.h"
#include "NES/NesCpu.h"
#include "NES/NesPpu.h"
#include "NES/NesDefaultVideoFilter.h"
#include "NES/NesTypes.h"
#include "NES/NesConstants.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/Debugger.h"
#include "Debugger/DebugBreakHelper.h"
#include "Debugger/BaseEventManager.h"

NesEventManager::NesEventManager(Debugger *debugger, NesConsole* console)
{
	_debugger = debugger;
	_console = console;
	_cpu = console->GetCpu();
	_mapper = console->GetMapper();
	_snapshotScanlineOffset = -1;

	NesDefaultVideoFilter::GetFullPalette(_palette, console->GetNesConfig(), console->GetPpu()->GetPpuModel());

	_ppuBuffer = new uint16_t[NesConstants::ScreenPixelCount];
	memset(_ppuBuffer, 0, NesConstants::ScreenPixelCount * sizeof(uint16_t));
}

NesEventManager::~NesEventManager()
{
	delete[] _ppuBuffer;
}

void NesEventManager::AddEvent(DebugEventType type, MemoryOperationInfo& operation, int32_t breakpointId)
{
	BaseNesPpu* ppu = _console->GetPpu();
	DebugEventInfo evt = {};
	evt.Type = type;
	evt.Operation = operation;
	evt.Scanline = (int16_t)ppu->GetCurrentScanline();
	evt.Cycle = (uint16_t)ppu->GetCurrentCycle();
	evt.BreakpointId = breakpointId;
	evt.ProgramCounter = _debugger->GetProgramCounter(CpuType::Nes, true);
	evt.DmaChannel = -1;
	evt.Flags = (uint32_t)EventFlags::ReadWriteOp;

	uint32_t addr = operation.Address;
	bool isWrite = operation.Type == MemoryOperationType::Write || operation.Type == MemoryOperationType::DmaWrite || operation.Type == MemoryOperationType::DummyWrite;
	if(isWrite && (addr & 0xE000) == 0x2000) {
		NesPpuState state;
		ppu->GetState(state);
		switch(addr & 0x07) {
			case 4:
				//OAM write
				evt.TargetMemory.Type = MemoryOperationType::Write;
				evt.TargetMemory.MemType = MemoryType::NesSpriteRam;
				evt.TargetMemory.Address = state.SpriteRamAddr;
				evt.TargetMemory.Value = operation.Value;
				evt.Flags |= (uint32_t)EventFlags::WithTargetMemory;
				break;

			case 5:
			case 6:
				//2005/2006 PPU register writes, mark as 2nd write when needed
				if(state.WriteToggle) {
					evt.Flags |= (uint32_t)EventFlags::RegSecondWrite;
				} else {
					evt.Flags |= (uint32_t)EventFlags::RegFirstWrite;
				}
				break;

			case 7:
				//VRAM write
				evt.TargetMemory.Type = MemoryOperationType::Write;
				evt.TargetMemory.MemType = MemoryType::NesPpuMemory;
				evt.TargetMemory.Address = state.BusAddress;
				evt.TargetMemory.Value = operation.Value;
				evt.Flags |= (uint32_t)EventFlags::WithTargetMemory;
				break;
		}
	}

	_debugEvents.push_back(evt);
}

void NesEventManager::AddEvent(DebugEventType type)
{
	MemoryOperationInfo op = {};
	if(type == DebugEventType::BgColorChange) {
		op.Address = _console->GetPpu()->GetCurrentBgColor();
	}

	BaseNesPpu* ppu = _console->GetPpu();
	DebugEventInfo evt = {};
	evt.Type = type;
	evt.Operation = op;
	evt.Scanline = (int16_t)ppu->GetCurrentScanline();
	evt.Cycle = (uint16_t)ppu->GetCurrentCycle();
	evt.BreakpointId = -1;
	evt.ProgramCounter = _cpu->GetState().PC;
	evt.DmaChannel = -1;
	_debugEvents.push_back(evt);
}

void NesEventManager::ClearFrameEvents()
{
	BaseEventManager::ClearFrameEvents();
	AddEvent(DebugEventType::BgColorChange);
}

DebugEventInfo NesEventManager::GetEvent(uint16_t y, uint16_t x)
{
	auto lock = _lock.AcquireSafe();

	int cycle = x / 2; //convert to cycle value
	int scanline = ((int)y / 2) - 1; //convert to scanline value
	
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

bool NesEventManager::ShowPreviousFrameEvents()
{
	return _config.ShowPreviousFrameEvents;
}

void NesEventManager::SetConfiguration(BaseEventViewerConfig& config)
{
	_config = (NesEventViewerConfig&)config;
}

EventViewerCategoryCfg NesEventManager::GetEventConfig(DebugEventInfo& evt)
{
	switch(evt.Type) {
		case DebugEventType::Irq: return _config.Irq;
		case DebugEventType::Nmi: return _config.Nmi;
		case DebugEventType::SpriteZeroHit: return _config.SpriteZeroHit;
		case DebugEventType::DmcDmaRead: return _config.DmcDmaReads;
		case DebugEventType::DmaRead: return _config.OtherDmaReads;
		case DebugEventType::Breakpoint: return _config.MarkedBreakpoints;
		case DebugEventType::Register:
			uint16_t addr = (uint16_t)evt.Operation.Address;
			bool isWrite = evt.Operation.Type == MemoryOperationType::Write || evt.Operation.Type == MemoryOperationType::DmaWrite || evt.Operation.Type == MemoryOperationType::DummyWrite;
			if(isWrite) {
				if(addr >= 0x2000 && addr <= 0x3FFF) {
					switch(addr & 0x200F) {
						case 0x2000: return _config.Ppu2000Write;
						case 0x2001: return _config.Ppu2001Write;
						case 0x2003: return _config.Ppu2003Write;
						case 0x2004: return _config.Ppu2004Write;
						case 0x2005: return _config.Ppu2005Write;
						case 0x2006: return _config.Ppu2006Write;
						case 0x2007: return _config.Ppu2007Write;
					}
				} else if(addr >= 0x4020 && _mapper->IsWriteRegister(addr)) {
					return _config.MapperRegisterWrites;
				} else if((addr >= 0x4000 && addr <= 0x4015) || addr == 0x4017 || addr == 0x401A) {
					return _config.ApuRegisterWrites;
				} else if(addr == 0x4016) {
					return _config.ControlRegisterWrites;
				}
			} else {
				if(addr >= 0x2000 && addr <= 0x3FFF) {
					switch(addr & 0x200F) {
						case 0x2002: return _config.Ppu2002Read;
						case 0x2004: return _config.Ppu2004Read;
						case 0x2007: return _config.Ppu2007Read;
					}
				} else if(addr >= 0x4020 && _mapper->IsReadRegister(addr)) {
					return _config.MapperRegisterReads;
				} else if((addr >= 0x4000 && addr <= 0x4015) || (addr >= 0x4018 && addr <= 0x401A)) {
					return _config.ApuRegisterReads;
				} else if(addr == 0x4016 || addr == 0x4017) {
					return _config.ControlRegisterReads;
				}
			}

			return {};
	}

	return {};
}

void NesEventManager::ConvertScanlineCycleToRowColumn(int32_t& x, int32_t& y)
{
	y = (y + 1) * 2;
	x = x * 2;
}

uint32_t NesEventManager::TakeEventSnapshot(bool forAutoRefresh)
{
	BaseNesPpu* ppu = _console->GetPpu();
	DebugBreakHelper breakHelper(_debugger);
	auto lock = _lock.AcquireSafe();

	uint16_t cycle = _console->GetPpu()->GetCurrentCycle();
	uint16_t scanline = ppu->GetCurrentScanline() + 1;

	if(scanline >= 240 || (scanline == 0 && cycle == 0)) {
		memcpy(_ppuBuffer, ppu->GetScreenBuffer(false, true), NesConstants::ScreenPixelCount * sizeof(uint16_t));
	} else {
		uint32_t offset = (NesConstants::ScreenWidth * scanline);
		memcpy(_ppuBuffer, ppu->GetScreenBuffer(false, true), offset * sizeof(uint16_t));
		memcpy(_ppuBuffer + offset, ppu->GetScreenBuffer(true) + offset, (NesConstants::ScreenPixelCount - offset) * sizeof(uint16_t));
	}

	_snapshotCurrentFrame = _debugEvents;
	_snapshotPrevFrame = _prevDebugEvents;
	_snapshotScanline = scanline;
	_snapshotCycle = cycle;
	_forAutoRefresh = forAutoRefresh;
	_scanlineCount = ppu->GetScanlineCount();
	return _scanlineCount;
}

FrameInfo NesEventManager::GetDisplayBufferSize()
{
	FrameInfo size;
	size.Width = NesConstants::CyclesPerLine * 2;
	size.Height = _scanlineCount * 2;
	return size;
}

void NesEventManager::DrawScreen(uint32_t *buffer)
{
	uint16_t *src = _ppuBuffer;
	for(uint32_t y = 0, len = NesConstants::ScreenHeight*2; y < len; y++) {
		int rowOffset = (y + 2) * NesConstants::CyclesPerLine * 2;

		for(uint32_t x = 0; x < NesConstants::ScreenWidth *2; x++) {
			int srcOffset = ((y >> 1) << 8) | (x >> 1);
			buffer[rowOffset + x + 1 * 2] = _palette[src[srcOffset]];
		}
	}

	if(_config.ShowNtscBorders) {
		DrawNtscBorders(buffer);
	}
}

void NesEventManager::DrawPixel(uint32_t *buffer, int32_t x, uint32_t y, uint32_t color)
{
	if(x < 0) {
		x += NesConstants::CyclesPerLine;
		y--;
	} else if(x >= NesConstants::CyclesPerLine) {
		x -= NesConstants::CyclesPerLine;
		y++;
	}

	buffer[y * NesConstants::CyclesPerLine * 4 + x * 2] = color;
	buffer[y * NesConstants::CyclesPerLine * 4 + x * 2 + 1] = color;
	buffer[y * NesConstants::CyclesPerLine * 4 + NesConstants::CyclesPerLine * 2 + x * 2] = color;
	buffer[y * NesConstants::CyclesPerLine * 4 + NesConstants::CyclesPerLine * 2 + x * 2 + 1] = color;
}

void NesEventManager::ProcessNtscBorderColorEvents(vector<DebugEventInfo>& events, vector<uint16_t>& bgColor, uint32_t& currentPos, uint16_t& currentColor)
{
	for(DebugEventInfo& evt : events) {
		if(evt.Type == DebugEventType::BgColorChange) {
			uint32_t pos = ((evt.Scanline + 1) * NesConstants::CyclesPerLine) + evt.Cycle;
			if(evt.Scanline >= 242) {
				break;
			}

			if(pos >= currentPos) {
				std::fill(bgColor.begin() + currentPos, bgColor.begin() + pos, currentColor);
				currentPos = pos;
				currentColor = evt.Operation.Address;
			}
		}
	}
}

void NesEventManager::DrawNtscBorders(uint32_t *buffer)
{
	//Generate array of bg color for all pixels on the screen
	uint32_t currentPos = 0;
	uint16_t currentColor = 0;
	vector<uint16_t> bgColor;
	bgColor.resize(NesConstants::CyclesPerLine * 243);

	ProcessNtscBorderColorEvents(_snapshotCurrentFrame, bgColor, currentPos, currentColor);
	
	if(!_forAutoRefresh && _snapshotScanline < 242) {
		uint32_t snapshotPos = (_snapshotScanline * NesConstants::CyclesPerLine) + _snapshotCycle;
		std::fill(bgColor.begin() + currentPos, bgColor.begin() + snapshotPos, currentColor);
		currentPos = snapshotPos;
		ProcessNtscBorderColorEvents(_snapshotPrevFrame, bgColor, currentPos, currentColor);
	}

	std::fill(bgColor.begin() + currentPos, bgColor.end(), currentColor);

	for(uint32_t y = 1; y < 241; y++) {
		//Pulse
		uint32_t basePos = y * NesConstants::CyclesPerLine;
		DrawPixel(buffer, -15, y, _palette[bgColor[basePos - 16] & 0x1F0]);

		//Left border
		for(int32_t x = 0; x < 15; x++) {
			DrawPixel(buffer, -x, y, _palette[bgColor[basePos - x]]);
		}

		//Right border
		for(int32_t x = 0; x < 11; x++) {
			DrawPixel(buffer, 257+x, y, _palette[bgColor[basePos + 257 + x]]);
		}
	}

	for(uint32_t y = 240; y < 242; y++) {
		//Bottom border
		uint32_t basePos = y * NesConstants::CyclesPerLine;
		DrawPixel(buffer, 326, y, _palette[bgColor[basePos + 326] & 0x1F0]);
		for(int32_t x = 0; x < 282; x++) {
			DrawPixel(buffer, 327 + x, y, _palette[bgColor[basePos + 327 + x]]);
		}
	}
}