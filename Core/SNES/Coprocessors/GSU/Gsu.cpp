#include "pch.h"
#include "SNES/Coprocessors/GSU/Gsu.h"
#include "SNES/Coprocessors/GSU/GsuRomHandler.h"
#include "SNES/Coprocessors/GSU/GsuRamHandler.h"
#include "SNES/SnesConsole.h"
#include "SNES/SnesCpu.h"
#include "SNES/SnesMemoryManager.h"
#include "SNES/BaseCartridge.h"
#include "SNES/RamHandler.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/MessageManager.h"
#include "Shared/BatteryManager.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/Serializer.h"
#include "Shared/MemoryOperationType.h"

Gsu::Gsu(SnesConsole *console, uint32_t gsuRamSize)
{
	_emu = console->GetEmulator();
	_console = console;
	_memoryManager = console->GetMemoryManager();
	_cpu = console->GetCpu();
	_settings = _emu->GetSettings();

	_clockMultiplier = std::max(1u, _settings->GetSnesConfig().GsuClockSpeed / 100);

	_state = {};
	_state.ProgramReadBuffer = 0x01; //Run a NOP on first cycle

	_console->InitializeRam(_cache, 512);

	_gsuRamSize = gsuRamSize;
	_gsuRam = new uint8_t[_gsuRamSize];
	_emu->RegisterMemory(MemoryType::GsuWorkRam, _gsuRam, _gsuRamSize);
	_console->InitializeRam(_gsuRam, _gsuRamSize);

	for(uint32_t i = 0; i < _gsuRamSize / 0x1000; i++) {
		_gsuRamHandlers.push_back(unique_ptr<IMemoryHandler>(new RamHandler(_gsuRam, i * 0x1000, _gsuRamSize, MemoryType::GsuWorkRam)));
		_gsuCpuRamHandlers.push_back(unique_ptr<IMemoryHandler>(new GsuRamHandler(_state, _gsuRamHandlers.back().get())));
	}
	
	//CPU mappings
	MemoryMappings *cpuMappings = _memoryManager->GetMemoryMappings();
	vector<unique_ptr<IMemoryHandler>> &prgRomHandlers = _console->GetCartridge()->GetPrgRomHandlers();
	for(unique_ptr<IMemoryHandler> &handler : prgRomHandlers) {
		_gsuCpuRomHandlers.push_back(unique_ptr<IMemoryHandler>(new GsuRomHandler(_state, handler.get())));
	}

	//GSU registers in CPU memory space
	cpuMappings->RegisterHandler(0x00, 0x3F, 0x3000, 0x3FFF, this);
	cpuMappings->RegisterHandler(0x80, 0xBF, 0x3000, 0x3FFF, this);

	for(int i = 0; i < 0x3F; i++) {
		cpuMappings->RegisterHandler(i, i, 0x6000, 0x7FFF, _gsuCpuRamHandlers);
		cpuMappings->RegisterHandler(i + 0x80, i + 0x80, 0x6000, 0x7FFF, _gsuCpuRamHandlers);
	}
	cpuMappings->RegisterHandler(0x70, 0x71, 0x0000, 0xFFFF, _gsuCpuRamHandlers);
	cpuMappings->RegisterHandler(0xF0, 0xF1, 0x0000, 0xFFFF, _gsuCpuRamHandlers);

	cpuMappings->RegisterHandler(0x00, 0x3F, 0x8000, 0xFFFF, _gsuCpuRomHandlers);
	cpuMappings->RegisterHandler(0x80, 0xBF, 0x8000, 0xFFFF, _gsuCpuRomHandlers);

	cpuMappings->RegisterHandler(0x40, 0x5F, 0x0000, 0xFFFF, _gsuCpuRomHandlers);
	cpuMappings->RegisterHandler(0xC0, 0xDF, 0x0000, 0xFFFF, _gsuCpuRomHandlers);

	//GSU mappings
	_mappings.RegisterHandler(0x00, 0x3F, 0x8000, 0xFFFF, prgRomHandlers);
	_mappings.RegisterHandler(0x00, 0x3F, 0x0000, 0x7FFF, prgRomHandlers); //Mirror

	_mappings.RegisterHandler(0x40, 0x5F, 0x0000, 0xFFFF, prgRomHandlers);
	_mappings.RegisterHandler(0x70, 0x71, 0x0000, 0xFFFF, _gsuRamHandlers);
}

