#include "pch.h"
#include "SNES/Coprocessors/CX4/Cx4.h"
#include "SNES/SnesConsole.h"
#include "SNES/SnesCpu.h"
#include "SNES/SnesMemoryManager.h"
#include "SNES/BaseCartridge.h"
#include "SNES/MemoryMappings.h"
#include "SNES/RamHandler.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/Serializer.h"

//TODO: Proper open bus behavior (and return 0s for missing save ram, too)
//TODO: CPU shouldn't have access to PRG ROM while the CX4 is loading from PRG ROM
//TODO: Timings are apparently not perfect (desync in MMX2 intro)

Cx4::Cx4(SnesConsole* console)
{
	_emu = console->GetEmulator();
	_console = console;
	_memoryManager = console->GetMemoryManager();
	_cpu = console->GetCpu();
	
	_emu->RegisterMemory(MemoryType::Cx4DataRam, _dataRam, Cx4::DataRamSize);
	_console->InitializeRam(_dataRam, Cx4::DataRamSize);
	
	auto &prgRomHandlers = console->GetCartridge()->GetPrgRomHandlers();
	auto &saveRamHandlers = console->GetCartridge()->GetSaveRamHandlers();
	MemoryMappings* cpuMappings = _memoryManager->GetMemoryMappings();

	//PRG ROM
	uint8_t bankCount = _emu->GetSettings()->GetSnesConfig().EnableStrictBoardMappings ? 0x3F : 0x7F;
	cpuMappings->RegisterHandler(0x00, std::min<uint8_t>(0x7D, bankCount), 0x8000, 0xFFFF, prgRomHandlers);
	cpuMappings->RegisterHandler(0x80, 0x80 + bankCount, 0x8000, 0xFFFF, prgRomHandlers);
	_mappings.RegisterHandler(0x00, bankCount, 0x8000, 0xFFFF, prgRomHandlers);
	_mappings.RegisterHandler(0x80, 0x80 + bankCount, 0x8000, 0xFFFF, prgRomHandlers);

	//Save RAM
	cpuMappings->RegisterHandler(0x70, 0x7D, 0x0000, 0x7FFF, saveRamHandlers);
	cpuMappings->RegisterHandler(0xF0, 0xFF, 0x0000, 0x7FFF, saveRamHandlers);
	_mappings.RegisterHandler(0x70, 0x7D, 0x0000, 0x7FFF, saveRamHandlers);
	_mappings.RegisterHandler(0xF0, 0xFF, 0x0000, 0x7FFF, saveRamHandlers);

	//Registers
	cpuMappings->RegisterHandler(0x00, 0x3F, 0x6000, 0x7FFF, this);
	cpuMappings->RegisterHandler(0x80, 0xBF, 0x6000, 0x7FFF, this);
	_mappings.RegisterHandler(0x00, 0x3F, 0x6000, 0x7FFF, this);
	_mappings.RegisterHandler(0x80, 0xBF, 0x6000, 0x7FFF, this);

	_clockRatio = (double)20000000 / console->GetMasterClockRate();
	Reset();
}

void Cx4::Reset()
{
	_state = {};
	_state.Stopped = true;
	_state.SingleRom = true;
	_state.RomAccessDelay = 3;
	_state.RamAccessDelay = 3;
}

