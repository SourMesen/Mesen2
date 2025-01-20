#include "pch.h"
#include "Gameboy/GbCpu.h"
#include "Gameboy/GbPpu.h"
#include "Gameboy/GbTimer.h"
#include "Gameboy/Gameboy.h"
#include "Gameboy/GbMemoryManager.h"
#include "Gameboy/GbControlManager.h"
#include "Shared/Emulator.h"
#include "Utilities/Serializer.h"

void GbCpu::Init(Emulator* emu, Gameboy* gameboy, GbMemoryManager* memoryManager)
{
	_emu = emu;
	_gameboy = gameboy;

	_ppu = gameboy->GetPpu();
	_memoryManager = memoryManager;
	
	_state = {};

	_state.PC = 0;
	_state.SP = 0xFFFF;
	_state.CycleCount = 6; //Makes boot_sclk_align serial test pass
}

GbCpu::~GbCpu()
{
}

GbCpuState& GbCpu::GetState()
{
	return _state;
}

bool GbCpu::IsHalted()
{
	return _state.HaltCounter > 0;
}

bool GbCpu::HandleStoppedState()
{
	if((((GbControlManager*)_gameboy->GetControlManager())->ReadInputPort() & 0x0F) != 0x0F) {
		_state.Stopped = false;
		_state.HaltCounter = 0;
		_ppu->SetCpuStopState(false);
		return true;
	}
	return false;
}

void GbCpu::Exec()
{
#ifndef DUMMYCPU
	if(_state.Stopped) {
		if(HandleStoppedState()) {
			return;
		}
	} else if(_prevIrqVector && !_state.HaltBug) {
		if(_state.IME) {
			uint16_t oldPc = _state.PC;
			ExecCpuCycle();
			ExecCpuCycle();

			PushByte(_state.PC >> 8);
			uint8_t irqVector = _memoryManager->ProcessIrqRequests(); //Check IRQ line again before jumping (ie_push)
			PushByte((uint8_t)_state.PC);

			ExecCpuCycle();
			
			switch(irqVector) {
				case 0:
					//IRQ request bit is no longer set, jump to $0000 (ie_push test)
					_state.PC = 0;
					break;

				case GbIrqSource::VerticalBlank:
					_gameboy->RefreshRamCheats();
					_state.PC = 0x40;
					break;

				case GbIrqSource::LcdStat: _state.PC = 0x48; break;
				case GbIrqSource::Timer: _state.PC = 0x50; break;
				case GbIrqSource::Serial: _state.PC = 0x58; break;
				case GbIrqSource::Joypad: _state.PC = 0x60; break;
			}
			if(irqVector) {
				//Only clear IRQ bit if an IRQ was processed
				_memoryManager->ClearIrqRequest(irqVector);
			}

			//Clear any pending IME update from a EI op - this can happen if EI is executed twice in a row
			//just before an interrupt occurs, which incorrectly caused IME to be set to true at the
			//start of the interrupt handler (which breaks "Batman - The Video Game")
			_state.EiPending = false;

			_state.IME = false;
			_emu->ProcessInterrupt<CpuType::Gameboy>(oldPc, _state.PC, false);
		}
		_state.HaltCounter = 0;
	}
#endif

	if(_state.HaltCounter) {
		if(_state.HaltBug) {
			ProcessHaltBug();
		} else {
#ifndef DUMMYCPU
			_emu->ProcessHaltedCpu<CpuType::Gameboy>();
			if(_state.HaltCounter > 1) {
				ProcessCgbSpeedSwitch();
			}
#endif
		}
	} else {
		if(_state.EiPending) {
			_state.EiPending = false;
			_state.IME = true;
		}

#ifndef DUMMYCPU
		_emu->ProcessInstruction<CpuType::Gameboy>();
#endif
		ExecOpCode(ReadOpCode());
	}

	ProcessNextCycleStart();
}

void GbCpu::PowerOn()
{
	ProcessNextCycleStart();
}

void GbCpu::ProcessNextCycleStart()
{
	if(_state.HaltCounter) {
		//When halted, the timing at which the CPU checks the IRQ state seems
		//to differ from the timing for other instructions. 
		//This is needed to make various tests pass.
		if(_gameboy->IsCgb()) {
			//On CGB, it looks like the IRQ is checked slightly earlier?
			if(_memoryManager->GetState().CgbSwitchSpeedRequest) {
				//Prevent IRQs when the CPU is switching speed
				//This might not be entirely correct? but Conker's Pocket Tales in GBC mode locks up without this
				//The "Color Panel Demo" fails to switch speed without this, too (which breaks the video output)
				_prevIrqVector = 0;
			} else {
				_prevIrqVector = _memoryManager->ProcessIrqRequests();
			}
			ExecCpuCycle();
		} else {
			//DMG HALT seems to check the irq state slightly later vs CGB?
			ExecMasterCycle();
			_prevIrqVector = _memoryManager->ProcessIrqRequests();
			ExecMasterCycle();
			ExecMasterCycle();
			ExecMasterCycle();
		}
	} else {
		ExecCpuCycle();
		_prevIrqVector = _memoryManager->ProcessIrqRequests();
	}
}

void GbCpu::ProcessHaltBug()
{
	if(_state.EiPending) {
		_state.EiPending = false;
		_state.IME = true;
	}

#ifndef DUMMYCPU
	_emu->ProcessInstruction<CpuType::Gameboy>();
#endif

	//HALT bug, execution continues, but PC isn't incremented for the first byte
	ExecCpuCycle();
	uint8_t opCode = ReadOpCode();
	_state.PC--;
	_state.HaltCounter = 0;
	_state.HaltBug = false;
	ExecOpCode(opCode);
}