Gsu::~Gsu()
{
	delete[] _gsuRam;
}

void Gsu::ProcessEndOfFrame()
{
	uint8_t clockMultiplier = std::max(1u, _settings->GetSnesConfig().GsuClockSpeed / 100);
	if(_clockMultiplier != clockMultiplier) {
		_state.CycleCount = (uint64_t)((double)_state.CycleCount / _clockMultiplier * clockMultiplier);
		_clockMultiplier = clockMultiplier;
	}
}

void Gsu::Run()
{
	uint64_t targetCycle = _memoryManager->GetMasterClock() * _clockMultiplier;

	while(!_stopped && _state.CycleCount < targetCycle) {
		Exec();
	}

	if(targetCycle > _state.CycleCount) {
		Step(targetCycle - _state.CycleCount);
	}
}

void Gsu::Exec()
{
	uint8_t opCode = ReadOpCode();

	switch(opCode) {
		case 0x00: STOP(); break;
		case 0x01: NOP(); break;
		case 0x02: CACHE(); break;
		case 0x03: LSR(); break;
		case 0x04: ROL(); break;
		case 0x05: BRA(); break;
		case 0x06: BGE(); break;
		case 0x07: BLT(); break;
		case 0x08: BNE(); break;
		case 0x09: BEQ(); break;
		case 0x0A: BPL(); break;
		case 0x0B: BMI(); break;
		case 0x0C: BCC(); break;
		case 0x0D: BCS(); break;
		case 0x0E: BVC(); break;
		case 0x0F: BVS(); break;

		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1E: case 0x1F:
			TO(opCode & 0x0F);
			break;

		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
		case 0x28: case 0x29: case 0x2A: case 0x2B: case 0x2C: case 0x2D: case 0x2E: case 0x2F:
			WITH(opCode & 0x0F);
			break;

		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
		case 0x38: case 0x39: case 0x3A: case 0x3B:
			STORE(opCode & 0x0F);
			break;

		case 0x3C: LOOP(); break;
		case 0x3D: ALT1(); break;
		case 0x3E: ALT2(); break;
		case 0x3F: ALT3(); break;

		case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
		case 0x48: case 0x49: case 0x4A: case 0x4B:
			LOAD(opCode & 0x0F);
			break;

		case 0x4C: PlotRpix(); break;
		case 0x4D: SWAP(); break;
		case 0x4E: ColorCMode(); break;
		case 0x4F: NOT(); break;

		case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
		case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C: case 0x5D: case 0x5E: case 0x5F:
			Add(opCode & 0x0F);
			break;

		case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
		case 0x68: case 0x69: case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6E: case 0x6F:
			SubCompare(opCode & 0x0F);
			break;

		case 0x70: MERGE(); break;

		case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7E: case 0x7F:
			AndBitClear(opCode & 0x0F);
			break;

		case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
		case 0x88: case 0x89: case 0x8A: case 0x8B: case 0x8C: case 0x8D: case 0x8E: case 0x8F:
			MULT(opCode & 0x0F);
			break;

		case 0x90: SBK(); break;

		case 0x91: LINK(1); break;
		case 0x92: LINK(2); break;
		case 0x93: LINK(3); break;
		case 0x94: LINK(4); break;

		case 0x95: SignExtend(); break;

		case 0x96: ASR(); break;
		case 0x97: ROR(); break;

		case 0x98: case 0x99: case 0x9A: case 0x9B: case 0x9C: case 0x9D:
			JMP(opCode & 0x0F);
			break;

		case 0x9E: LOB(); break;
		case 0x9F: FMultLMult(); break;

		case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4: case 0xA5: case 0xA6: case 0xA7:
		case 0xA8: case 0xA9: case 0xAA: case 0xAB: case 0xAC: case 0xAD: case 0xAE: case 0xAF:
			IbtSmsLms(opCode & 0x0F);
			break;

		case 0xB0: case 0xB1: case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB6: case 0xB7:
		case 0xB8: case 0xB9: case 0xBA: case 0xBB: case 0xBC: case 0xBD: case 0xBE: case 0xBF:
			FROM(opCode & 0x0F);
			break;

		case 0xC0: HIB(); break;

		case 0xC1: case 0xC2: case 0xC3: case 0xC4: case 0xC5: case 0xC6: case 0xC7:
		case 0xC8: case 0xC9: case 0xCA: case 0xCB: case 0xCC: case 0xCD: case 0xCE: case 0xCF:
			OrXor(opCode & 0x0F);
			break;

		case 0xD0: case 0xD1: case 0xD2: case 0xD3: case 0xD4: case 0xD5: case 0xD6: case 0xD7:
		case 0xD8: case 0xD9: case 0xDA: case 0xDB: case 0xDC: case 0xDD: case 0xDE:
			INC(opCode & 0x0F);
			break;

		case 0xDF: GetCRamBRomB(); break;

		case 0xE0: case 0xE1: case 0xE2: case 0xE3: case 0xE4: case 0xE5: case 0xE6: case 0xE7:
		case 0xE8: case 0xE9: case 0xEA: case 0xEB: case 0xEC: case 0xED: case 0xEE:
			DEC(opCode & 0x0F);
			break;

		case 0xEF: GETB(); break;

		case 0xF0: case 0xF1: case 0xF2: case 0xF3: case 0xF4: case 0xF5: case 0xF6: case 0xF7:
		case 0xF8: case 0xF9: case 0xFA: case 0xFB: case 0xFC: case 0xFD: case 0xFE: case 0xFF:
			IwtLmSm(opCode & 0x0F);
			break;
	}

	if(_state.SFR.Running) {
		_emu->ProcessInstruction<CpuType::Gsu>();
	}

	if(!_r15Changed) {
		_state.R[15]++;
	} else {
		_r15Changed = false;
	}
}