void Cx4::Run()
{
	uint64_t targetCycle = (uint64_t)(_memoryManager->GetMasterClock() * _clockRatio);

	while(_state.CycleCount < targetCycle) {
		if(_state.Locked) {
			Step(1);
		} else if(_state.Suspend.Enabled) {
			if(_state.Suspend.Duration == 0) {
				Step(1);
			} else {
				Step(1);
				_state.Suspend.Duration--;
				if(_state.Suspend.Duration == 0) {
					_state.Suspend.Enabled = false;
				}
			}
		} else if(_state.Cache.Enabled) {
			ProcessCache(targetCycle);
		} else if(_state.Dma.Enabled) {
			ProcessDma(targetCycle);
		} else if(_state.Stopped) {
			Step(targetCycle - _state.CycleCount);
		} else if(!ProcessCache(targetCycle)) {
			if(!_state.Cache.Enabled) {
				//Cache operation required, but both caches are locked, stop
				Stop();
			}
		} else {
			_emu->ProcessInstruction<CpuType::Cx4>();

			uint16_t opCode = _prgRam[_state.Cache.Page][_state.PC];
			_state.PC++;
			
			if(_state.PC == 0) {
				//If execution reached the end of the page, start loading the next page
				//This must be done BEFORE running the instruction (otherwise a jump/branch to address 0 will trigger this)
				SwitchCachePage();
			}

			Exec(opCode);
		}
	}
}

void Cx4::Step(uint64_t cycles)
{
	if(_state.Bus.Enabled) {
		if(_state.Bus.DelayCycles > cycles) {
			_state.Bus.DelayCycles -= (uint8_t)cycles;
		} else {
			_state.Bus.Enabled = false;
			_state.Bus.DelayCycles = 0;

			if(_state.Bus.Reading) {
				_state.MemoryDataReg = ReadCx4(_state.Bus.Address);
				_state.Bus.Reading = false;
			}

			if(_state.Bus.Writing) {
				WriteCx4(_state.Bus.Address, _state.MemoryDataReg);
				_state.Bus.Writing = false;
			}
		}
	}

	_state.CycleCount += cycles;
}

void Cx4::SwitchCachePage()
{
	if(_state.Cache.Page == 1) {
		Stop();
		return;
	}

	_state.Cache.Page = 1;
	if(_state.Cache.Lock[1]) {
		Stop();
		return;
	} 

	_state.PB = _state.P;

	uint64_t targetCycle = (uint64_t)(_memoryManager->GetMasterClock() * _clockRatio);
	if(!ProcessCache(targetCycle) && !_state.Cache.Enabled) {
		Stop();
	}
}

bool Cx4::ProcessCache(uint64_t targetCycle)
{
	uint32_t address = (_state.Cache.Base + (_state.PB << 9)) & 0xFFFFFF;

	if(_state.Cache.Pos == 0) {
		if(_state.Cache.Address[_state.Cache.Page] == address) {
			//Current cache page matches the needed address, keep using it
			_state.Cache.Enabled = false;
			return true;
		}

		//Check if the other page matches
		_state.Cache.Page ^= 1;

		if(_state.Cache.Address[_state.Cache.Page] == address) {
			//The other cache page matches, use it
			_state.Cache.Enabled = false;
			return true;
		}

		if(_state.Cache.Lock[_state.Cache.Page]) {
			//If it's locked, use the other page
			_state.Cache.Page ^= 1;
		}

		if(_state.Cache.Lock[_state.Cache.Page]) {
			//The both pages are locked, and the cache is invalid, give up.
			_state.Cache.Enabled = false;
			return false;
		}
	
		_state.Cache.Enabled = true;
	}

	//Populate the cache
	while(_state.Cache.Pos < 256) {
		uint8_t lsb = ReadCx4(address + (_state.Cache.Pos * 2));
		Step(GetAccessDelay(address + (_state.Cache.Pos * 2)));

		uint8_t msb = ReadCx4(address + (_state.Cache.Pos * 2) + 1);
		Step(GetAccessDelay(address + (_state.Cache.Pos * 2) + 1));

		_prgRam[_state.Cache.Page][_state.Cache.Pos] = (msb << 8) | lsb;
		_state.Cache.Pos++;

		if(_state.CycleCount > targetCycle) {
			break;
		}
	}

	if(_state.Cache.Pos >= 256) {
		_state.Cache.Address[_state.Cache.Page] = address;
		_state.Cache.Pos = 0;
		_state.Cache.Enabled = false;
		return true;
	}

	//Cache loading is not finished
	return false;
}