void GbCpu::ExecOpCode(uint8_t opCode)
{
	switch(opCode) {
		case 0x00: NOP(); break;
		case 0x01: LD(_regBC, ReadCodeWord()); break;
		case 0x02: LD_Indirect(_regBC, _state.A); break;
		case 0x03: INC(_regBC); break;
		case 0x04: INC(_state.B); break;
		case 0x05: DEC(_state.B); break;
		case 0x06: LD(_state.B, ReadCode()); break;
		case 0x07: RLCA(); break;
		case 0x08: LD_Indirect16(ReadCodeWord(), _state.SP); break;
		case 0x09: ADD(_regHL, _regBC); break;
		case 0x0A: LD(_state.A, Read(_regBC)); break;
		case 0x0B: DEC(_regBC); break;
		case 0x0C: INC(_state.C); break;
		case 0x0D: DEC(_state.C); break;
		case 0x0E: LD(_state.C, ReadCode()); break;
		case 0x0F: RRCA(); break;
		case 0x10: STOP(); break;
		case 0x11: LD(_regDE, ReadCodeWord()); break;
		case 0x12: LD_Indirect(_regDE, _state.A); break;
		case 0x13: INC(_regDE); break;
		case 0x14: INC(_state.D); break;
		case 0x15: DEC(_state.D); break;
		case 0x16: LD(_state.D, ReadCode()); break;
		case 0x17: RLA(); break;
		case 0x18: JR(ReadCode()); break;
		case 0x19: ADD(_regHL, _regDE); break;
		case 0x1A: LD(_state.A, Read(_regDE)); break;
		case 0x1B: DEC(_regDE); break;
		case 0x1C: INC(_state.E); break;
		case 0x1D: DEC(_state.E); break;
		case 0x1E: LD(_state.E, ReadCode()); break;
		case 0x1F: RRA(); break;
		case 0x20: JR((_state.Flags & GbCpuFlags::Zero) == 0, ReadCode()); break;
		case 0x21: LD(_regHL, ReadCodeWord()); break;
		case 0x22: LD_Indirect(_regHL, _state.A); _regHL.Inc(); break;
		case 0x23: INC(_regHL); break;
		case 0x24: INC(_state.H); break;
		case 0x25: DEC(_state.H); break;
		case 0x26: LD(_state.H, ReadCode()); break;
		case 0x27: DAA(); break;
		case 0x28: JR((_state.Flags & GbCpuFlags::Zero) != 0, ReadCode()); break;
		case 0x29: ADD(_regHL, _regHL); break;
		case 0x2A: LD(_state.A, Read<GbOamCorruptionType::ReadIncDec>(_regHL)); _regHL.Inc(); break;
		case 0x2B: DEC(_regHL); break;
		case 0x2C: INC(_state.L); break;
		case 0x2D: DEC(_state.L); break;
		case 0x2E: LD(_state.L, ReadCode()); break;
		case 0x2F: CPL(); break;
		case 0x30: JR((_state.Flags & GbCpuFlags::Carry) == 0, ReadCode()); break;
		case 0x31: LD(_state.SP, ReadCodeWord()); break;
		case 0x32: LD_Indirect(_regHL, _state.A); _regHL.Dec(); break;
		case 0x33: INC_SP(); break;
		case 0x34: INC_Indirect(_regHL); break;
		case 0x35: DEC_Indirect(_regHL); break;
		case 0x36: LD_Indirect(_regHL, ReadCode()); break;
		case 0x37: SCF(); break;
		case 0x38: JR((_state.Flags & GbCpuFlags::Carry) != 0, ReadCode()); break;
		case 0x39: ADD(_regHL, _state.SP); break;
		case 0x3A: LD(_state.A, Read<GbOamCorruptionType::ReadIncDec>(_regHL)); _regHL.Dec(); break;
		case 0x3B: DEC_SP(); break;
		case 0x3C: INC(_state.A); break;
		case 0x3D: DEC(_state.A); break;
		case 0x3E: LD(_state.A, ReadCode()); break;
		case 0x3F: CCF(); break;
		case 0x40: LD(_state.B, _state.B); break;
		case 0x41: LD(_state.B, _state.C); break;
		case 0x42: LD(_state.B, _state.D); break;
		case 0x43: LD(_state.B, _state.E); break;
		case 0x44: LD(_state.B, _state.H); break;
		case 0x45: LD(_state.B, _state.L); break;
		case 0x46: LD(_state.B, Read(_regHL)); break;
		case 0x47: LD(_state.B, _state.A); break;
		case 0x48: LD(_state.C, _state.B); break;
		case 0x49: LD(_state.C, _state.C); break;
		case 0x4A: LD(_state.C, _state.D); break;
		case 0x4B: LD(_state.C, _state.E); break;
		case 0x4C: LD(_state.C, _state.H); break;
		case 0x4D: LD(_state.C, _state.L); break;
		case 0x4E: LD(_state.C, Read(_regHL)); break;
		case 0x4F: LD(_state.C, _state.A); break;
		case 0x50: LD(_state.D, _state.B); break;
		case 0x51: LD(_state.D, _state.C); break;
		case 0x52: LD(_state.D, _state.D); break;
		case 0x53: LD(_state.D, _state.E); break;
		case 0x54: LD(_state.D, _state.H); break;
		case 0x55: LD(_state.D, _state.L); break;
		case 0x56: LD(_state.D, Read(_regHL)); break;
		case 0x57: LD(_state.D, _state.A); break;
		case 0x58: LD(_state.E, _state.B); break;
		case 0x59: LD(_state.E, _state.C); break;
		case 0x5A: LD(_state.E, _state.D); break;
		case 0x5B: LD(_state.E, _state.E); break;
		case 0x5C: LD(_state.E, _state.H); break;
		case 0x5D: LD(_state.E, _state.L); break;
		case 0x5E: LD(_state.E, Read(_regHL)); break;
		case 0x5F: LD(_state.E, _state.A); break;
		case 0x60: LD(_state.H, _state.B); break;
		case 0x61: LD(_state.H, _state.C); break;
		case 0x62: LD(_state.H, _state.D); break;
		case 0x63: LD(_state.H, _state.E); break;
		case 0x64: LD(_state.H, _state.H); break;
		case 0x65: LD(_state.H, _state.L); break;
		case 0x66: LD(_state.H, Read(_regHL)); break;
		case 0x67: LD(_state.H, _state.A); break;
		case 0x68: LD(_state.L, _state.B); break;
		case 0x69: LD(_state.L, _state.C); break;
		case 0x6A: LD(_state.L, _state.D); break;
		case 0x6B: LD(_state.L, _state.E); break;
		case 0x6C: LD(_state.L, _state.H); break;
		case 0x6D: LD(_state.L, _state.L); break;
		case 0x6E: LD(_state.L, Read(_regHL)); break;
		case 0x6F: LD(_state.L, _state.A); break;
		case 0x70: LD_Indirect(_regHL, _state.B); break;
		case 0x71: LD_Indirect(_regHL, _state.C); break;
		case 0x72: LD_Indirect(_regHL, _state.D); break;
		case 0x73: LD_Indirect(_regHL, _state.E); break;
		case 0x74: LD_Indirect(_regHL, _state.H); break;
		case 0x75: LD_Indirect(_regHL, _state.L); break;
		case 0x76: HALT(); break;
		case 0x77: LD_Indirect(_regHL, _state.A); break;
		case 0x78: LD(_state.A, _state.B); break;
		case 0x79: LD(_state.A, _state.C); break;
		case 0x7A: LD(_state.A, _state.D); break;
		case 0x7B: LD(_state.A, _state.E); break;
		case 0x7C: LD(_state.A, _state.H); break;
		case 0x7D: LD(_state.A, _state.L); break;
		case 0x7E: LD(_state.A, Read(_regHL)); break;
		case 0x7F: LD(_state.A, _state.A); break;
		case 0x80: ADD(_state.B); break;
		case 0x81: ADD(_state.C); break;
		case 0x82: ADD(_state.D); break;
		case 0x83: ADD(_state.E); break;
		case 0x84: ADD(_state.H); break;
		case 0x85: ADD(_state.L); break;
		case 0x86: ADD(Read(_regHL)); break;
		case 0x87: ADD(_state.A); break;
		case 0x88: ADC(_state.B); break;
		case 0x89: ADC(_state.C); break;
		case 0x8A: ADC(_state.D); break;
		case 0x8B: ADC(_state.E); break;
		case 0x8C: ADC(_state.H); break;
		case 0x8D: ADC(_state.L); break;
		case 0x8E: ADC(Read(_regHL)); break;
		case 0x8F: ADC(_state.A); break;
		case 0x90: SUB(_state.B); break;
		case 0x91: SUB(_state.C); break;
		case 0x92: SUB(_state.D); break;
		case 0x93: SUB(_state.E); break;
		case 0x94: SUB(_state.H); break;
		case 0x95: SUB(_state.L); break;
		case 0x96: SUB(Read(_regHL)); break;
		case 0x97: SUB(_state.A); break;
		case 0x98: SBC(_state.B); break;
		case 0x99: SBC(_state.C); break;
		case 0x9A: SBC(_state.D); break;
		case 0x9B: SBC(_state.E); break;
		case 0x9C: SBC(_state.H); break;
		case 0x9D: SBC(_state.L); break;
		case 0x9E: SBC(Read(_regHL)); break;
		case 0x9F: SBC(_state.A); break;
		case 0xA0: AND(_state.B); break;
		case 0xA1: AND(_state.C); break;
		case 0xA2: AND(_state.D); break;
		case 0xA3: AND(_state.E); break;
		case 0xA4: AND(_state.H); break;
		case 0xA5: AND(_state.L); break;
		case 0xA6: AND(Read(_regHL)); break;
		case 0xA7: AND(_state.A); break;
		case 0xA8: XOR(_state.B); break;
		case 0xA9: XOR(_state.C); break;
		case 0xAA: XOR(_state.D); break;
		case 0xAB: XOR(_state.E); break;
		case 0xAC: XOR(_state.H); break;
		case 0xAD: XOR(_state.L); break;
		case 0xAE: XOR(Read(_regHL)); break;
		case 0xAF: XOR(_state.A); break;
		case 0xB0: OR(_state.B); break;
		case 0xB1: OR(_state.C); break;
		case 0xB2: OR(_state.D); break;
		case 0xB3: OR(_state.E); break;
		case 0xB4: OR(_state.H); break;
		case 0xB5: OR(_state.L); break;
		case 0xB6: OR(Read(_regHL)); break;
		case 0xB7: OR(_state.A); break;
		case 0xB8: CP(_state.B); break;
		case 0xB9: CP(_state.C); break;
		case 0xBA: CP(_state.D); break;
		case 0xBB: CP(_state.E); break;
		case 0xBC: CP(_state.H); break;
		case 0xBD: CP(_state.L); break;
		case 0xBE: CP(Read(_regHL)); break;
		case 0xBF: CP(_state.A); break;
		case 0xC0: RET((_state.Flags & GbCpuFlags::Zero) == 0); break;
		case 0xC1: POP(_regBC); break;
		case 0xC2: JP((_state.Flags & GbCpuFlags::Zero) == 0, ReadCodeWord()); break;
		case 0xC3: JP(ReadCodeWord()); break;
		case 0xC4: CALL((_state.Flags & GbCpuFlags::Zero) == 0, ReadCodeWord()); break;
		case 0xC5: PUSH(_regBC); break;
		case 0xC6: ADD(ReadCode()); break;
		case 0xC7: RST(0x00); break;
		case 0xC8: RET((_state.Flags & GbCpuFlags::Zero) != 0); break;
		case 0xC9: RET(); break;
		case 0xCA: JP((_state.Flags & GbCpuFlags::Zero) != 0, ReadCodeWord()); break;
		case 0xCB: PREFIX(); break;
		case 0xCC: CALL((_state.Flags & GbCpuFlags::Zero) != 0, ReadCodeWord()); break;
		case 0xCD: CALL(ReadCodeWord()); break;
		case 0xCE: ADC(ReadCode()); break;
		case 0xCF: RST(0x08); break;
		case 0xD0: RET((_state.Flags & GbCpuFlags::Carry) == 0); break;
		case 0xD1: POP(_regDE); break;
		case 0xD2: JP((_state.Flags & GbCpuFlags::Carry) == 0, ReadCodeWord()); break;
		case 0xD3: InvalidOp(); break;
		case 0xD4: CALL((_state.Flags & GbCpuFlags::Carry) == 0, ReadCodeWord()); break;
		case 0xD5: PUSH(_regDE); break;
		case 0xD6: SUB(ReadCode()); break;
		case 0xD7: RST(0x10); break;
		case 0xD8: RET((_state.Flags & GbCpuFlags::Carry) != 0); break;
		case 0xD9: RETI(); break;
		case 0xDA: JP((_state.Flags & GbCpuFlags::Carry) != 0, ReadCodeWord()); break;
		case 0xDB: InvalidOp(); break;
		case 0xDC: CALL((_state.Flags & GbCpuFlags::Carry) != 0, ReadCodeWord()); break;
		case 0xDD: InvalidOp(); break;
		case 0xDE: SBC(ReadCode()); break;
		case 0xDF: RST(0x18); break;
		case 0xE0: LD_Indirect(0xFF00 | ReadCode(), _state.A); break;
		case 0xE1: POP(_regHL); break;
		case 0xE2: LD_Indirect(0xFF00 | _state.C, _state.A); break;
		case 0xE3: InvalidOp(); break;
		case 0xE4: InvalidOp(); break;
		case 0xE5: PUSH(_regHL); break;
		case 0xE6: AND(ReadCode()); break;
		case 0xE7: RST(0x20); break;
		case 0xE8: ADD_SP(ReadCode()); break;
		case 0xE9: JP_HL(); break;
		case 0xEA: LD_Indirect(ReadCodeWord(), _state.A); break;
		case 0xEB: InvalidOp(); break;
		case 0xEC: InvalidOp(); break;
		case 0xED: InvalidOp(); break;
		case 0xEE: XOR(ReadCode()); break;
		case 0xEF: RST(0x28); break;
		case 0xF0: LD(_state.A, Read(0xFF00 | ReadCode())); break;
		case 0xF1: POP_AF(); break;
		case 0xF2: LD(_state.A, Read(0xFF00 | _state.C)); break;
		case 0xF3: DI(); break;
		case 0xF4: InvalidOp(); break;
		case 0xF5: PUSH(_regAF); break;
		case 0xF6: OR(ReadCode()); break;
		case 0xF7: RST(0x30); break;
		case 0xF8: LD_HL(ReadCode()); break;
		case 0xF9: LD(_state.SP, _regHL); ExecCpuCycle(); break;
		case 0xFA: LD(_state.A, Read(ReadCodeWord())); break;
		case 0xFB: EI(); break;
		case 0xFC: InvalidOp(); break;
		case 0xFD: InvalidOp(); break;
		case 0xFE: CP(ReadCode()); break;
		case 0xFF: RST(0x38); break;
	}
}