uint8_t Gsu::ReadGsu(uint32_t addr, MemoryOperationType opType)
{
	IMemoryHandler *handler = _mappings.GetHandler(addr);
	uint8_t value;
	if(handler) {
		value = handler->Read(addr);
	} else {
		//TODO: Open bus?
		value = 0;
		LogDebug("[Debug] GSU - Missing read handler: " + HexUtilities::ToHex(addr));
	}
	_emu->ProcessMemoryRead<CpuType::Gsu>(addr, value, opType);

	return value;
}

void Gsu::WriteGsu(uint32_t addr, uint8_t value, MemoryOperationType opType)
{
	if(_emu->ProcessMemoryWrite<CpuType::Gsu>(addr, value, opType)) {
		IMemoryHandler* handler = _mappings.GetHandler(addr);
		if(handler) {
			handler->Write(addr, value);
		} else {
			LogDebug("[Debug] GSU - Missing write handler: " + HexUtilities::ToHex(addr));
		}
	}
}

void Gsu::InitProgramCache(uint16_t cacheAddr)
{
	uint16_t dest = (cacheAddr & 0x01F0);
	
	if(_state.ProgramBank <= 0x5F) {
		WaitRomOperation();
		WaitForRomAccess();
	} else {
		WaitRamOperation();
		WaitForRamAccess();
	}
	
	uint32_t srcBaseAddr = (_state.ProgramBank << 16) + _state.CacheBase + dest;
	for(int i = 0; i < 16; i++) {
		_cache[dest + i] = ReadGsu(srcBaseAddr + i, MemoryOperationType::Read);
	}
	Step(_state.ClockSelect ? 5*16 : 6*16);

	_cacheValid[cacheAddr >> 4] = true;
}

uint8_t Gsu::ReadOperand()
{
	uint8_t result = _state.ProgramReadBuffer;
	_state.R[15]++;
	_state.ProgramReadBuffer = ReadProgramByte(MemoryOperationType::ExecOperand);
	return result;
}

uint8_t Gsu::ReadOpCode()
{
	uint8_t result = _state.ProgramReadBuffer;
	_state.ProgramReadBuffer = ReadProgramByte(MemoryOperationType::ExecOpCode);
	return result;
}

uint8_t Gsu::ReadProgramByte(MemoryOperationType opType)
{
	_lastOpAddr = (_state.ProgramBank << 16) | _state.R[15];
	uint16_t cacheAddr = _state.R[15] - _state.CacheBase;
	if(cacheAddr < 512) {
		if(!_cacheValid[cacheAddr >> 4]) {
			InitProgramCache(cacheAddr & 0xFFF0);
		}
		
		Step(_state.ClockSelect ? 1 : 2);
		_emu->ProcessMemoryRead<CpuType::Gsu>(_lastOpAddr, _cache[cacheAddr], opType);
		return _cache[cacheAddr];
	} else {
		if(_state.ProgramBank <= 0x5F) {
			WaitRomOperation();
			WaitForRomAccess();
		} else {
			WaitRamOperation();
			WaitForRamAccess();
		}
		Step(_state.ClockSelect ? 5 : 6);
		return ReadGsu(_lastOpAddr, opType);
	}
}