void Cx4::ProcessDma(uint64_t targetCycle)
{
	while(_state.Dma.Pos < _state.Dma.Length) {
		uint32_t src = (_state.Dma.Source + _state.Dma.Pos) & 0xFFFFFF;
		uint32_t dest = (_state.Dma.Dest + _state.Dma.Pos) & 0xFFFFFF;

		IMemoryHandler *srcHandler = _mappings.GetHandler(src);
		IMemoryHandler *destHandler = _mappings.GetHandler(dest);
		if(!srcHandler || !destHandler || srcHandler->GetMemoryType() == destHandler->GetMemoryType() || destHandler->GetMemoryType() == MemoryType::SnesPrgRom) {
			//Invalid DMA, the chip is locked until it gets restarted by a write to $7F53
			_state.Locked = true;
			_state.Dma.Pos = 0;
			_state.Dma.Enabled = false;
			return;
		}

		Step(GetAccessDelay(src));
		uint8_t value = ReadCx4(src);

		Step(GetAccessDelay(dest));
		WriteCx4(dest, value);
		_state.Dma.Pos++;

		if(_state.CycleCount > targetCycle) {
			break;
		}
	}

	if(_state.Dma.Pos >= _state.Dma.Length) {
		_state.Dma.Pos = 0;
		_state.Dma.Enabled = false;
	}
}

uint8_t Cx4::GetAccessDelay(uint32_t addr)
{
	IMemoryHandler* handler = _mappings.GetHandler(addr);
	if(handler->GetMemoryType() == MemoryType::SnesPrgRom) {
		return 1 + _state.RomAccessDelay;
	} else if(handler->GetMemoryType() == MemoryType::SnesSaveRam) {
		return 1 + _state.RamAccessDelay;
	}

	return 1;
}

uint8_t Cx4::ReadCx4(uint32_t addr)
{
	IMemoryHandler* handler = _mappings.GetHandler(addr);
	if(handler) {
		uint8_t value = handler->Read(addr);
		_emu->ProcessMemoryRead<CpuType::Cx4>(addr, value, MemoryOperationType::Read);
		return value;
	}
	return 0;
}

void Cx4::WriteCx4(uint32_t addr, uint8_t value)
{
	if(_emu->ProcessMemoryWrite<CpuType::Cx4>(addr, value, MemoryOperationType::Write)) {
		IMemoryHandler* handler = _mappings.GetHandler(addr);
		if(handler) {
			handler->Write(addr, value);
		}
	}
}

uint8_t Cx4::Read(uint32_t addr)
{
	addr = 0x7000 | (addr & 0xFFF);
	if(addr <= 0x7BFF) {
		return _dataRam[addr & 0xFFF];
	} else if(addr >= 0x7F60 && addr <= 0x7F7F) {
		return _state.Vectors[addr & 0x1F];
	} else if((addr >= 0x7F80 && addr <= 0x7FAF) || (addr >= 0x7FC0 && addr <= 0x7FEF)) {
		addr &= 0x3F;
		uint32_t &reg = _state.Regs[addr / 3];
		switch(addr % 3) {
			case 0: return reg;
			case 1: return reg >> 8;
			case 2: return reg >> 16;
		}
	} else if(addr >= 0x7F53 && addr <= 0x7F5F) {
		return (uint8_t)_state.Suspend.Enabled | ((uint8_t)_state.IrqFlag << 1) | ((uint8_t)IsRunning() << 6) | ((uint8_t)IsBusy() << 7);
	}

	switch(addr) {
		case 0x7F40: return _state.Dma.Source;
		case 0x7F41: return _state.Dma.Source >> 8;
		case 0x7F42: return _state.Dma.Source >> 16;
		case 0x7F43: return (uint8_t)_state.Dma.Length;
		case 0x7F44: return _state.Dma.Length >> 8;
		case 0x7F45: return _state.Dma.Dest;
		case 0x7F46: return _state.Dma.Dest >> 8;
		case 0x7F47: return _state.Dma.Dest >> 16;
		case 0x7F48: return _state.Cache.Page;
		case 0x7F49: return _state.Cache.Base;
		case 0x7F4A: return _state.Cache.Base >> 8;
		case 0x7F4B: return _state.Cache.Base >> 16;
		case 0x7F4C: return (uint8_t)_state.Cache.Lock[0] | ((uint8_t)_state.Cache.Lock[1] << 1);
		case 0x7F4D: return (uint8_t)_state.Cache.ProgramBank;
		case 0x7F4E: return _state.Cache.ProgramBank >> 8;
		case 0x7F4F: return _state.Cache.ProgramCounter;
		case 0x7F50: return _state.RamAccessDelay | (_state.RomAccessDelay << 4);
		case 0x7F51: return _state.IrqDisabled;
		case 0x7F52: return _state.SingleRom;
	}

	return 0;
}