void GbCpu::ProcessCgbSpeedSwitch()
{
	//Process CGB switch speed
	_state.HaltCounter--;

	//Prevent DIV from ticking while switching speed
	_gameboy->GetTimer()->Write(0xFF04, 0);

	if(_state.HaltCounter == 1 && _memoryManager->GetState().CgbSwitchSpeedRequest) {
		_memoryManager->ToggleSpeed();
		_state.HaltCounter = 0;
	}
}

void GbCpu::ExecCpuCycle()
{
#ifndef DUMMYCPU
	_memoryManager->Exec();
	_memoryManager->Exec();
#endif
}

void GbCpu::ExecMasterCycle()
{
#ifndef DUMMYCPU
	_memoryManager->ExecMasterCycle();
#endif
}

uint8_t GbCpu::ReadOpCode()
{
	uint8_t value = ReadMemory<MemoryOperationType::ExecOpCode, GbOamCorruptionType::ReadIncDec>(_state.PC);
	_state.PC++;
	return value;
}

uint8_t GbCpu::ReadCode()
{
	ExecCpuCycle();
	uint8_t value = ReadMemory<MemoryOperationType::ExecOperand, GbOamCorruptionType::ReadIncDec>(_state.PC);
	_state.PC++;
	return value;
}

uint16_t GbCpu::ReadCodeWord()
{
	uint8_t low = ReadCode();
	uint8_t high = ReadCode();
	return (high << 8) | low;
}

template<GbOamCorruptionType oamCorruptionType>
uint8_t GbCpu::Read(uint16_t addr)
{
	ExecCpuCycle();
	uint8_t value = ReadMemory<MemoryOperationType::Read, oamCorruptionType>(addr);
	return value;
}

template<MemoryOperationType type, GbOamCorruptionType oamCorruptionType>
uint8_t GbCpu::ReadMemory(uint16_t addr)
{
#ifdef DUMMYCPU
	uint8_t value = _memoryManager->DebugRead(addr);
	LogMemoryOperation(addr, value, type);
	return value;
#else
	return _memoryManager->Read<type, oamCorruptionType>(addr);
#endif
}

void GbCpu::Write(uint16_t addr, uint8_t value)
{
#ifdef DUMMYCPU
	LogMemoryOperation(addr, value, MemoryOperationType::Write);
#else
	_memoryManager->ProcessCpuWrite(addr, value);
#endif
}

bool GbCpu::CheckFlag(uint8_t flag)
{
	return (_state.Flags & flag) != 0;
}

void GbCpu::SetFlag(uint8_t flag)
{
	_state.Flags |= flag;
}