uint16_t Gsu::ReadSrcReg()
{
	return _state.R[_state.SrcReg];
}

void Gsu::WriteDestReg(uint16_t value)
{
	WriteRegister(_state.DestReg, value);
}

void Gsu::WriteRegister(uint8_t reg, uint16_t value)
{
	_state.R[reg] = value;
	
	if(reg == 14) {
		_state.SFR.RomReadPending = true;
		_state.RomDelay = _state.ClockSelect ? 5 : 6;
	} else if(reg == 15) {
		_r15Changed = true;
	}
}

void Gsu::ResetFlags()
{
	_state.SFR.Prefix = 0;
	_state.SFR.Alt1 = false;
	_state.SFR.Alt2 = false;

	_state.SrcReg = 0;
	_state.DestReg = 0;
}

void Gsu::InvalidateCache()
{
	memset(_cacheValid, 0, sizeof(_cacheValid));
}

void Gsu::WaitRomOperation()
{
	if(_state.RomDelay) {
		//Wait for existing RAM operation to complete
		Step(_state.RomDelay);
	}
}

void Gsu::WaitRamOperation()
{
	if(_state.RamDelay) {
		//Wait for existing RAM operation to complete
		Step(_state.RamDelay);
	}
}

void Gsu::WaitForRomAccess()
{
	if(!_state.GsuRomAccess) {
		_waitForRomAccess = true;
		_stopped = true;
	}
}

void Gsu::WaitForRamAccess()
{
	if(!_state.GsuRamAccess) {
		_waitForRamAccess = true;
		_stopped = true;
	}
}

void Gsu::UpdateRunningState()
{
	_stopped = !_state.SFR.Running || _waitForRamAccess || _waitForRomAccess;
}

uint8_t Gsu::ReadRomBuffer()
{
	WaitRomOperation();
	return _state.RomReadBuffer;
}

uint8_t Gsu::ReadRamBuffer(uint16_t addr)
{
	WaitRamOperation();
	WaitForRamAccess();
	return ReadGsu(0x700000 | (_state.RamBank << 16) | addr, MemoryOperationType::Read);
}

void Gsu::WriteRam(uint16_t addr, uint8_t value)
{
	WaitRamOperation();

	_state.RamDelay = _state.ClockSelect ? 5 : 6;
	_state.RamWriteAddress = addr;
	_state.RamWriteValue = value;
}

void Gsu::Step(uint64_t cycles)
{
	_state.CycleCount += cycles;

	if(_state.RomDelay) {
		_state.RomDelay -= std::min<uint8_t>((uint8_t)cycles, _state.RomDelay);
		if(_state.RomDelay == 0) {
			WaitForRomAccess();
			_state.RomReadBuffer = ReadGsu((_state.RomBank << 16) | _state.R[14], MemoryOperationType::Read);
			_state.SFR.RomReadPending = false;
		}
	}

	if(_state.RamDelay) {
		_state.RamDelay -= std::min<uint8_t>((uint8_t)cycles, _state.RamDelay);
		if(_state.RamDelay == 0) {
			WaitForRamAccess();
			WriteGsu(0x700000 | (_state.RamBank << 16) | _state.RamWriteAddress, _state.RamWriteValue, MemoryOperationType::Write);
		}
	}
}

void Gsu::Reset()
{
	_state = {};
	_state.ProgramReadBuffer = 0x01; //Run a NOP on first cycle
	
	_console->InitializeRam(_cache, 512);
	memset(_cacheValid, 0, sizeof(_cacheValid));
	_waitForRomAccess = false;
	_waitForRamAccess = false;
	_stopped = true;
	_lastOpAddr = 0;
}

