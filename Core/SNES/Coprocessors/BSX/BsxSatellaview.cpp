#include "pch.h"
#include "SNES/Coprocessors/BSX/BsxSatellaview.h"
#include "SNES/SnesConsole.h"
#include "SNES/SnesMemoryManager.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Utilities/Serializer.h"

BsxSatellaview::BsxSatellaview(SnesConsole* console, IMemoryHandler* bBusHandler) : IMemoryHandler(MemoryType::SnesRegister)
{
	_console = console;
	_memoryManager = console->GetMemoryManager();
	_customDate = console->GetEmulator()->GetSettings()->GetSnesConfig().BsxCustomDate;
	_bBusHandler = bBusHandler;
	Reset();
}

void BsxSatellaview::Reset()
{
	_prevMasterClock = 0;
	_streamReg = 0;
	_extOutput = 0xFF;

	time_t resetDate;
	if(_customDate >= 0) {
		resetDate = (time_t)_customDate;
	} else {
		//Use the current date/time as the BS-X date/time
		time(&resetDate);
	}

	_stream[0].Reset(_console, resetDate);
	_stream[1].Reset(_console, resetDate);
}

uint8_t BsxSatellaview::Read(uint32_t addr)
{
	addr &= 0xFFFF;
	if(addr >= 0x2188 && addr <= 0x219F) {
		//Handle BS-X $2188-219F registers
		ProcessClocks();

		switch(addr) {
			case 0x2188: return _stream[0].GetChannel() & 0xFF;
			case 0x2189: return (_stream[0].GetChannel()) >> 8;
			case 0x218A: return _stream[0].GetPrefixCount();
			case 0x218B: return _stream[0].GetPrefix();
			case 0x218C: return _stream[0].GetData();
			case 0x218D: return _stream[0].GetStatus((_streamReg & 0x01) != 0);
			
			case 0x218E: return _stream[1].GetChannel() & 0xFF;
			case 0x218F: return (_stream[1].GetChannel()) >> 8;
			case 0x2190: return _stream[1].GetPrefixCount();
			case 0x2191: return _stream[1].GetPrefix();
			case 0x2192: return _stream[1].GetData();
			case 0x2193: return _stream[1].GetStatus((_streamReg & 0x01) != 0);

			case 0x2194: return _streamReg; //LED and Stream register
			case 0x2195: return 0; //Unknown
			case 0x2196: return 0x10; //Satellaview status
			case 0x2197: return _extOutput; //Soundlink / EXT output
			case 0x2198: return 0x80; //Serial IO (Serial number)
			case 0x2199: return 0x01; //Serial IO (???)
			case 0x219A: return 0x10; //Unknown
		}
	} 
	
	//Use regular B-bus handler for everything else
	return _bBusHandler->Read(addr);
}

void BsxSatellaview::Write(uint32_t addr, uint8_t value)
{
	addr &= 0xFFFF;
	if(addr >= 0x2188 && addr <= 0x219F) {
		//Handle BS-X register writes
		ProcessClocks();

		switch(addr) {
			case 0x2188: _stream[0].SetChannelLow(value); break;
			case 0x2189: _stream[0].SetChannelHigh(value); break;
			case 0x218B: _stream[0].SetPrefixLatch(value); break;
			case 0x218C: _stream[0].SetDataLatch(value); break;

			case 0x218E: _stream[1].SetChannelLow(value); break;
			case 0x218F: _stream[1].SetChannelHigh(value); break;
			case 0x2191: _stream[1].SetPrefixLatch(value); break;
			case 0x2192: _stream[1].SetDataLatch(value); break;

			case 0x2194: _streamReg = value; break;
			case 0x2197: _extOutput = value; break;
		}
	} else {
		//Regular B-bus handler
		_bBusHandler->Write(addr, value);
	}
}

void BsxSatellaview::ProcessClocks()
{
	if(_stream[0].NeedUpdate() || _stream[1].NeedUpdate()) {
		uint64_t gap = _memoryManager->GetMasterClock() - _prevMasterClock;
		uint64_t clocksPerFrame = _console->GetMasterClockRate() / 1000; //1000 frames/sec (224kbits/sec)

		while(gap >= clocksPerFrame) {
			bool needUpdate = _stream[0].FillQueues() || _stream[1].FillQueues();
			if(!needUpdate) {
				gap = 0;
				break;
			}
			gap -= clocksPerFrame;
		}

		_prevMasterClock = _memoryManager->GetMasterClock() - gap;
	} else {
		_prevMasterClock = _memoryManager->GetMasterClock();
	}
}

uint8_t BsxSatellaview::Peek(uint32_t addr)
{
	return 0;
}

void BsxSatellaview::PeekBlock(uint32_t addr, uint8_t* output)
{
	memset(output, 0, 0x1000);
}

AddressInfo BsxSatellaview::GetAbsoluteAddress(uint32_t address)
{
	return { -1, MemoryType::None };
}

void BsxSatellaview::Serialize(Serializer& s)
{
	SV(_extOutput); SV(_streamReg); SV(_customDate); SV(_prevMasterClock);
	SV(_stream[0]);
	SV(_stream[1]);
}