void GbCpu::ClearFlag(uint8_t flag)
{
	_state.Flags &= ~flag;
}

void GbCpu::SetFlagState(uint8_t flag, bool state)
{
	if(state) {
		SetFlag(flag);
	} else {
		ClearFlag(flag);
	}
}

void GbCpu::PushByte(uint8_t value)
{
#ifndef DUMMYCPU
	_ppu->ProcessOamCorruption<GbOamCorruptionType::Write>(_state.SP);
#endif
	_state.SP--;
	Write(_state.SP, value);
}

void GbCpu::PushWord(uint16_t value)
{
	PushByte(value >> 8);
	PushByte((uint8_t)value);
}

uint16_t GbCpu::PopWord()
{
	uint8_t low = Read<GbOamCorruptionType::ReadIncDec>(_state.SP);
	_state.SP++;
	uint8_t high = Read<GbOamCorruptionType::Read>(_state.SP);
	_state.SP++;
	return (high << 8) | low;
}

void GbCpu::LD(uint8_t& dst, uint8_t value)
{
	dst = value;
}

void GbCpu::LD(uint16_t& dst, uint16_t value)
{
	dst = value;
}

void GbCpu::LD(Register16& dst, uint16_t value)
{
	dst.Write(value);
}

void GbCpu::LD_Indirect(uint16_t dst, uint8_t value)
{
	Write(dst, value);
}

void GbCpu::LD_Indirect16(uint16_t dst, uint16_t value)
{
	Write(dst, (uint8_t)value);
	Write(dst + 1, value >> 8);
}

//ld   HL,SP+dd  F8          12 00hc HL = SP +/- dd ;dd is 8bit signed number
void GbCpu::LD_HL(int8_t value)
{
	int result = (uint8_t)_state.SP + (uint8_t)value;

	//"Both of these set carry and half-carry based on the low byte of SP added to the UNSIGNED immediate byte. The Negative and Zero flags are always cleared."
	SetFlagState(GbCpuFlags::HalfCarry, (((uint8_t)_state.SP ^ result ^ value) & 0x10) != 0);
	SetFlagState(GbCpuFlags::Carry, result > 0xFF);

	_regHL.Write(_state.SP + value);

	ClearFlag(GbCpuFlags::AddSub);
	ClearFlag(GbCpuFlags::Zero);

	ExecCpuCycle();
}

// inc  r           xx         4 z0h- r=r+1
void GbCpu::INC(uint8_t& dst)
{
	SetFlagState(GbCpuFlags::HalfCarry, ((dst ^ 1 ^ (dst + 1)) & 0x10) != 0);
	dst++;

	SetFlagState(GbCpuFlags::Zero, dst == 0);
	ClearFlag(GbCpuFlags::AddSub);
}

// inc  rr        x3           8 ---- rr = rr+1      ;rr may be BC,DE,HL,SP
void GbCpu::INC(Register16& dst)
{
	//16-bit inc does not alter flags
	ExecCpuCycle();
#ifndef DUMMYCPU
	_ppu->ProcessOamCorruption<GbOamCorruptionType::Write>(dst.Read());
#endif
	dst.Inc();
}

// inc  rr        x3           8 ---- rr = rr+1      ;rr may be BC,DE,HL,SP
void GbCpu::INC_SP()
{
	ExecCpuCycle();
#ifndef DUMMYCPU
	_ppu->ProcessOamCorruption<GbOamCorruptionType::Write>(_state.SP);
#endif
	_state.SP++;
}

// inc  (HL)        34        12 z0h- (HL)=(HL)+1
void GbCpu::INC_Indirect(uint16_t addr)
{
	uint8_t val = Read(addr);
	INC(val);
	Write(addr, val);
}

// dec  r           xx         4 z1h- r=r-1
void GbCpu::DEC(uint8_t& dst)
{
	SetFlagState(GbCpuFlags::HalfCarry, (dst & 0x0F) == 0x00);
	dst--;

	SetFlagState(GbCpuFlags::Zero, dst == 0);
	SetFlag(GbCpuFlags::AddSub);
}

// dec  rr        xB           8 ---- rr = rr-1      ;rr may be BC,DE,HL,SP
void GbCpu::DEC(Register16& dst)
{
	//16-bit dec does not alter flags
	ExecCpuCycle();
#ifndef DUMMYCPU
	_ppu->ProcessOamCorruption<GbOamCorruptionType::Write>(dst.Read());
#endif
	dst.Dec();
}

// dec  (HL)        35        12 z1h- (HL)=(HL)-1
void GbCpu::DEC_Indirect(uint16_t addr)
{
	uint8_t val = Read(addr);
	DEC(val);
	Write(addr, val);
}

// dec  rr        xB           8 ---- rr = rr-1      ;rr may be BC,DE,HL,SP
void GbCpu::DEC_SP()
{
#ifndef DUMMYCPU
	_ppu->ProcessOamCorruption<GbOamCorruptionType::Write>(_state.SP);
#endif
	_state.SP--;
	ExecCpuCycle();
}

// add  A,r         8x         4 z0hc A=A+r
// add  A,n         C6 nn      8 z0hc A=A+n
// add  A,(HL)      86         8 z0hc A=A+(HL)
void GbCpu::ADD(uint8_t value)
{
	int result = _state.A + value;

	//SetFlagState(GbCpuFlags::HalfCarry, (_state.A & 0xF) + (value & 0xF) > 0xF);
	SetFlagState(GbCpuFlags::HalfCarry, ((_state.A ^ value ^ result) & 0x10) != 0);

	_state.A = (uint8_t)result;

	SetFlagState(GbCpuFlags::Carry, result > 0xFF);
	SetFlagState(GbCpuFlags::Zero, _state.A == 0);
	ClearFlag(GbCpuFlags::AddSub);
}

//add  SP,dd     E8          16 00hc SP = SP +/- dd ;dd is 8bit signed number
void GbCpu::ADD_SP(int8_t value)
{
	int result = (uint8_t)_state.SP + (uint8_t)value;

	//"Both of these set carry and half-carry based on the low byte of SP added to the UNSIGNED immediate byte. The Negative and Zero flags are always cleared."
	SetFlagState(GbCpuFlags::HalfCarry, (((uint8_t)_state.SP ^ result ^ value) & 0x10) != 0);
	SetFlagState(GbCpuFlags::Carry, result > 0xFF);

	_state.SP += value;

	ClearFlag(GbCpuFlags::AddSub);
	ClearFlag(GbCpuFlags::Zero);

	ExecCpuCycle();
	ExecCpuCycle();
}

// add  HL,rr     x9           8 -0hc HL = HL+rr     ;rr may be BC,DE,HL,SP
void GbCpu::ADD(Register16& reg, uint16_t value)
{
	int result = reg.Read() + value;

	SetFlagState(GbCpuFlags::HalfCarry, ((reg.Read() ^ value ^ result) & 0x1000) != 0);

	reg.Write((uint16_t)result);

	SetFlagState(GbCpuFlags::Carry, result > 0xFFFF);
	ClearFlag(GbCpuFlags::AddSub);
	ExecCpuCycle();
}

// adc  A,r         8x         4 z0hc A=A+r+cy
// adc  A,n         CE nn      8 z0hc A=A+n+cy
// adc  A,(HL)      8E         8 z0hc A=A+(HL)+cy
void GbCpu::ADC(uint8_t value)
{
	uint8_t carry = ((_state.Flags & GbCpuFlags::Carry) >> 4);
	int result = _state.A + value + carry;

	SetFlagState(GbCpuFlags::HalfCarry, (_state.A & 0x0F) + (value & 0x0F) + carry > 0x0F);

	_state.A = (uint8_t)result;

	SetFlagState(GbCpuFlags::Carry, result > 0xFF);
	SetFlagState(GbCpuFlags::Zero, _state.A == 0);
	ClearFlag(GbCpuFlags::AddSub);
}