uint8_t Gsu::Read(uint32_t addr)
{
	addr &= 0x33FF;
	if(_state.SFR.Running && addr != 0x3030 && addr != 0x3031 && addr != 0x303B) {
		//"During GSU operation, only SFR, SCMR, and VCR may be accessed."
		return 0;
	}

	switch(addr) {
		case 0x3000: case 0x3002: case 0x3004: case 0x3006: case 0x3008: case 0x300A: case 0x300C:case 0x300E:
		case 0x3010: case 0x3012: case 0x3014: case 0x3016: case 0x3018: case 0x301A: case 0x301C:case 0x301E:
			return (uint8_t)_state.R[(addr >> 1) & 0x0F];

		case 0x3001: case 0x3003: case 0x3005: case 0x3007: case 0x3009: case 0x300B: case 0x300D:case 0x300F:
		case 0x3011: case 0x3013: case 0x3015: case 0x3017: case 0x3019: case 0x301B: case 0x301D:case 0x301F:
			return _state.R[(addr >> 1) & 0x0F] >> 8;

		case 0x3030: return _state.SFR.GetFlagsLow();
		case 0x3031: {
			uint8_t flags = _state.SFR.GetFlagsHigh();
			_state.SFR.Irq = false;
			_cpu->ClearIrqSource(SnesIrqSource::Coprocessor);
			return flags;
		}

		case 0x3034: return _state.ProgramBank;
		case 0x3036: return _state.RomBank;
		case 0x303B: return 0x04; //Version (can be 1 or 4?)
		case 0x303C: return _state.RamBank;
		case 0x303E: return (uint8_t)_state.CacheBase;
		case 0x303F: return _state.CacheBase >> 8;
	}
	
	if(addr >= 0x3100 && addr <= 0x32FF) {
		return _cache[(_state.CacheBase + (addr - 0x3100)) & 0x1FF];
	}

	LogDebug("[Debug] Missing read handler: $" + HexUtilities::ToHex(addr));

	//TODO open bus and proper mirroring?
	return 0;
}

void Gsu::Write(uint32_t addr, uint8_t value)
{
	addr &= 0x33FF;
	if(_state.SFR.Running && addr != 0x3030 && addr != 0x303A) {
		//"During GSU operation, only SFR, SCMR, and VCR may be accessed."
		return;
	}

	switch(addr) {
		case 0x3000: case 0x3002: case 0x3004: case 0x3006: case 0x3008: case 0x300A: case 0x300C: case 0x300E:
		case 0x3010: case 0x3012: case 0x3014: case 0x3016: case 0x3018: case 0x301A: case 0x301C: case 0x301E:
			_state.RegisterLatch = value;
			break;

		case 0x3001: case 0x3003: case 0x3005: case 0x3007: case 0x3009: case 0x300B: case 0x300D: case 0x300F:
		case 0x3011: case 0x3013: case 0x3015: case 0x3017: case 0x3019: case 0x301B: case 0x301D: case 0x301F: {
			uint8_t reg = (addr >> 1) & 0x0F;
			_state.R[reg] = (value << 8) | _state.RegisterLatch;
			
			if(reg == 14) {
				_state.SFR.RomReadPending = true;
				_state.RomDelay = _state.ClockSelect ? 5 : 6;
			} else if(addr == 0x301F) {
				_state.SFR.Running = true;
				UpdateRunningState();
			}
			break;
		}

		case 0x3030: {
			bool running = _state.SFR.Running;
			_state.SFR.Zero = (value & 0x02) != 0;
			_state.SFR.Carry = (value & 0x04) != 0;
			_state.SFR.Sign = (value & 0x08) != 0;
			_state.SFR.Overflow = (value & 0x10) != 0;
			_state.SFR.Running = (value & 0x20) != 0;

			if(running && !_state.SFR.Running) {
				_state.CacheBase = 0;
				InvalidateCache();
			}
			UpdateRunningState();
			break;
		}

		case 0x3033: _state.BackupRamEnabled = (value & 0x01); break;
		case 0x3034: _state.ProgramBank = (value & 0x7F); InvalidateCache(); break;
		
		case 0x3037:
			_state.HighSpeedMode = (value & 0x20) != 0;
			_state.IrqDisabled = (value & 0x80) != 0;
			break;

		case 0x3038: _state.ScreenBase = value; break;
		case 0x3039: _state.ClockSelect = (value & 0x01); break;

		case 0x303A:
			_state.ColorGradient = (value & 0x03);
			switch(_state.ColorGradient) {
				case 0: _state.PlotBpp = 2; break;
				case 1: _state.PlotBpp = 4; break;
				case 2: _state.PlotBpp = 4; break;
				case 3: _state.PlotBpp = 8; break;
			}
			_state.ScreenHeight = ((value & 0x04) >> 2) | ((value & 0x20) >> 4);
			_state.GsuRamAccess = (value & 0x08) != 0;
			_state.GsuRomAccess = (value & 0x10) != 0;

			if(_state.GsuRamAccess) {
				_waitForRamAccess = false;
			}
			if(_state.GsuRomAccess) {
				_waitForRomAccess = false;
			}
			UpdateRunningState();
			break;
	}

	if(addr >= 0x3100 && addr <= 0x32FF) {
		uint16_t cacheAddr = (_state.CacheBase + (addr - 0x3100)) & 0x1FF;
		_cache[cacheAddr] = value;
		if((cacheAddr & 0x0F) == 0x0F) {
			_cacheValid[cacheAddr >> 4] = true;
		}
	}
}