void Cx4::Write(uint32_t addr, uint8_t value)
{
	addr = 0x7000 | (addr & 0xFFF);

	if(addr <= 0x7BFF) {
		_dataRam[addr & 0xFFF] = value;
		return;
	} 
	
	if(addr >= 0x7F60 && addr <= 0x7F7F) {
		_state.Vectors[addr & 0x1F] = value;
	} else if((addr >= 0x7F80 && addr <= 0x7FAF) || (addr >= 0x7FC0 && addr <= 0x7FEF)) {
		addr &= 0x3F;
		uint32_t &reg = _state.Regs[addr / 3];
		switch(addr % 3) {
			case 0: reg = (reg & 0xFFFF00) | value; break;
			case 1: reg = (reg & 0xFF00FF) | (value << 8); break;
			case 2: reg = (reg & 0x00FFFF) | (value << 16); break;
		}
	} else if(addr >= 0x7F55 && addr <= 0x7F5C) {
		_state.Suspend.Enabled = true;
		_state.Suspend.Duration = (addr - 0x7F55) * 32;
	} else {
		switch(addr) {
			case 0x7F40: _state.Dma.Source = (_state.Dma.Source & 0xFFFF00) | value; break;
			case 0x7F41: _state.Dma.Source = (_state.Dma.Source & 0xFF00FF) | (value << 8); break;
			case 0x7F42: _state.Dma.Source = (_state.Dma.Source & 0x00FFFF) | (value << 16); break;
			case 0x7F43: _state.Dma.Length = (_state.Dma.Length & 0xFF00) | value; break;
			case 0x7F44: _state.Dma.Length = (_state.Dma.Length & 0x00FF) | (value << 8); break;
			case 0x7F45: _state.Dma.Dest = (_state.Dma.Dest & 0xFFFF00) | value; break;
			case 0x7F46: _state.Dma.Dest = (_state.Dma.Dest & 0xFF00FF) | (value << 8); break;
			case 0x7F47:
				_state.Dma.Dest = (_state.Dma.Dest & 0x00FFFF) | (value << 16);
				if(_state.Stopped) {
					_state.Dma.Enabled = true;
				}
				break;

			case 0x7F48:
				_state.Cache.Page = value & 0x01;
				if(_state.Stopped) {
					_state.Cache.Enabled = true;
				}
				break;

			case 0x7F49: _state.Cache.Base = (_state.Cache.Base & 0xFFFF00) | value; break;
			case 0x7F4A: _state.Cache.Base = (_state.Cache.Base & 0xFF00FF) | (value << 8); break;
			case 0x7F4B: _state.Cache.Base = (_state.Cache.Base & 0x00FFFF) | (value << 16); break;

			case 0x7F4C:
				_state.Cache.Lock[0] = (value & 0x01) != 0;
				_state.Cache.Lock[1] = (value & 0x02) != 0;
				break;

			case 0x7F4D: _state.Cache.ProgramBank = (_state.Cache.ProgramBank & 0xFF00) | value; break;
			case 0x7F4E: _state.Cache.ProgramBank = (_state.Cache.ProgramBank & 0x00FF) | ((value & 0x7F) << 8); break;

			case 0x7F4F:
				_state.Cache.ProgramCounter = value;
				if(_state.Stopped) {
					_state.Stopped = false;
					_state.PB = _state.Cache.ProgramBank;
					_state.PC = _state.Cache.ProgramCounter;
				}
				break;

			case 0x7F50:
				_state.RamAccessDelay = value & 0x07;
				_state.RomAccessDelay = (value >> 4) & 0x07;
				break;

			case 0x7F51:
				_state.IrqDisabled = value & 0x01;
				if(_state.IrqDisabled) {
					_state.IrqFlag = true;
					_cpu->ClearIrqSource(SnesIrqSource::Coprocessor);
				}
				break;

			case 0x7F52: _state.SingleRom = (value & 0x01) != 0; break;

			case 0x7F53:
				_state.Locked = false;
				_state.Stopped = true;
				break;

			case 0x7F5D: _state.Suspend.Enabled = false; break;

			case 0x7F5E: 
				//Clear IRQ flag in CX4, but keeps IRQ signal high
				_state.IrqFlag = false; 
				break;
		}
	}
}