//sub  r           9x         4 z1hc A = A - r
//sub  n           D6 nn      8 z1hc A = A - n
//sub(HL)          96         8 z1hc A = A - (HL)
void GbCpu::SUB(uint8_t value)
{
	int result = (int)_state.A - value;

	SetFlagState(GbCpuFlags::HalfCarry, (_state.A & 0x0F) < (value & 0x0F));

	_state.A = (uint8_t)result;

	SetFlagState(GbCpuFlags::Carry, result < 0);
	SetFlagState(GbCpuFlags::Zero, _state.A == 0);
	SetFlag(GbCpuFlags::AddSub);
}

// sbc  A,r         9x         4 z1hc A=A-r-cy
// sbc  A,n         DE nn      8 z1hc A=A-n-cy
// sbc  A,(HL)      9E         8 z1hc A=A-(HL)-cy
void GbCpu::SBC(uint8_t value)
{
	uint8_t carry = ((_state.Flags & GbCpuFlags::Carry) >> 4);
	int result = (int)_state.A - value - carry;

	SetFlagState(GbCpuFlags::HalfCarry, (_state.A & 0x0F) < (value & 0x0F) + carry);

	_state.A = (uint8_t)result;

	SetFlagState(GbCpuFlags::Carry, result < 0);
	SetFlagState(GbCpuFlags::Zero, _state.A == 0);
	SetFlag(GbCpuFlags::AddSub);
}

// and r           Ax         4 z010 A = A & r
// and n           E6 nn      8 z010 A = A & n
// and (HL)        A6         8 z010 A = A & (HL)
void GbCpu::AND(uint8_t value)
{
	_state.A &= value;
	SetFlagState(GbCpuFlags::Zero, _state.A == 0);
	ClearFlag(GbCpuFlags::AddSub);
	ClearFlag(GbCpuFlags::Carry);
	SetFlag(GbCpuFlags::HalfCarry);
}

//or   r         Bx         4 z000 A = A | r
//or n           F6 nn      8 z000 A = A | n
//or (HL)        B6         8 z000 A = A | (HL)
void GbCpu::OR(uint8_t value)
{
	_state.A |= value;
	SetFlagState(GbCpuFlags::Zero, _state.A == 0);
	ClearFlag(GbCpuFlags::AddSub);
	ClearFlag(GbCpuFlags::Carry);
	ClearFlag(GbCpuFlags::HalfCarry);
}

// xor  r          Ax         4 z000
// xor n           EE nn      8 z000
// xor (HL)        AE         8 z000
void GbCpu::XOR(uint8_t value)
{
	_state.A ^= value;
	SetFlagState(GbCpuFlags::Zero, _state.A == 0);
	ClearFlag(GbCpuFlags::AddSub);
	ClearFlag(GbCpuFlags::Carry);
	ClearFlag(GbCpuFlags::HalfCarry);
}

// cp   r          Bx         4 z1hc compare A-r
// cp   n          FE nn      8 z1hc compare A-n
// cp   (HL)       BE         8 z1hc compare A-(HL)
void GbCpu::CP(uint8_t value)
{
	int result = (int)_state.A - value;

	SetFlagState(GbCpuFlags::Carry, result < 0);
	SetFlagState(GbCpuFlags::HalfCarry, ((_state.A ^ value ^ result) & 0x10) != 0);
	SetFlagState(GbCpuFlags::Zero, (uint8_t)result == 0);
	SetFlag(GbCpuFlags::AddSub);
}

void GbCpu::NOP()
{

}

void GbCpu::InvalidOp()
{
#ifndef DUMMYCPU
	//Disable all IRQs
	_memoryManager->Write(0xFFFF, 0);
#endif

	//Halt CPU to lock it up permanently
	_state.HaltCounter = 1;
}

void GbCpu::STOP()
{
	//TODOGB some stop-related quirks aren't implemented yet
	//See: https://gbdev.io/pandocs/Reducing_Power_Consumption.html#the-bizarre-case-of-the-game-boy-stop-instruction-before-even-considering-timing

	//Skip the next byte (unused operand)
	ReadCode();

	if(_gameboy->IsCgb() && _memoryManager->GetState().CgbSwitchSpeedRequest) {
#ifndef DUMMYCPU
		//Stop for ~33941 cycles - most likely not accurate, but matches speed_switch_timing_stat test rom
		_state.HaltCounter = 33942;
#endif
	} else {
		_state.Stopped = true;
		_state.HaltCounter = 1;
#ifndef DUMMYCPU
		_ppu->SetCpuStopState(true);
#endif
	}
}

void GbCpu::HALT()
{
	if(_state.IME || _memoryManager->ProcessIrqRequests() == 0) {
		_state.HaltCounter = 1;
	} else {
		//HALT bug, execution continues, but PC isn't incremented for the first byte
		_state.HaltCounter = 1;
		_state.HaltBug = true;
	}
}

// cpl              2F         4 -11- A = A xor FF
void GbCpu::CPL()
{
	_state.A ^= 0xFF;
	SetFlag(GbCpuFlags::AddSub);
	SetFlag(GbCpuFlags::HalfCarry);
}

//rl   r         CB 1x        8 z00c rotate left through carry
void GbCpu::RL(uint8_t& dst)
{
	uint8_t carry = (uint8_t)CheckFlag(GbCpuFlags::Carry);
	SetFlagState(GbCpuFlags::Carry, (dst & 0x80) != 0);
	dst = (dst << 1) | carry;

	SetFlagState(GbCpuFlags::Zero, dst == 0);
	ClearFlag(GbCpuFlags::AddSub);
	ClearFlag(GbCpuFlags::HalfCarry);
}

//rl   (HL)      CB 16       16 z00c rotate left through carry
void GbCpu::RL_Indirect(uint16_t addr)
{
	uint8_t val = Read(addr);
	RL(val);
	Write(addr, val);
}

//rlc  r         CB 0x        8 z00c rotate left
void GbCpu::RLC(uint8_t& dst)
{
	SetFlagState(GbCpuFlags::Carry, (dst & 0x80) != 0);
	dst = (dst << 1) | ((dst & 0x80) >> 7);

	SetFlagState(GbCpuFlags::Zero, dst == 0);
	ClearFlag(GbCpuFlags::AddSub);
	ClearFlag(GbCpuFlags::HalfCarry);
}

//rlc  (HL)      CB 06       16 z00c rotate left
void GbCpu::RLC_Indirect(uint16_t addr)
{
	uint8_t val = Read(addr);
	RLC(val);
	Write(addr, val);
}

//rr   r         CB 1x        8 z00c rotate right through carry
void GbCpu::RR(uint8_t& dst)
{
	uint8_t carry = (uint8_t)CheckFlag(GbCpuFlags::Carry) << 7;
	SetFlagState(GbCpuFlags::Carry, (dst & 0x01) != 0);
	dst = (dst >> 1) | carry;

	SetFlagState(GbCpuFlags::Zero, dst == 0);
	ClearFlag(GbCpuFlags::AddSub);
	ClearFlag(GbCpuFlags::HalfCarry);
}

//rr   (HL)      CB 1E       16 z00c rotate right through carry
void GbCpu::RR_Indirect(uint16_t addr)
{
	uint8_t val = Read(addr);
	RR(val);
	Write(addr, val);
}

//rrc  r         CB 0x        8 z00c rotate right
void GbCpu::RRC(uint8_t& dst)
{
	SetFlagState(GbCpuFlags::Carry, (dst & 0x01) != 0);
	dst = (dst >> 1) | ((dst & 0x01) << 7);

	SetFlagState(GbCpuFlags::Zero, dst == 0);
	ClearFlag(GbCpuFlags::AddSub);
	ClearFlag(GbCpuFlags::HalfCarry);
}