uint8_t Gsu::Peek(uint32_t addr)
{
	return 0;
}

void Gsu::PeekBlock(uint32_t addr, uint8_t *output)
{
	memset(output, 0, 0x1000);
}

AddressInfo Gsu::GetAbsoluteAddress(uint32_t address)
{
	return { -1, MemoryType::None };
}

void Gsu::Serialize(Serializer &s)
{
	SV(_state.CycleCount); SV(_state.RegisterLatch); SV(_state.ProgramBank); SV(_state.RomBank); SV(_state.RamBank); SV(_state.IrqDisabled);
	SV(_state.HighSpeedMode); SV(_state.ClockSelect); SV(_state.BackupRamEnabled); SV(_state.ScreenBase); SV(_state.ColorGradient); SV(_state.PlotBpp);
	SV(_state.ScreenHeight); SV(_state.GsuRamAccess); SV(_state.GsuRomAccess); SV(_state.CacheBase); SV(_state.PlotTransparent); SV(_state.PlotDither);
	SV(_state.ColorHighNibble); SV(_state.ColorFreezeHigh); SV(_state.ObjMode); SV(_state.ColorReg); SV(_state.SrcReg); SV(_state.DestReg);
	SV(_state.RomReadBuffer); SV(_state.RomDelay); SV(_state.ProgramReadBuffer); SV(_state.RamWriteAddress); SV(_state.RamWriteValue); SV(_state.RamDelay);
	SV(_state.RamAddress); SV(_state.PrimaryCache.X); SV(_state.PrimaryCache.Y); SV(_state.PrimaryCache.ValidBits); SV(_state.SecondaryCache.X);
	SV(_state.SecondaryCache.Y); SV(_state.SecondaryCache.ValidBits);
	SV(_state.SFR.Alt1); SV(_state.SFR.Alt2); SV(_state.SFR.Carry); SV(_state.SFR.ImmHigh); SV(_state.SFR.ImmLow); SV(_state.SFR.Irq); SV(_state.SFR.Overflow);
	SV(_state.SFR.Prefix); SV(_state.SFR.RomReadPending); SV(_state.SFR.Running); SV(_state.SFR.Sign); SV(_state.SFR.Zero);

	SVArray(_state.R, 16);
	SVArray(_state.PrimaryCache.Pixels, 8);
	SVArray(_state.SecondaryCache.Pixels, 8);

	SV(_waitForRamAccess); SV(_waitForRomAccess); SV(_stopped);
	SVArray(_cacheValid, 32);
	SVArray(_cache, 512);
	SVArray(_gsuRam, _gsuRamSize);
}

void Gsu::LoadBattery()
{
	_emu->GetBatteryManager()->LoadBattery(".srm", (uint8_t*)_gsuRam, _gsuRamSize);
}

void Gsu::SaveBattery()
{
	_emu->GetBatteryManager()->SaveBattery(".srm", (uint8_t*)_gsuRam, _gsuRamSize);
}

GsuState& Gsu::GetState()
{
	return _state;
}

MemoryMappings* Gsu::GetMemoryMappings()
{
	return &_mappings;
}

uint32_t Gsu::DebugGetProgramCounter()
{
	return _lastOpAddr;
}

void Gsu::DebugSetProgramCounter(uint32_t addr)
{
	_state.ProgramBank = (addr >> 16) & 0xFF;
	_state.R[15] = addr & 0xFFFF;
	
	_lastOpAddr = addr & 0xFFFFFF;
	IMemoryHandler* handler = _mappings.GetHandler(_lastOpAddr);
	_state.ProgramReadBuffer = handler ? handler->Read(_lastOpAddr) : 0;
}