bool Cx4::IsRunning()
{
	return IsBusy() || !_state.Stopped;
}

bool Cx4::IsBusy()
{
	return _state.Cache.Enabled || _state.Dma.Enabled || _state.Bus.DelayCycles > 0;
}

void Cx4::Serialize(Serializer &s)
{
	SV(_state.CycleCount); SV(_state.PB); SV(_state.PC); SV(_state.A); SV(_state.P); SV(_state.SP); SV(_state.Mult); SV(_state.RomBuffer);
	SV(_state.RamBuffer[0]); SV(_state.RamBuffer[1]); SV(_state.RamBuffer[2]); SV(_state.MemoryDataReg); SV(_state.MemoryAddressReg);
	SV(_state.DataPointerReg); SV(_state.Negative); SV(_state.Zero); SV(_state.Carry); SV(_state.Overflow); SV(_state.IrqFlag); SV(_state.Stopped);
	SV(_state.Locked); SV(_state.IrqDisabled); SV(_state.SingleRom); SV(_state.RamAccessDelay); SV(_state.RomAccessDelay); SV(_state.Bus.Address);
	SV(_state.Bus.DelayCycles); SV(_state.Bus.Enabled); SV(_state.Bus.Reading); SV(_state.Bus.Writing); SV(_state.Dma.Dest); SV(_state.Dma.Enabled);
	SV(_state.Dma.Length); SV(_state.Dma.Source); SV(_state.Dma.Pos); SV(_state.Suspend.Duration); SV(_state.Suspend.Enabled); SV(_state.Cache.Enabled);
	SV(_state.Cache.Lock[0]); SV(_state.Cache.Lock[1]); SV(_state.Cache.Address[0]); SV(_state.Cache.Address[1]); SV(_state.Cache.Base);
	SV(_state.Cache.Page); SV(_state.Cache.ProgramBank); SV(_state.Cache.ProgramCounter); SV(_state.Cache.Pos);
	
	SVArray(_state.Stack, 8);
	SVArray(_state.Regs, 16);
	SVArray(_state.Vectors, 0x20);
	SVArray(_prgRam[0], 256);
	SVArray(_prgRam[1], 256);
	SVArray(_dataRam, Cx4::DataRamSize);
}

uint8_t Cx4::Peek(uint32_t addr)
{
	return 0;
}

void Cx4::PeekBlock(uint32_t addr, uint8_t* output)
{
	memset(output, 0, 0x1000);
}

AddressInfo Cx4::GetAbsoluteAddress(uint32_t address)
{
	return { -1, MemoryType::None };
}

MemoryMappings* Cx4::GetMemoryMappings()
{
	return &_mappings;
}

Cx4State& Cx4::GetState()
{
	return _state;
}