//rrc  (HL)      CB 0E       16 z00c rotate right
void GbCpu::RRC_Indirect(uint16_t addr)
{
	uint8_t val = Read(addr);
	RRC(val);
	Write(addr, val);
}

//rra            1F           4 000c rotate akku right through carry
void GbCpu::RRA()
{
	RR(_state.A);
	ClearFlag(GbCpuFlags::Zero);
}

//rrca           0F           4 000c rotate akku right
void GbCpu::RRCA()
{
	RRC(_state.A);
	ClearFlag(GbCpuFlags::Zero);
}

//rlca           07           4 000c rotate akku left
void GbCpu::RLCA()
{
	RLC(_state.A);
	ClearFlag(GbCpuFlags::Zero);
}

//rla            17           4 000c rotate akku left through carry 
void GbCpu::RLA()
{
	RL(_state.A);
	ClearFlag(GbCpuFlags::Zero);
}

//srl  r         CB 3x        8 z00c shift right logical (b7=0)
void GbCpu::SRL(uint8_t& dst)
{
	SetFlagState(GbCpuFlags::Carry, (dst & 0x01) != 0);
	dst = (dst >> 1);

	SetFlagState(GbCpuFlags::Zero, dst == 0);
	ClearFlag(GbCpuFlags::AddSub);
	ClearFlag(GbCpuFlags::HalfCarry);
}

//srl  (HL)      CB 3E       16 z00c shift right logical (b7=0)
void GbCpu::SRL_Indirect(uint16_t addr)
{
	uint8_t val = Read(addr);
	SRL(val);
	Write(addr, val);
}

//sra  r         CB 2x        8 z00c shift right arithmetic (b7=b7)
void GbCpu::SRA(uint8_t& dst)
{
	SetFlagState(GbCpuFlags::Carry, (dst & 0x01) != 0);
	dst = (dst & 0x80) | (dst >> 1);

	SetFlagState(GbCpuFlags::Zero, dst == 0);
	ClearFlag(GbCpuFlags::AddSub);
	ClearFlag(GbCpuFlags::HalfCarry);
}

//sra  (HL)      CB 2E       16 z00c shift right arithmetic (b7=b7)
void GbCpu::SRA_Indirect(uint16_t addr)
{
	uint8_t val = Read(addr);
	SRA(val);
	Write(addr, val);
}

//sla  r         CB 2x        8 z00c shift left arithmetic (b0=0)
void GbCpu::SLA(uint8_t& dst)
{
	SetFlagState(GbCpuFlags::Carry, (dst & 0x80) != 0);
	dst = (dst << 1);

	SetFlagState(GbCpuFlags::Zero, dst == 0);
	ClearFlag(GbCpuFlags::AddSub);
	ClearFlag(GbCpuFlags::HalfCarry);
}

//sla  (HL)      CB 26       16 z00c shift left arithmetic (b0=0)
void GbCpu::SLA_Indirect(uint16_t addr)
{
	uint8_t val = Read(addr);
	SLA(val);
	Write(addr, val);
}

//swap r         CB 3x        8 z000 exchange low/hi-nibble
void GbCpu::SWAP(uint8_t& dst)
{
	dst = ((dst & 0x0F) << 4) | (dst >> 4);

	SetFlagState(GbCpuFlags::Zero, dst == 0);
	ClearFlag(GbCpuFlags::AddSub);
	ClearFlag(GbCpuFlags::Carry);
	ClearFlag(GbCpuFlags::HalfCarry);
}

//swap (HL)      CB 36       16 z000 exchange low/hi-nibble
void GbCpu::SWAP_Indirect(uint16_t addr)
{
	uint8_t val = Read(addr);
	SWAP(val);
	Write(addr, val);
}

template<uint8_t bit>
void GbCpu::BIT(uint8_t src)
{
	SetFlagState(GbCpuFlags::Zero, (src & (1 << bit)) == 0);
	ClearFlag(GbCpuFlags::AddSub);
	SetFlag(GbCpuFlags::HalfCarry);
}

template<uint8_t bit>
void GbCpu::RES(uint8_t& dst)
{
	dst &= ~(1 << bit);
}

template<uint8_t bit>
void GbCpu::RES_Indirect(uint16_t addr)
{
	uint8_t val = Read(addr);
	RES<bit>(val);
	Write(addr, val);
}

template<uint8_t bit>
void GbCpu::SET(uint8_t& dst)
{
	dst |= (1 << bit);
}

template<uint8_t bit>
void GbCpu::SET_Indirect(uint16_t addr)
{
	uint8_t val = Read(addr);
	SET<bit>(val);
	Write(addr, val);
}

// daa              27         4 z-0x decimal adjust akku
void GbCpu::DAA()
{
	if(CheckFlag(GbCpuFlags::AddSub)) {
		if(CheckFlag(GbCpuFlags::Carry)) {
			_state.A -= 0x60;
		}
		if(CheckFlag(GbCpuFlags::HalfCarry)) {
			_state.A -= 0x06;
		}
	} else {
		if(CheckFlag(GbCpuFlags::Carry) || _state.A > 0x99) {
			_state.A += 0x60;
			SetFlag(GbCpuFlags::Carry);
		}
		if(CheckFlag(GbCpuFlags::HalfCarry) || (_state.A & 0x0F) > 0x09) {
			_state.A += 0x6;
		}
	}

	SetFlagState(GbCpuFlags::Zero, _state.A == 0);
	ClearFlag(GbCpuFlags::HalfCarry);
}

//jp   nn        C3 nn nn    16 ---- jump to nn, PC=nn
void GbCpu::JP(uint16_t dstAddr)
{
	_state.PC = dstAddr;
	ExecCpuCycle();
}

//jp   HL        E9           4 ---- jump to HL, PC=HL
void GbCpu::JP_HL()
{
	_state.PC = _regHL;
}

//jp   f,nn      xx nn nn 16;12 ---- conditional jump if nz,z,nc,c
void GbCpu::JP(bool condition, uint16_t dstAddr)
{
	if(condition) {
		_state.PC = dstAddr;
		ExecCpuCycle();
	}
}

//jr   PC+dd     18 dd       12 ---- relative jump to nn (PC=PC+/-7bit)
void GbCpu::JR(int8_t offset)
{
	_state.PC += offset;
	ExecCpuCycle();
}

//jr   f,PC+dd   xx dd     12;8 ---- conditional relative jump if nz,z,nc,c 
void GbCpu::JR(bool condition, int8_t offset)
{
	if(condition) {
		_state.PC += offset;
		ExecCpuCycle();
	}
}

//call nn        CD nn nn    24 ---- call to nn, SP=SP-2, (SP)=PC, PC=nn
void GbCpu::CALL(uint16_t dstAddr)
{
	ExecCpuCycle();
	PushWord(_state.PC);
	_state.PC = dstAddr;
}

//call f,nn      xx nn nn 24;12 ---- conditional call if nz,z,nc,c
void GbCpu::CALL(bool condition, uint16_t dstAddr)
{
	if(condition) {
		ExecCpuCycle();
		PushWord(_state.PC);
		_state.PC = dstAddr;
	}
}

//ret            C9          16 ---- return, PC=(SP), SP=SP+2
void GbCpu::RET()
{
	_state.PC = PopWord();
	ExecCpuCycle();
}

//ret  f         xx        20;8 ---- conditional return if nz,z,nc,c
void GbCpu::RET(bool condition)
{
	ExecCpuCycle();
	if(condition) {
		_state.PC = PopWord();
		ExecCpuCycle();
	}
}

//reti           D9          16 ---- return and enable interrupts (IME=1)
void GbCpu::RETI()
{
	_state.PC = PopWord();
	_state.IME = true;
	ExecCpuCycle();
}

//rst  n         xx          16 ---- call to 00,08,10,18,20,28,30,38
void GbCpu::RST(uint8_t value)
{
	ExecCpuCycle();
	PushWord(_state.PC);
	_state.PC = value;
}

