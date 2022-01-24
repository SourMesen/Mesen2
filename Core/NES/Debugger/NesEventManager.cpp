#include "stdafx.h"
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
	_cpu = console->GetCpu();
	_ppu = console->GetPpu();
	_mapper = console->GetMapper();

	NesDefaultVideoFilter::GetFullPalette(_palette, console->GetNesConfig(), _ppu->GetPpuModel());

	_ppuBuffer = new uint16_t[NesConstants::ScreenPixelCount];
	memset(_ppuBuffer, 0, NesConstants::ScreenPixelCount * sizeof(uint16_t));
}

NesEventManager::~NesEventManager()
{
	delete[] _ppuBuffer;
}

void NesEventManager::AddEvent(DebugEventType type, MemoryOperationInfo& operation, int32_t breakpointId)
{
	DebugEventInfo evt = {};
	evt.Type = type;
	evt.Operation = operation;
	evt.Scanline = (int16_t)_ppu->GetCurrentScanline();
	evt.Cycle = (uint16_t)_ppu->GetCurrentCycle();
	evt.BreakpointId = breakpointId;
	evt.ProgramCounter = _cpu->GetState().PC;

	uint32_t addr = operation.Address;
	if(operation.Type == MemoryOperationType::Write && (addr & 0xE000) == 0x2000) {
		NesPpuState state;
		_ppu->GetState(state);
		switch(addr & 0x07) {
			case 4:
				//OAM write
				evt.TargetMemory.Type = MemoryOperationType::Write;
				evt.TargetMemory.MemType = MemoryType::NesSpriteRam;
				evt.TargetMemory.Address = state.SpriteRamAddr;
				evt.TargetMemory.Value = operation.Value;
				evt.Flags = (uint32_t)EventFlags::WithTargetMemory;
				break;

			case 5:
			case 6:
				//2005/2006 PPU register writes, mark as 2nd write when needed
				if(state.WriteToggle) {
					evt.Flags = (uint32_t)EventFlags::NesPpuSecondWrite;
				}
				break;

			case 7:
				//VRAM write
				evt.TargetMemory.Type = MemoryOperationType::Write;
				evt.TargetMemory.MemType = MemoryType::NesPpuMemory;
				evt.TargetMemory.Address = state.BusAddress;
				evt.TargetMemory.Value = operation.Value;
				evt.Flags = (uint32_t)EventFlags::WithTargetMemory;
				break;
		}
	}

	_debugEvents.push_back(evt);
}

void NesEventManager::AddEvent(DebugEventType type)
{
	MemoryOperationInfo op = {};
	if(type == DebugEventType::BgColorChange) {
		op.Address = _ppu->GetCurrentBgColor();
	}
	AddEvent(type, op, -1);
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
		case DebugEventType::Breakpoint: return _config.MarkedBreakpoints;
		case DebugEventType::Register:
			uint16_t addr = (uint16_t)evt.Operation.Address;
			if(evt.Operation.Type == MemoryOperationType::Write) {
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
				} else if(addr >= 0x4018 && _mapper->IsWriteRegister(addr)) {
					return _config.MapperRegisterWrites;
				} else if(addr >= 0x4000 && addr <= 0x4015 || addr == 0x4017) {
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
				} else if(addr >= 0x4018 && _mapper->IsReadRegister(addr)) {
					return _config.MapperRegisterWrites;
				} else if(addr >= 0x4000 && addr <= 0x4015) {
					return _config.ApuRegisterReads;
				} else if(addr == 0x4016 || addr == 0x4017) {
					return _config.ControlRegisterReads;
				}
			}

			return {};
	}

	return {};
}

void NesEventManager::DrawEvent(DebugEventInfo &evt, bool drawBackground, uint32_t *buffer)
{
	EventViewerCategoryCfg eventCfg = GetEventConfig(evt);
	uint32_t color = eventCfg.Color;
	
	if(!eventCfg.Visible) {
		return;
	}

	uint32_t y = std::min<uint32_t>((evt.Scanline + 1) * 2, _scanlineCount * 2);
	uint32_t x = evt.Cycle * 2;
	DrawDot(x, y, color, drawBackground, buffer);
}

