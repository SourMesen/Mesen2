#include "pch.h"
#include "SMS/Debugger/SmsEventManager.h"
#include "SMS/SmsCpu.h"
#include "SMS/SmsVdp.h"
#include "SMS/SmsConsole.h"
#include "Shared/ColorUtilities.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/Debugger.h"
#include "Debugger/DebugBreakHelper.h"
#include "Debugger/BaseEventManager.h"

SmsEventManager::SmsEventManager(Debugger* debugger, SmsConsole* console, SmsCpu* cpu, SmsVdp* vdp)
{
	_debugger = debugger;
	_console = console;
	_cpu = cpu;
	_vdp = vdp;

	_ppuBuffer = new uint16_t[256*240];
	memset(_ppuBuffer, 0, 256*240 * sizeof(uint16_t));
}

SmsEventManager::~SmsEventManager()
{
	delete[] _ppuBuffer;
}

void SmsEventManager::AddEvent(DebugEventType type, MemoryOperationInfo& operation, int32_t breakpointId)
{
	DebugEventInfo evt = {};
	evt.Type = type;
	evt.Flags = (uint32_t)EventFlags::ReadWriteOp;
	evt.Operation = operation;
	evt.Scanline = _vdp->GetScanline();
	evt.Cycle = _vdp->GetCycle();
	evt.BreakpointId = breakpointId;
	evt.DmaChannel = -1;

	if(evt.Operation.Type == MemoryOperationType::Write) {
		switch(evt.Operation.Address & 0xC1) {
			case 0x80:
				if(_vdp->GetState().CodeReg == 3) {
					evt.Flags |= (uint32_t)EventFlags::SmsVdpPaletteWrite | (uint32_t)EventFlags::WithTargetMemory;
					evt.TargetMemory.MemType = MemoryType::SmsPaletteRam;
					evt.TargetMemory.Type = operation.Type;
					evt.TargetMemory.Value = operation.Value;

					if(_console->GetModel() == SmsModel::GameGear) {
						evt.TargetMemory.Address = _vdp->GetState().AddressReg & 0x3F;
					} else {
						evt.TargetMemory.Address = _vdp->GetState().AddressReg & 0x1F;
					}
				} else {
					evt.Flags |= (uint32_t)EventFlags::WithTargetMemory;
					evt.TargetMemory.MemType = MemoryType::SmsVideoRam;
					evt.TargetMemory.Value = operation.Value;
					evt.TargetMemory.Type = operation.Type;
					evt.TargetMemory.Address = _vdp->GetState().AddressReg;
				}
				break;

			case 0x81:
				if(_vdp->GetState().ControlPortMsbToggle) {
					evt.Flags |= (uint32_t)EventFlags::RegSecondWrite;
					if((evt.Operation.Value >> 6) == 2) {
						//Value written to the register
						evt.RegisterId = _vdp->GetState().AddressReg & 0xFF;
					}
				} else {
					evt.Flags |= (uint32_t)EventFlags::RegFirstWrite;
				}
				break;
		}
	}

	evt.ProgramCounter = _debugger->GetProgramCounter(CpuType::Sms, true);
	_debugEvents.push_back(evt);
}

void SmsEventManager::AddEvent(DebugEventType type)
{
	DebugEventInfo evt = {};
	evt.Type = type;
	evt.Scanline = _vdp->GetScanline();
	evt.Cycle = _vdp->GetCycle();
	evt.BreakpointId = -1;
	evt.DmaChannel = -1;
	evt.ProgramCounter = _cpu->GetState().PC;
	_debugEvents.push_back(evt);
}

DebugEventInfo SmsEventManager::GetEvent(uint16_t y, uint16_t x)
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

bool SmsEventManager::ShowPreviousFrameEvents()
{
	return _config.ShowPreviousFrameEvents;
}

void SmsEventManager::SetConfiguration(BaseEventViewerConfig& config)
{
	_config = (SmsEventViewerConfig&)config;
}

EventViewerCategoryCfg SmsEventManager::GetEventConfig(DebugEventInfo& evt)
{
	switch(evt.Type) {
		default: return {};
		case DebugEventType::Breakpoint: return _config.MarkedBreakpoints;
		case DebugEventType::Irq: return _config.Irq;
		case DebugEventType::Register:
			if(_console->GetModel() == SmsModel::GameGear && evt.Operation.Address <= 6) {
				return evt.Operation.Type == MemoryOperationType::Read ? _config.GameGearPortRead : _config.GameGearPortWrite;
			} else if(evt.Operation.Type == MemoryOperationType::Read) {
				switch(evt.Operation.Address & 0xC1) {
					case 0x40: return _config.VdpVCounterRead;
					case 0x41: return _config.VdpHCounterRead;
					case 0x80: return _config.VdpVramRead;
					case 0x81: return _config.VdpControlPortRead;
					case 0xC0: case 0xC1: return _config.IoRead;
					default: return {};
				}
			} else {
				switch(evt.Operation.Address & 0xC1) {
					case 0x00: return _config.MemoryControlWrite;
					case 0x01: return _config.IoWrite;
					case 0x40: case 0x41: return _config.PsgWrite;
					case 0x80: return (evt.Flags & (uint32_t)EventFlags::SmsVdpPaletteWrite) ? _config.VdpPaletteWrite : _config.VdpVramWrite;
					case 0x81: return _config.VdpControlPortWrite;
					default: return {};
				}
			}
	}
}

void SmsEventManager::ConvertScanlineCycleToRowColumn(int32_t& x, int32_t& y)
{
	y *= 2;
	x *= 2;
}

uint32_t SmsEventManager::TakeEventSnapshot(bool forAutoRefresh)
{
	DebugBreakHelper breakHelper(_debugger);
	auto lock = _lock.AcquireSafe();

	uint16_t cycle = _vdp->GetCycle();
	uint16_t scanline = _vdp->GetScanline();

	if(scanline >= _vdp->GetState().VisibleScanlineCount || scanline == 0) {
		memcpy(_ppuBuffer, _vdp->GetScreenBuffer(false), 256 * 240 * sizeof(uint16_t));
	} else {
		uint32_t offset = 256 * scanline;
		memcpy(_ppuBuffer, _vdp->GetScreenBuffer(false), offset * sizeof(uint16_t));
		memcpy(_ppuBuffer + offset, _vdp->GetScreenBuffer(true) + offset, (256 * 240 - offset) * sizeof(uint16_t));
	}

	_snapshotCurrentFrame = _debugEvents;
	_snapshotPrevFrame = _prevDebugEvents;
	_snapshotScanline = scanline;
	_snapshotCycle = cycle;
	_forAutoRefresh = forAutoRefresh;
	_visibleScanlineCount = _vdp->GetState().VisibleScanlineCount;
	_scanlineCount = _vdp->GetScanlineCount();
	return _scanlineCount;
}

FrameInfo SmsEventManager::GetDisplayBufferSize()
{
	FrameInfo size;
	size.Width = SmsEventManager::ScanlineWidth;
	size.Height = _scanlineCount * 2;
	return size;
}

void SmsEventManager::DrawScreen(uint32_t* buffer)
{
	uint16_t *src = _ppuBuffer;
	for(uint32_t y = 0, len = _visibleScanlineCount * 2; y < len; y++) {
		for(uint32_t x = 0; x < 256*2; x++) {
			int srcOffset = (y >> 1) * 256 + (x >> 1);
			buffer[y*SmsEventManager::ScanlineWidth + x + 26] = ColorUtilities::Rgb555ToArgb(src[srcOffset]);
		}
	}
}