void GbCpu::POP(Register16& reg)
{
	reg.Write(PopWord());
}

void GbCpu::PUSH(Register16& reg)
{
	ExecCpuCycle();
	PushWord(reg);
}

void GbCpu::POP_AF()
{
	_regAF.Write(PopWord() & 0xFFF0);
}

//scf            37           4 -001 cy=1 
void GbCpu::SCF()
{
	SetFlag(GbCpuFlags::Carry);
	ClearFlag(GbCpuFlags::AddSub);
	ClearFlag(GbCpuFlags::HalfCarry);
}

//ccf            3F           4 -00c cy=cy xor 1
void GbCpu::CCF()
{
	_state.Flags ^= GbCpuFlags::Carry;
	ClearFlag(GbCpuFlags::AddSub);
	ClearFlag(GbCpuFlags::HalfCarry);
}

void GbCpu::EI()
{
	_state.EiPending = true;
}

void GbCpu::DI()
{
	_state.IME = false;
}

void GbCpu::PREFIX()
{
	switch(ReadCode()) {
		case 0x00: RLC(_state.B); break;
		case 0x01: RLC(_state.C); break;
		case 0x02: RLC(_state.D); break;
		case 0x03: RLC(_state.E); break;
		case 0x04: RLC(_state.H); break;
		case 0x05: RLC(_state.L); break;
		case 0x06: RLC_Indirect(_regHL); break;
		case 0x07: RLC(_state.A); break;
		case 0x08: RRC(_state.B); break;
		case 0x09: RRC(_state.C); break;
		case 0x0A: RRC(_state.D); break;
		case 0x0B: RRC(_state.E); break;
		case 0x0C: RRC(_state.H); break;
		case 0x0D: RRC(_state.L); break;
		case 0x0E: RRC_Indirect(_regHL); break;
		case 0x0F: RRC(_state.A); break;
		case 0x10: RL(_state.B); break;
		case 0x11: RL(_state.C); break;
		case 0x12: RL(_state.D); break;
		case 0x13: RL(_state.E); break;
		case 0x14: RL(_state.H); break;
		case 0x15: RL(_state.L); break;
		case 0x16: RL_Indirect(_regHL); break;
		case 0x17: RL(_state.A); break;
		case 0x18: RR(_state.B); break;
		case 0x19: RR(_state.C); break;
		case 0x1A: RR(_state.D); break;
		case 0x1B: RR(_state.E); break;
		case 0x1C: RR(_state.H); break;
		case 0x1D: RR(_state.L); break;
		case 0x1E: RR_Indirect(_regHL); break;
		case 0x1F: RR(_state.A); break;
		case 0x20: SLA(_state.B); break;
		case 0x21: SLA(_state.C); break;
		case 0x22: SLA(_state.D); break;
		case 0x23: SLA(_state.E); break;
		case 0x24: SLA(_state.H); break;
		case 0x25: SLA(_state.L); break;
		case 0x26: SLA_Indirect(_regHL); break;
		case 0x27: SLA(_state.A); break;
		case 0x28: SRA(_state.B); break;
		case 0x29: SRA(_state.C); break;
		case 0x2A: SRA(_state.D); break;
		case 0x2B: SRA(_state.E); break;
		case 0x2C: SRA(_state.H); break;
		case 0x2D: SRA(_state.L); break;
		case 0x2E: SRA_Indirect(_regHL); break;
		case 0x2F: SRA(_state.A); break;
		case 0x30: SWAP(_state.B); break;
		case 0x31: SWAP(_state.C); break;
		case 0x32: SWAP(_state.D); break;
		case 0x33: SWAP(_state.E); break;
		case 0x34: SWAP(_state.H); break;
		case 0x35: SWAP(_state.L); break;
		case 0x36: SWAP_Indirect(_regHL); break;
		case 0x37: SWAP(_state.A); break;
		case 0x38: SRL(_state.B); break;
		case 0x39: SRL(_state.C); break;
		case 0x3A: SRL(_state.D); break;
		case 0x3B: SRL(_state.E); break;
		case 0x3C: SRL(_state.H); break;
		case 0x3D: SRL(_state.L); break;
		case 0x3E: SRL_Indirect(_regHL); break;
		case 0x3F: SRL(_state.A); break;
		case 0x40: BIT<0>(_state.B); break;
		case 0x41: BIT<0>(_state.C); break;
		case 0x42: BIT<0>(_state.D); break;
		case 0x43: BIT<0>(_state.E); break;
		case 0x44: BIT<0>(_state.H); break;
		case 0x45: BIT<0>(_state.L); break;
		case 0x46: BIT<0>(Read(_regHL)); break;
		case 0x47: BIT<0>(_state.A); break;
		case 0x48: BIT<1>(_state.B); break;
		case 0x49: BIT<1>(_state.C); break;
		case 0x4A: BIT<1>(_state.D); break;
		case 0x4B: BIT<1>(_state.E); break;
		case 0x4C: BIT<1>(_state.H); break;
		case 0x4D: BIT<1>(_state.L); break;
		case 0x4E: BIT<1>(Read(_regHL)); break;
		case 0x4F: BIT<1>(_state.A); break;
		case 0x50: BIT<2>(_state.B); break;
		case 0x51: BIT<2>(_state.C); break;
		case 0x52: BIT<2>(_state.D); break;
		case 0x53: BIT<2>(_state.E); break;
		case 0x54: BIT<2>(_state.H); break;
		case 0x55: BIT<2>(_state.L); break;
		case 0x56: BIT<2>(Read(_regHL)); break;
		case 0x57: BIT<2>(_state.A); break;
		case 0x58: BIT<3>(_state.B); break;
		case 0x59: BIT<3>(_state.C); break;
		case 0x5A: BIT<3>(_state.D); break;
		case 0x5B: BIT<3>(_state.E); break;
		case 0x5C: BIT<3>(_state.H); break;
		case 0x5D: BIT<3>(_state.L); break;
		case 0x5E: BIT<3>(Read(_regHL)); break;
		case 0x5F: BIT<3>(_state.A); break;
		case 0x60: BIT<4>(_state.B); break;
		case 0x61: BIT<4>(_state.C); break;
		case 0x62: BIT<4>(_state.D); break;
		case 0x63: BIT<4>(_state.E); break;
		case 0x64: BIT<4>(_state.H); break;
		case 0x65: BIT<4>(_state.L); break;
		case 0x66: BIT<4>(Read(_regHL)); break;
		case 0x67: BIT<4>(_state.A); break;
		case 0x68: BIT<5>(_state.B); break;
		case 0x69: BIT<5>(_state.C); break;
		case 0x6A: BIT<5>(_state.D); break;
		case 0x6B: BIT<5>(_state.E); break;
		case 0x6C: BIT<5>(_state.H); break;
		case 0x6D: BIT<5>(_state.L); break;
		case 0x6E: BIT<5>(Read(_regHL)); break;
		case 0x6F: BIT<5>(_state.A); break;
		case 0x70: BIT<6>(_state.B); break;
		case 0x71: BIT<6>(_state.C); break;
		case 0x72: BIT<6>(_state.D); break;
		case 0x73: BIT<6>(_state.E); break;
		case 0x74: BIT<6>(_state.H); break;
		case 0x75: BIT<6>(_state.L); break;
		case 0x76: BIT<6>(Read(_regHL)); break;
		case 0x77: BIT<6>(_state.A); break;
		case 0x78: BIT<7>(_state.B); break;
		case 0x79: BIT<7>(_state.C); break;
		case 0x7A: BIT<7>(_state.D); break;
		case 0x7B: BIT<7>(_state.E); break;
		case 0x7C: BIT<7>(_state.H); break;
		case 0x7D: BIT<7>(_state.L); break;
		case 0x7E: BIT<7>(Read(_regHL)); break;
		case 0x7F: BIT<7>(_state.A); break;
		case 0x80: RES<0>(_state.B); break;
		case 0x81: RES<0>(_state.C); break;
		case 0x82: RES<0>(_state.D); break;
		case 0x83: RES<0>(_state.E); break;
		case 0x84: RES<0>(_state.H); break;
		case 0x85: RES<0>(_state.L); break;
		case 0x86: RES_Indirect<0>(_regHL); break;
		case 0x87: RES<0>(_state.A); break;
		case 0x88: RES<1>(_state.B); break;
		case 0x89: RES<1>(_state.C); break;
		case 0x8A: RES<1>(_state.D); break;
		case 0x8B: RES<1>(_state.E); break;
		case 0x8C: RES<1>(_state.H); break;
		case 0x8D: RES<1>(_state.L); break;
		case 0x8E: RES_Indirect<1>(_regHL); break;
		case 0x8F: RES<1>(_state.A); break;
		case 0x90: RES<2>(_state.B); break;
		case 0x91: RES<2>(_state.C); break;
		case 0x92: RES<2>(_state.D); break;
		case 0x93: RES<2>(_state.E); break;
		case 0x94: RES<2>(_state.H); break;
		case 0x95: RES<2>(_state.L); break;
		case 0x96: RES_Indirect<2>(_regHL); break;
		case 0x97: RES<2>(_state.A); break;
		case 0x98: RES<3>(_state.B); break;
		case 0x99: RES<3>(_state.C); break;
		case 0x9A: RES<3>(_state.D); break;
		case 0x9B: RES<3>(_state.E); break;
		case 0x9C: RES<3>(_state.H); break;
		case 0x9D: RES<3>(_state.L); break;
		case 0x9E: RES_Indirect<3>(_regHL); break;
		case 0x9F: RES<3>(_state.A); break;
		case 0xA0: RES<4>(_state.B); break;
		case 0xA1: RES<4>(_state.C); break;
		case 0xA2: RES<4>(_state.D); break;
		case 0xA3: RES<4>(_state.E); break;
		case 0xA4: RES<4>(_state.H); break;
		case 0xA5: RES<4>(_state.L); break;
		case 0xA6: RES_Indirect<4>(_regHL); break;
		case 0xA7: RES<4>(_state.A); break;
		case 0xA8: RES<5>(_state.B); break;
		case 0xA9: RES<5>(_state.C); break;
		case 0xAA: RES<5>(_state.D); break;
		case 0xAB: RES<5>(_state.E); break;
		case 0xAC: RES<5>(_state.H); break;
		case 0xAD: RES<5>(_state.L); break;
		case 0xAE: RES_Indirect<5>(_regHL); break;
		case 0xAF: RES<5>(_state.A); break;
		case 0xB0: RES<6>(_state.B); break;
		case 0xB1: RES<6>(_state.C); break;
		case 0xB2: RES<6>(_state.D); break;
		case 0xB3: RES<6>(_state.E); break;
		case 0xB4: RES<6>(_state.H); break;
		case 0xB5: RES<6>(_state.L); break;
		case 0xB6: RES_Indirect<6>(_regHL); break;
		case 0xB7: RES<6>(_state.A); break;
		case 0xB8: RES<7>(_state.B); break;
		case 0xB9: RES<7>(_state.C); break;
		case 0xBA: RES<7>(_state.D); break;
		case 0xBB: RES<7>(_state.E); break;
		case 0xBC: RES<7>(_state.H); break;
		case 0xBD: RES<7>(_state.L); break;
		case 0xBE: RES_Indirect<7>(_regHL); break;
		case 0xBF: RES<7>(_state.A); break;
		case 0xC0: SET<0>(_state.B); break;
		case 0xC1: SET<0>(_state.C); break;
		case 0xC2: SET<0>(_state.D); break;
		case 0xC3: SET<0>(_state.E); break;
		case 0xC4: SET<0>(_state.H); break;
		case 0xC5: SET<0>(_state.L); break;
		case 0xC6: SET_Indirect<0>(_regHL); break;
		case 0xC7: SET<0>(_state.A); break;
		case 0xC8: SET<1>(_state.B); break;
		case 0xC9: SET<1>(_state.C); break;
		case 0xCA: SET<1>(_state.D); break;
		case 0xCB: SET<1>(_state.E); break;
		case 0xCC: SET<1>(_state.H); break;
		case 0xCD: SET<1>(_state.L); break;
		case 0xCE: SET_Indirect<1>(_regHL); break;
		case 0xCF: SET<1>(_state.A); break;
		case 0xD0: SET<2>(_state.B); break;
		case 0xD1: SET<2>(_state.C); break;
		case 0xD2: SET<2>(_state.D); break;
		case 0xD3: SET<2>(_state.E); break;
		case 0xD4: SET<2>(_state.H); break;
		case 0xD5: SET<2>(_state.L); break;
		case 0xD6: SET_Indirect<2>(_regHL); break;
		case 0xD7: SET<2>(_state.A); break;
		case 0xD8: SET<3>(_state.B); break;
		case 0xD9: SET<3>(_state.C); break;
		case 0xDA: SET<3>(_state.D); break;
		case 0xDB: SET<3>(_state.E); break;
		case 0xDC: SET<3>(_state.H); break;
		case 0xDD: SET<3>(_state.L); break;
		case 0xDE: SET_Indirect<3>(_regHL); break;
		case 0xDF: SET<3>(_state.A); break;
		case 0xE0: SET<4>(_state.B); break;
		case 0xE1: SET<4>(_state.C); break;
		case 0xE2: SET<4>(_state.D); break;
		case 0xE3: SET<4>(_state.E); break;
		case 0xE4: SET<4>(_state.H); break;
		case 0xE5: SET<4>(_state.L); break;
		case 0xE6: SET_Indirect<4>(_regHL); break;
		case 0xE7: SET<4>(_state.A); break;
		case 0xE8: SET<5>(_state.B); break;
		case 0xE9: SET<5>(_state.C); break;
		case 0xEA: SET<5>(_state.D); break;
		case 0xEB: SET<5>(_state.E); break;
		case 0xEC: SET<5>(_state.H); break;
		case 0xED: SET<5>(_state.L); break;
		case 0xEE: SET_Indirect<5>(_regHL); break;
		case 0xEF: SET<5>(_state.A); break;
		case 0xF0: SET<6>(_state.B); break;
		case 0xF1: SET<6>(_state.C); break;
		case 0xF2: SET<6>(_state.D); break;
		case 0xF3: SET<6>(_state.E); break;
		case 0xF4: SET<6>(_state.H); break;
		case 0xF5: SET<6>(_state.L); break;
		case 0xF6: SET_Indirect<6>(_regHL); break;
		case 0xF7: SET<6>(_state.A); break;
		case 0xF8: SET<7>(_state.B); break;
		case 0xF9: SET<7>(_state.C); break;
		case 0xFA: SET<7>(_state.D); break;
		case 0xFB: SET<7>(_state.E); break;
		case 0xFC: SET<7>(_state.H); break;
		case 0xFD: SET<7>(_state.L); break;
		case 0xFE: SET_Indirect<7>(_regHL); break;
		case 0xFF: SET<7>(_state.A); break;
	}
}

void GbCpu::Serialize(Serializer& s)
{
	SV(_state.PC); SV(_state.SP); SV(_state.A); SV(_state.Flags); SV(_state.B);
	SV(_state.C); SV(_state.D); SV(_state.E); SV(_state.H); SV(_state.L); SV(_state.IME); SV(_state.HaltCounter);
	SV(_state.EiPending);
	SV(_state.CycleCount);
	SV(_state.HaltBug);
	SV(_state.Stopped);
	SV(_prevIrqVector);
}