uint32_t NesEventManager::TakeEventSnapshot()
{
	DebugBreakHelper breakHelper(_debugger);
	auto lock = _lock.AcquireSafe();

	uint16_t cycle = _ppu->GetCurrentCycle();
	uint16_t scanline = _ppu->GetCurrentScanline() + 1;

	if(scanline >= 240 || (scanline == 0 && cycle == 0)) {
		memcpy(_ppuBuffer, _ppu->GetScreenBuffer(false), NesConstants::ScreenPixelCount * sizeof(uint16_t));
	} else {
		uint32_t offset = (NesConstants::ScreenWidth * scanline);
		memcpy(_ppuBuffer, _ppu->GetScreenBuffer(false), offset * sizeof(uint16_t));
		memcpy(_ppuBuffer + offset, _ppu->GetScreenBuffer(true) + offset, (NesConstants::ScreenPixelCount - offset) * sizeof(uint16_t));
	}

	_snapshotCurrentFrame = _debugEvents;
	_snapshotPrevFrame = _prevDebugEvents;
	_snapshotScanline = scanline;
	_snapshotCycle = cycle;
	_scanlineCount = _ppu->GetScanlineCount();
	return _scanlineCount;
}

FrameInfo NesEventManager::GetDisplayBufferSize()
{
	FrameInfo size;
	size.Width = NesConstants::CyclesPerLine * 2;
	size.Height = _scanlineCount * 2;
	return size;
}

void NesEventManager::GetDisplayBuffer(uint32_t *buffer, uint32_t bufferSize)
{
	auto lock = _lock.AcquireSafe();

	FrameInfo size = GetDisplayBufferSize();
	if(_snapshotScanline < 0 || bufferSize < size.Width * size.Height * sizeof(uint32_t)) {
		return;
	}

	for(uint32_t i = 0; i < size.Width*size.Height; i++) {
		buffer[i] = 0xFF555555;
	}

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

	constexpr uint32_t currentScanlineColor = 0xFFFFFF55;
	uint32_t scanlineOffset = _snapshotScanline * 2 * NesConstants::CyclesPerLine * 2;
	for(int i = 0; i < NesConstants::CyclesPerLine * 2; i++) {
		if(_snapshotScanline != 0) {
			buffer[scanlineOffset + i] = currentScanlineColor;
			buffer[scanlineOffset + NesConstants::CyclesPerLine * 2 + i] = currentScanlineColor;
		}
	}

	FilterEvents();
	for(DebugEventInfo &evt : _sentEvents) {
		DrawEvent(evt, true, buffer);
	}
	for(DebugEventInfo &evt : _sentEvents) {
		DrawEvent(evt, false, buffer);
	}

	//Draw dot over current pixel
	if(_snapshotScanline != 0 && _snapshotCycle != 0) {
		DrawDot(_snapshotCycle * 2, _snapshotScanline * 2, 0xFF990099, true, buffer);
		DrawDot(_snapshotCycle * 2, _snapshotScanline * 2, 0xFFFF00FF, false, buffer);
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

void NesEventManager::DrawNtscBorders(uint32_t *buffer)
{
	//Generate array of bg color for all pixels on the screen
	uint32_t currentPos = 0;
	uint16_t currentColor = 0;
	vector<uint16_t> bgColor;
	bgColor.resize(NesConstants::CyclesPerLine * 243);

	//TODO use bg color changes from previous frame when needed
	for(DebugEventInfo &evt : _snapshotCurrentFrame) {
		if(evt.Type == DebugEventType::BgColorChange) {
			uint32_t pos = ((evt.Scanline + 1) * NesConstants::CyclesPerLine) + evt.Cycle;
			if(pos >= currentPos && evt.Scanline < 242) {
				std::fill(bgColor.begin() + currentPos, bgColor.begin() + pos, currentColor);
				currentColor = evt.Operation.Address;
				currentPos = pos;
			}
		}
	}
	std::fill(bgColor.begin() + currentPos, bgColor.end(), currentColor);

	for(uint32_t y = 1; y < 241; y++) {
		//Pulse
		uint32_t basePos = y * NesConstants::CyclesPerLine;
		DrawPixel(buffer, -15, y, _palette[bgColor[basePos - 16] & 0x30]);

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
		DrawPixel(buffer, 326, y, _palette[bgColor[basePos + 326] & 0x30]);
		for(int32_t x = 0; x < 282; x++) {
			DrawPixel(buffer, 327 + x, y, _palette[bgColor[basePos + 327 + x]]);
		}
	}
}