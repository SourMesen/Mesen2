#include "pch.h"
#include "SMS/SmsCpu.h"
#include "SMS/SmsConsole.h"
#include "SMS/SmsTypes.h"
#include "SMS/SmsMemoryManager.h"
#include "Shared/Emulator.h"
#include "Shared/MemoryOperationType.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/Serializer.h"

SmsCpuParityTable SmsCpu::_parity = {};

void SmsCpu::Init(Emulator* emu, SmsConsole* console, SmsMemoryManager* memoryManager)
{
	_emu = emu;
	_console = console;
	_memoryManager = memoryManager;

	_state = {};

	_state.PC = 0;

	if(!_console->HasBios()) {
		InitPostBiosState();
	}
}

SmsCpuState& SmsCpu::GetState()
{
	return _state;
}

void SmsCpu::Exec()
{
	uint8_t opCode = 0;
	_state.FlagsChanged <<= 1;
	if(_state.Halted) {
		_emu->ProcessHaltedCpu<CpuType::Sms>();
		ExecCycles(4);
	} else {
		#ifndef DUMMYCPU
		_emu->ProcessInstruction<CpuType::Sms>();
		#endif
		opCode = ReadOpCode();
		ExecOpCode<0>(opCode);
	}

	if(_state.NmiPending) {
		_state.Halted = false;
		_state.NmiPending = false;
		_state.IFF1 = false;
		ExecCycles(4);
		IncrementR();
		uint16_t originalPc = _state.PC;
		RST(0x66);
		_emu->ProcessInterrupt<CpuType::Sms>(originalPc, _state.PC, true);
	} else if(_state.IFF1 && _state.ActiveIrqs && opCode != 0xFB) {
		//Process IRQs if enabled, but not if the previous op was EI (0xFB)
		_state.Halted = false;
		_state.IFF1 = false;
		_state.IFF2 = false;
		ExecCycles(6);
		IncrementR();
		uint16_t originalPc = _state.PC;
		if(_state.IM == 2) {
			ExecCycles(1);
			uint16_t addr = (_state.I << 8) | _memoryManager->GetOpenBus();
			uint8_t lsb = Read(addr);
			uint8_t msb = Read(addr + 1);
			PushWord(_state.PC);
			_state.PC = (msb << 8) | lsb;
			_state.WZ = _state.PC;
		} else {
			//TODOSMS interrupt mode 0 is not implemented
			RST(0x38);
		}
		_console->RefreshRamCheats();
		_emu->ProcessInterrupt<CpuType::Sms>(originalPc, _state.PC, false);
	}
}

template<uint8_t prefix>
void SmsCpu::ExecOpCode(uint8_t opCode)
{
	auto getHl = [this]() constexpr -> Register16& { 
		switch(prefix) {
			default: return _regHL;
			case 0xDD: return _regIX;
			case 0xFD: return _regIY;
		}
	};

	auto getIndirectHl = [this]() constexpr -> uint16_t {
		switch(prefix) {
			default: return _regHL;
			case 0xDD: {
				uint16_t value = _regIX + (int8_t)ReadCode();
				ExecCycles(5);
				return value;
			}
			case 0xFD: {
				uint16_t value = _regIY + (int8_t)ReadCode();
				ExecCycles(5);
				return value;
			}
		}
	};

	auto getH = [this]() constexpr -> uint8_t& {
		switch(prefix) {
			default: return _state.H;
			case 0xDD: return _state.IXH;
			case 0xFD: return _state.IYH;
		}
	};

	auto getL = [this]() constexpr -> uint8_t& {
		switch(prefix) {
			default: return _state.L;
			case 0xDD: return _state.IXL;
			case 0xFD: return _state.IYL;
		}
	};

	switch(opCode) {
		case 0x00: NOP(); break;
		case 0x01: LD(_regBC, ReadCodeWord()); break;
		case 0x02: LD_Indirect_A(_regBC); break;
		case 0x03: INC(_regBC); break;
		case 0x04: INC(_state.B); break;
		case 0x05: DEC(_state.B); break;
		case 0x06: LD(_state.B, ReadCode()); break;
		case 0x07: RLCA(); break;
		case 0x08: ExchangeAf(); break;
		case 0x09: ADD(getHl(), _regBC); break;
		case 0x0A: LD(_state.A, Read(_regBC)); _state.WZ = _regBC + 1;  break;
		case 0x0B: DEC(_regBC); break;
		case 0x0C: INC(_state.C); break;
		case 0x0D: DEC(_state.C); break;
		case 0x0E: LD(_state.C, ReadCode()); break;
		case 0x0F: RRCA(); break;
		case 0x10: DJNZ();  break;
		case 0x11: LD(_regDE, ReadCodeWord()); break;
		case 0x12: LD_Indirect_A(_regDE); break;
		case 0x13: INC(_regDE); break;
		case 0x14: INC(_state.D); break;
		case 0x15: DEC(_state.D); break;
		case 0x16: LD(_state.D, ReadCode()); break;
		case 0x17: RLA(); break;
		case 0x18: JR(ReadCode()); break;
		case 0x19: ADD(getHl(), _regDE); break;
		case 0x1A: LD(_state.A, Read(_regDE)); _state.WZ = _regDE + 1;  break;
		case 0x1B: DEC(_regDE); break;
		case 0x1C: INC(_state.E); break;
		case 0x1D: DEC(_state.E); break;
		case 0x1E: LD(_state.E, ReadCode()); break;
		case 0x1F: RRA(); break;
		case 0x20: JR((_state.Flags & SmsCpuFlags::Zero) == 0, ReadCode()); break;
		case 0x21: LD(getHl(), ReadCodeWord()); break;
		case 0x22: LD_Indirect16(ReadCodeWord(), getHl()); break;
		case 0x23: INC(getHl()); break;
		case 0x24: INC(getH()); break;
		case 0x25: DEC(getH()); break;
		case 0x26: LD(getH(), ReadCode()); break;
		case 0x27: DAA(); break;
		case 0x28: JR((_state.Flags & SmsCpuFlags::Zero) != 0, ReadCode()); break;
		case 0x29: ADD(getHl(), getHl()); break;
		case 0x2A: LDReg_Indirect16(getHl(), ReadCodeWord()); break;
		case 0x2B: DEC(getHl()); break;
		case 0x2C: INC(getL()); break;
		case 0x2D: DEC(getL()); break;
		case 0x2E: LD(getL(), ReadCode()); break;
		case 0x2F: CPL(); break;
		case 0x30: JR((_state.Flags & SmsCpuFlags::Carry) == 0, ReadCode()); break;
		case 0x31: LD(_state.SP, ReadCodeWord()); break;
		case 0x32: LD_Indirect_A(ReadCodeWord()); break;
		case 0x33: INC_SP(); break;
		case 0x34: INC_Indirect(getIndirectHl()); break;
		case 0x35: DEC_Indirect(getIndirectHl()); break;
		case 0x36: LD_IndirectImm(getIndirectHl()); break;
		case 0x37: SCF(); break;
		case 0x38: JR((_state.Flags & SmsCpuFlags::Carry) != 0, ReadCode()); break;
		case 0x39: ADD(getHl(), _state.SP); break;
		case 0x3A: LDA_Address(ReadCodeWord()); break;
		case 0x3B: DEC_SP(); break;
		case 0x3C: INC(_state.A); break;
		case 0x3D: DEC(_state.A); break;
		case 0x3E: LD(_state.A, ReadCode()); break;
		case 0x3F: CCF(); break;
		case 0x40: LD(_state.B, _state.B); break;
		case 0x41: LD(_state.B, _state.C); break;
		case 0x42: LD(_state.B, _state.D); break;
		case 0x43: LD(_state.B, _state.E); break;
		case 0x44: LD(_state.B, getH()); break;
		case 0x45: LD(_state.B, getL()); break;
		case 0x46: LD(_state.B, Read(getIndirectHl())); break;
		case 0x47: LD(_state.B, _state.A); break;
		case 0x48: LD(_state.C, _state.B); break;
		case 0x49: LD(_state.C, _state.C); break;
		case 0x4A: LD(_state.C, _state.D); break;
		case 0x4B: LD(_state.C, _state.E); break;
		case 0x4C: LD(_state.C, getH()); break;
		case 0x4D: LD(_state.C, getL()); break;
		case 0x4E: LD(_state.C, Read(getIndirectHl())); break;
		case 0x4F: LD(_state.C, _state.A); break;
		case 0x50: LD(_state.D, _state.B); break;
		case 0x51: LD(_state.D, _state.C); break;
		case 0x52: LD(_state.D, _state.D); break;
		case 0x53: LD(_state.D, _state.E); break;
		case 0x54: LD(_state.D, getH()); break;
		case 0x55: LD(_state.D, getL()); break;
		case 0x56: LD(_state.D, Read(getIndirectHl())); break;
		case 0x57: LD(_state.D, _state.A); break;
		case 0x58: LD(_state.E, _state.B); break;
		case 0x59: LD(_state.E, _state.C); break;
		case 0x5A: LD(_state.E, _state.D); break;
		case 0x5B: LD(_state.E, _state.E); break;
		case 0x5C: LD(_state.E, getH()); break;
		case 0x5D: LD(_state.E, getL()); break;
		case 0x5E: LD(_state.E, Read(getIndirectHl())); break;
		case 0x5F: LD(_state.E, _state.A); break;
		case 0x60: LD(getH(), _state.B); break;
		case 0x61: LD(getH(), _state.C); break;
		case 0x62: LD(getH(), _state.D); break;
		case 0x63: LD(getH(), _state.E); break;
		case 0x64: LD(getH(), getH()); break;
		case 0x65: LD(getH(), getL()); break;
		case 0x66: LD(_state.H, Read(getIndirectHl())); break;
		case 0x67: LD(getH(), _state.A); break;
		case 0x68: LD(getL(), _state.B); break;
		case 0x69: LD(getL(), _state.C); break;
		case 0x6A: LD(getL(), _state.D); break;
		case 0x6B: LD(getL(), _state.E); break;
		case 0x6C: LD(getL(), getH()); break;
		case 0x6D: LD(getL(), getL()); break;
		case 0x6E: LD(_state.L, Read(getIndirectHl())); break;
		case 0x6F: LD(getL(), _state.A); break;
		case 0x70: LD_Indirect(getIndirectHl(), _state.B); break;
		case 0x71: LD_Indirect(getIndirectHl(), _state.C); break;
		case 0x72: LD_Indirect(getIndirectHl(), _state.D); break;
		case 0x73: LD_Indirect(getIndirectHl(), _state.E); break;
		case 0x74: LD_Indirect(getIndirectHl(), _state.H); break;
		case 0x75: LD_Indirect(getIndirectHl(), _state.L); break;
		case 0x76: HALT(); break;
		case 0x77: LD_Indirect(getIndirectHl(), _state.A); break;
		case 0x78: LD(_state.A, _state.B); break;
		case 0x79: LD(_state.A, _state.C); break;
		case 0x7A: LD(_state.A, _state.D); break;
		case 0x7B: LD(_state.A, _state.E); break;
		case 0x7C: LD(_state.A, getH()); break;
		case 0x7D: LD(_state.A, getL()); break;
		case 0x7E: LD(_state.A, Read(getIndirectHl())); break;
		case 0x7F: LD(_state.A, _state.A); break;
		case 0x80: ADD(_state.B); break;
		case 0x81: ADD(_state.C); break;
		case 0x82: ADD(_state.D); break;
		case 0x83: ADD(_state.E); break;
		case 0x84: ADD(getH()); break;
		case 0x85: ADD(getL()); break;
		case 0x86: ADD(Read(getIndirectHl())); break;
		case 0x87: ADD(_state.A); break;
		case 0x88: ADC(_state.B); break;
		case 0x89: ADC(_state.C); break;
		case 0x8A: ADC(_state.D); break;
		case 0x8B: ADC(_state.E); break;
		case 0x8C: ADC(getH()); break;
		case 0x8D: ADC(getL()); break;
		case 0x8E: ADC(Read(getIndirectHl())); break;
		case 0x8F: ADC(_state.A); break;
		case 0x90: SUB(_state.B); break;
		case 0x91: SUB(_state.C); break;
		case 0x92: SUB(_state.D); break;
		case 0x93: SUB(_state.E); break;
		case 0x94: SUB(getH()); break;
		case 0x95: SUB(getL()); break;
		case 0x96: SUB(Read(getIndirectHl())); break;
		case 0x97: SUB(_state.A); break;
		case 0x98: SBC(_state.B); break;
		case 0x99: SBC(_state.C); break;
		case 0x9A: SBC(_state.D); break;
		case 0x9B: SBC(_state.E); break;
		case 0x9C: SBC(getH()); break;
		case 0x9D: SBC(getL()); break;
		case 0x9E: SBC(Read(getIndirectHl())); break;
		case 0x9F: SBC(_state.A); break;
		case 0xA0: AND(_state.B); break;
		case 0xA1: AND(_state.C); break;
		case 0xA2: AND(_state.D); break;
		case 0xA3: AND(_state.E); break;
		case 0xA4: AND(getH()); break;
		case 0xA5: AND(getL()); break;
		case 0xA6: AND(Read(getIndirectHl())); break;
		case 0xA7: AND(_state.A); break;
		case 0xA8: XOR(_state.B); break;
		case 0xA9: XOR(_state.C); break;
		case 0xAA: XOR(_state.D); break;
		case 0xAB: XOR(_state.E); break;
		case 0xAC: XOR(getH()); break;
		case 0xAD: XOR(getL()); break;
		case 0xAE: XOR(Read(getIndirectHl())); break;
		case 0xAF: XOR(_state.A); break;
		case 0xB0: OR(_state.B); break;
		case 0xB1: OR(_state.C); break;
		case 0xB2: OR(_state.D); break;
		case 0xB3: OR(_state.E); break;
		case 0xB4: OR(getH()); break;
		case 0xB5: OR(getL()); break;
		case 0xB6: OR(Read(getIndirectHl())); break;
		case 0xB7: OR(_state.A); break;
		case 0xB8: CP(_state.B); break;
		case 0xB9: CP(_state.C); break;
		case 0xBA: CP(_state.D); break;
		case 0xBB: CP(_state.E); break;
		case 0xBC: CP(getH()); break;
		case 0xBD: CP(getL()); break;
		case 0xBE: CP(Read(getIndirectHl())); break;
		case 0xBF: CP(_state.A); break;
		case 0xC0: RET((_state.Flags & SmsCpuFlags::Zero) == 0); break;
		case 0xC1: POP(_regBC); break;
		case 0xC2: JP((_state.Flags & SmsCpuFlags::Zero) == 0, ReadCodeWord()); break;
		case 0xC3: JP(ReadCodeWord()); break;
		case 0xC4: CALL((_state.Flags & SmsCpuFlags::Zero) == 0, ReadCodeWord()); break;
		case 0xC5: PUSH(_regBC); break;
		case 0xC6: ADD(ReadCode()); break;
		case 0xC7: RST(0x00); break;
		case 0xC8: RET((_state.Flags & SmsCpuFlags::Zero) != 0); break;
		case 0xC9: RET(); break;
		case 0xCA: JP((_state.Flags & SmsCpuFlags::Zero) != 0, ReadCodeWord()); break;
		case 0xCB: PREFIX_CB<prefix>(); break;
		case 0xCC: CALL((_state.Flags & SmsCpuFlags::Zero) != 0, ReadCodeWord()); break;
		case 0xCD: CALL(ReadCodeWord()); break;
		case 0xCE: ADC(ReadCode()); break;
		case 0xCF: RST(0x08); break;
		case 0xD0: RET((_state.Flags & SmsCpuFlags::Carry) == 0); break;
		case 0xD1: POP(_regDE); break;
		case 0xD2: JP((_state.Flags & SmsCpuFlags::Carry) == 0, ReadCodeWord()); break;
		case 0xD3: OUT_Imm(ReadCode()); break;
		case 0xD4: CALL((_state.Flags & SmsCpuFlags::Carry) == 0, ReadCodeWord()); break;
		case 0xD5: PUSH(_regDE); break;
		case 0xD6: SUB(ReadCode()); break;
		case 0xD7: RST(0x10); break;
		case 0xD8: RET((_state.Flags & SmsCpuFlags::Carry) != 0); break;
		case 0xD9: EXX(); break;
		case 0xDA: JP((_state.Flags & SmsCpuFlags::Carry) != 0, ReadCodeWord()); break;
		case 0xDB: IN_Imm(ReadCode()); break;
		case 0xDC: CALL((_state.Flags & SmsCpuFlags::Carry) != 0, ReadCodeWord()); break;
		case 0xDD: ExecOpCode<0xDD>(ReadNextOpCode()); break;
		case 0xDE: SBC(ReadCode()); break;
		case 0xDF: RST(0x18); break;
		case 0xE0: RET((_state.Flags & SmsCpuFlags::Parity) == 0); break;
		case 0xE1: POP(getHl()); break;
		case 0xE2: JP((_state.Flags & SmsCpuFlags::Parity) == 0, ReadCodeWord()); break;
		case 0xE3: ExchangeSp(getHl()); break;
		case 0xE4: CALL((_state.Flags & SmsCpuFlags::Parity) == 0, ReadCodeWord()); break;
		case 0xE5: PUSH(getHl()); break;
		case 0xE6: AND(ReadCode()); break;
		case 0xE7: RST(0x20); break;
		case 0xE8: RET((_state.Flags & SmsCpuFlags::Parity) != 0); break;
		case 0xE9: JP(getHl()); break;
		case 0xEA: JP((_state.Flags & SmsCpuFlags::Parity) != 0, ReadCodeWord()); break;
		case 0xEB: ExchangeDeHl(); break;
		case 0xEC: CALL((_state.Flags & SmsCpuFlags::Parity) != 0, ReadCodeWord()); break;
		case 0xED: PREFIX_ED(); break;
		case 0xEE: XOR(ReadCode()); break;
		case 0xEF: RST(0x28); break;
		case 0xF0: RET((_state.Flags & SmsCpuFlags::Sign) == 0); break;
		case 0xF1: POP_AF(); break;
		case 0xF2: JP((_state.Flags & SmsCpuFlags::Sign) == 0, ReadCodeWord()); break;
		case 0xF3: DI(); break;
		case 0xF4: CALL((_state.Flags & SmsCpuFlags::Sign) == 0, ReadCodeWord()); break;
		case 0xF5: PUSH(_regAF); break;
		case 0xF6: OR(ReadCode()); break;
		case 0xF7: RST(0x30); break;
		case 0xF8: RET((_state.Flags & SmsCpuFlags::Sign) != 0); break;
		case 0xF9: LD(_state.SP, getHl()); ExecCycles(2); break;
		case 0xFA: JP((_state.Flags & SmsCpuFlags::Sign) != 0, ReadCodeWord()); break;
		case 0xFB: EI(); break;
		case 0xFC: CALL((_state.Flags & SmsCpuFlags::Sign) != 0, ReadCodeWord()); break;
		case 0xFD: ExecOpCode<0xFD>(ReadNextOpCode()); break;
		case 0xFE: CP(ReadCode()); break;
		case 0xFF: RST(0x38); break;
	}
}

void SmsCpu::PREFIX_ED()
{
	switch(ReadNextOpCode()) {
		case 0x40: IN(_state.B, _state.C); break;
		case 0x41: OUT(_state.B, _state.C); break;
		case 0x42: SBC16(_regBC); break;
		case 0x43: LD_Indirect16(ReadCodeWord(), _regBC); break;
		case 0x44: NEG(); break;
		case 0x45: RETI(); break;
		case 0x46: IM(0); break; 
		case 0x47: LD(_state.I, _state.A); ExecCycles(1); break;
		case 0x48: IN(_state.C, _state.C); break;
		case 0x49: OUT(_state.C, _state.C); break;
		case 0x4A: ADC16(_regBC); break;
		case 0x4B: LDReg_Indirect16(_regBC, ReadCodeWord()); break;
		case 0x4C: NEG(); break;
		case 0x4D: RETI(); break;
		case 0x4E: IM(0); break;
		case 0x4F: LD(_state.R, _state.A); ExecCycles(1); break;
		case 0x50: IN(_state.D, _state.C); break;
		case 0x51: OUT(_state.D, _state.C); break;
		case 0x52: SBC16(_regDE); break;
		case 0x53: LD_Indirect16(ReadCodeWord(), _regDE); break;
		case 0x54: NEG(); break;
		case 0x55: RETI(); break;
		case 0x56: IM(1); break;
		case 0x57: LD_IR(_state.A, _state.I); break;
		case 0x58: IN(_state.E, _state.C); break;
		case 0x59: OUT(_state.E, _state.C); break;
		case 0x5A: ADC16(_regDE); break;
		case 0x5B: LDReg_Indirect16(_regDE, ReadCodeWord()); break;
		case 0x5C: NEG(); break;
		case 0x5D: RETI(); break;
		case 0x5E: IM(2); break;
		case 0x5F: LD_IR(_state.A, _state.R); break;
		case 0x60: IN(_state.H, _state.C); break;
		case 0x61: OUT(_state.H, _state.C); break;
		case 0x62: SBC16(_regHL); break;
		case 0x63: LD_Indirect16(ReadCodeWord(), _regHL); break;
		case 0x64: NEG(); break;
		case 0x65: RETI(); break;
		case 0x66: IM(0); break;
		case 0x67: RRD(); break;
		case 0x68: IN(_state.L, _state.C); break;
		case 0x69: OUT(_state.L, _state.C); break;
		case 0x6A: ADC16(_regHL); break;
		case 0x6B: LDReg_Indirect16(_regHL, ReadCodeWord()); break;
		case 0x6C: NEG(); break;
		case 0x6D: RETI(); break;
		case 0x6E: IM(0); break;
		case 0x6F: RLD(); break;
		case 0x70: IN(_state.C); break;
		case 0x71: OUT(0, _state.C); break;
		case 0x72: SBC16(_state.SP); break;
		case 0x73: LD_Indirect16(ReadCodeWord(), _state.SP); break;
		case 0x74: NEG(); break;
		case 0x75: RETI(); break;
		case 0x76: IM(1); break;
		case 0x77: break; //NOP
		case 0x78: IN(_state.A, _state.C); break;
		case 0x79: OUT(_state.A, _state.C); break;
		case 0x7A: ADC16(_state.SP); break;
		case 0x7B: LDReg_Indirect16(_state.SP, ReadCodeWord()); break;
		case 0x7C: NEG(); break;
		case 0x7D: RETI(); break;
		case 0x7E: IM(2); break;
		case 0x7F: break; //NOP
		case 0xA0: LDI(); break;
		case 0xA1: CPI(); break;
		case 0xA2: INI(); break;
		case 0xA3: OUTI(); break;
		case 0xA8: LDD(); break;
		case 0xA9: CPD(); break;
		case 0xAA: IND(); break;
		case 0xAB: OUTD(); break;
		case 0xB0: LDIR(); break;
		case 0xB1: CPIR(); break;
		case 0xB2: INIR(); break;
		case 0xB3: OTIR(); break;
		case 0xB8: LDDR(); break;
		case 0xB9: CPDR(); break;
		case 0xBA: INDR(); break;
		case 0xBB: OTDR(); break;
		default: break;
	}
}

void SmsCpu::EXX()
{
	std::swap(_state.B, _state.AltB);
	std::swap(_state.C, _state.AltC);
	std::swap(_state.D, _state.AltD);
	std::swap(_state.E, _state.AltE);
	std::swap(_state.H, _state.AltH);
	std::swap(_state.L, _state.AltL);
}

void SmsCpu::ExchangeAf()
{
	std::swap(_state.A, _state.AltA);
	std::swap(_state.Flags, _state.AltFlags);
}

void SmsCpu::ExchangeSp(Register16& reg)
{
	uint8_t lo = Read(_state.SP);
	uint8_t hi = Read(_state.SP+1);

	uint16_t regValue = reg;
	reg.Write(lo | (hi << 8));
	_state.WZ = reg;

	ExecCycles(1);

	Write(_state.SP, (uint8_t)regValue);
	Write(_state.SP + 1, (uint8_t)(regValue >> 8));

	ExecCycles(2);
}

void SmsCpu::ExchangeDeHl()
{
	std::swap(_state.D, _state.H);
	std::swap(_state.E, _state.L);
}

void SmsCpu::DJNZ()
{
	ExecCycles(1);
	int8_t offset = ReadCode();
	_state.B--;
	if(_state.B != 0) {
		ExecCycles(5);
		_state.PC += offset;
		_state.WZ = _state.PC;
	}
}

void SmsCpu::NEG()
{
	uint8_t originalValue = _state.A;
	uint8_t newValue = (uint8_t)-_state.A;
	SetFlagState(SmsCpuFlags::Parity, _state.A == 0x80);
	_state.A = newValue;
	SetStandardFlags<0xE8>(_state.A);
	SetFlag(SmsCpuFlags::AddSub);
	SetFlagState(SmsCpuFlags::Carry, originalValue != 0);
	SetFlagState(SmsCpuFlags::HalfCarry, (originalValue ^ _state.A) & 0x10);
}

void SmsCpu::RRD()
{
	_state.WZ = _regHL + 1;
	uint8_t value = Read(_regHL);
	uint8_t a = _state.A;
	_state.A &= 0xF0;
	_state.A |= value & 0x0F;
	value >>= 4;
	value |= (a & 0x0F) << 4;

	ExecCycles(4);

	Write(_regHL, value);

	SetStandardFlags<0xEC>(_state.A);
	ClearFlag(SmsCpuFlags::AddSub);
	ClearFlag(SmsCpuFlags::HalfCarry);
}

void SmsCpu::RLD()
{
	_state.WZ = _regHL + 1;
	uint8_t value = Read(_regHL);
	uint8_t a = _state.A;

	_state.A &= 0xF0;
	_state.A |= (value & 0xF0) >> 4;
	value <<= 4;
	value |= (a & 0x0F);
	Write(_regHL, value);

	SetStandardFlags<0xEC>(_state.A);
	ClearFlag(SmsCpuFlags::AddSub);
	ClearFlag(SmsCpuFlags::HalfCarry);
}

template<bool forInc>
void SmsCpu::CPD()
{
	uint8_t value = Read(_regHL);
	
	ExecCycles(5);

	uint8_t result = (uint8_t)((int)_state.A - value);

	bool halfCarry = ((_state.A ^ value ^ result) & 0x10) != 0;
	SetFlagState(SmsCpuFlags::HalfCarry, halfCarry);
	SetStandardFlags<0xC0>(result);
	SetFlagState(SmsCpuFlags::F3, (result - halfCarry) & 0x08);
	SetFlagState(SmsCpuFlags::F5, (result - halfCarry) & 0x02);
	SetFlag(SmsCpuFlags::AddSub);

	if constexpr(forInc) {
		_regHL.Inc();
		_state.WZ++;
	} else {
		_regHL.Dec();
		_state.WZ--;
	}
	_regBC.Dec();

	SetFlagState(SmsCpuFlags::Parity, _regBC != 0);
}

void SmsCpu::CPDR()
{
	CPD<false>();
	if(_regBC != 0 && !CheckFlag(SmsCpuFlags::Zero)) {
		ExecCycles(5);
		_state.PC -= 2;
		_state.WZ = _state.PC + 1;
		SetStandardFlags<0x28>(_state.PC >> 8);
	}
}

void SmsCpu::CPI()
{
	CPD<true>();
}

void SmsCpu::CPIR()
{
	CPI();
	if(_regBC != 0 && !CheckFlag(SmsCpuFlags::Zero)) {
		ExecCycles(5);
		_state.PC -= 2;
		_state.WZ = _state.PC + 1;
		SetStandardFlags<0x28>(_state.PC >> 8);
	}
}

template<bool forInc>
void SmsCpu::LDD()
{
	uint8_t val = Read(_regHL);
	Write(_regDE, val);

	if constexpr(forInc) {
		_regHL.Inc();
		_regDE.Inc();
	} else {
		_regHL.Dec();
		_regDE.Dec();
	}
	_regBC.Dec();
	
	ExecCycles(2);

	ClearFlag(SmsCpuFlags::AddSub);
	ClearFlag(SmsCpuFlags::HalfCarry);
	SetFlagState(SmsCpuFlags::Parity, _regBC != 0);

	uint8_t flagVal = _state.A + val;
	SetFlagState(SmsCpuFlags::F3, flagVal & 0x08);
	SetFlagState(SmsCpuFlags::F5, flagVal & 0x02);
}

void SmsCpu::LDDR()
{
	LDD();
	if(_regBC != 0) {
		ExecCycles(5);
		_state.PC -= 2;
		_state.WZ = _state.PC + 1;
		SetStandardFlags<0x28>(_state.PC >> 8);
	}
}

void SmsCpu::LDI()
{
	LDD<true>();
}

void SmsCpu::LDIR()
{
	LDI();
	if(_regBC != 0) {
		ExecCycles(5);
		_state.PC -= 2;
		_state.WZ = _state.PC + 1;
		SetStandardFlags<0x28>(_state.PC >> 8);
	}
}

void SmsCpu::OUT(uint8_t src, uint8_t port)
{
	WritePort(port, src);
	_state.WZ = _regBC + 1;
}

void SmsCpu::OUT_Imm(uint8_t port)
{
	WritePort(port, _state.A);
	_state.WZ = (_state.A << 8) + ((port + 1) & 0xFF);
}

void SmsCpu::IN(uint8_t& dst, uint8_t port)
{
	dst = IN(port);
}

uint8_t SmsCpu::IN(uint8_t port)
{
	uint8_t val = ReadPort(port);
	SetStandardFlags<0xEC>(val);
	ClearFlag(SmsCpuFlags::AddSub);
	ClearFlag(SmsCpuFlags::HalfCarry);
	_state.WZ = _regBC + 1;
	return val;
}

void SmsCpu::IN_Imm(uint8_t port)
{
	//IN A, (n) doesn't update flags
	_state.WZ = (_state.A << 8) + port + 1;
	_state.A = ReadPort(port);
}

template<bool forInc>
void SmsCpu::OUTD()
{
	ExecCycles(1);
	_state.B--;
	if constexpr(forInc) {
		_state.WZ = _regBC + 1;
	} else {
		_state.WZ = _regBC - 1;
	}
	SetStandardFlags<0xE8>(_state.B);
	SetFlag(SmsCpuFlags::AddSub);
	uint8_t val = Read(_regHL);
	if constexpr(forInc) {
		_regHL.Inc();
	} else {
		_regHL.Dec();
	}
	WritePort(_state.C, val);
}

void SmsCpu::OTDR()
{
	OUTD<false>();
	if(_state.B != 0) {
		ExecCycles(5);
		_state.PC -= 2;
		UpdateInOutRepeatFlags();
	}
}

void SmsCpu::OUTI()
{
	OUTD<true>();
}

void SmsCpu::OTIR()
{
	OUTD<true>();
	if(_state.B != 0) {
		ExecCycles(5);
		_state.PC -= 2;
		UpdateInOutRepeatFlags();
	}
}

template<bool forInc>
void SmsCpu::IND()
{
	ExecCycles(1);

	if constexpr(forInc) {
		_state.WZ = _regBC + 1;
	} else {
		_state.WZ = _regBC - 1;
	}
	_state.B--;
	SetStandardFlags<0xE8>(_state.B);
	SetFlag(SmsCpuFlags::AddSub);
	uint8_t val = ReadPort(_state.C);
	Write(_regHL, val);
	if constexpr(forInc) {
		_regHL.Inc();
	} else {
		_regHL.Dec();
	}
}

void SmsCpu::INDR()
{
	IND<false>();
	if(_state.B != 0) {
		ExecCycles(5);
		_state.PC -= 2;
		UpdateInOutRepeatFlags();
	}
}

void SmsCpu::INI()
{
	IND<true>();
}

void SmsCpu::INIR()
{
	IND<true>();
	if(_state.B != 0) {
		ExecCycles(5);
		_state.PC -= 2;
		UpdateInOutRepeatFlags();
	}
}

void SmsCpu::UpdateInOutRepeatFlags()
{
	SetStandardFlags<0x28>(_state.PC >> 8);
	if(CheckFlag(SmsCpuFlags::Carry)) {
		if(CheckFlag(SmsCpuFlags::Sign)) {
			SetFlagState(SmsCpuFlags::Parity, _parity.CheckParity((_state.B - 1) & 0x07) == (uint8_t)CheckFlag(SmsCpuFlags::Parity));
			SetFlagState(SmsCpuFlags::HalfCarry, (_state.B & 0x0F) == 0);
		} else {
			SetFlagState(SmsCpuFlags::Parity, _parity.CheckParity((_state.B + 1) & 0x07) == (uint8_t)CheckFlag(SmsCpuFlags::Parity));
			SetFlagState(SmsCpuFlags::HalfCarry, (_state.B & 0x0F) == 0x0F);
		}
	} else {
		SetFlagState(SmsCpuFlags::Parity, _parity.CheckParity(_state.B & 0x07) == (uint8_t)CheckFlag(SmsCpuFlags::Parity));
	}
}

void SmsCpu::IncrementR()
{
	_state.R = (_state.R & 0x80) | ((_state.R + 1) & 0x7F);
}

void SmsCpu::ExecCycles(uint8_t cycles)
{
	_state.CycleCount += cycles;
#ifndef DUMMYCPU
	_memoryManager->Exec(cycles * 3);
#endif
}

uint8_t SmsCpu::ReadOpCode()
{
	ExecCycles(4);
	IncrementR();
	uint8_t value = ReadMemory<MemoryOperationType::ExecOpCode>(_state.PC);
	_state.PC++;
	return value;
}

uint8_t SmsCpu::ReadNextOpCode()
{
	ExecCycles(4);
	IncrementR();
	uint8_t value = ReadMemory<MemoryOperationType::ExecOperand>(_state.PC);
	_state.PC++;
	return value;
}

uint8_t SmsCpu::ReadCode()
{
	ExecCycles(3);
	uint8_t value = ReadMemory<MemoryOperationType::ExecOperand>(_state.PC);
	_state.PC++;
	return value;
}

uint16_t SmsCpu::ReadCodeWord()
{
	uint8_t low = ReadCode();
	uint8_t high = ReadCode();
	return (high << 8) | low;
}

uint8_t SmsCpu::Read(uint16_t addr)
{
	ExecCycles(3);
	return ReadMemory<MemoryOperationType::Read>(addr);
}

template<MemoryOperationType type>
uint8_t SmsCpu::ReadMemory(uint16_t addr)
{
#ifdef DUMMYCPU
	uint8_t value = _memoryManager->DebugRead(addr);
	LogMemoryOperation(addr, value, type, MemoryType::SmsMemory);
	return value;
#else
	return _memoryManager->Read(addr, type);
#endif
}

void SmsCpu::Write(uint16_t addr, uint8_t value)
{
	ExecCycles(3);

#ifdef DUMMYCPU
	LogMemoryOperation(addr, value, MemoryOperationType::Write, MemoryType::SmsMemory);
#else
	_memoryManager->Write(addr, value);
#endif
}

uint8_t SmsCpu::ReadPort(uint8_t port)
{
	ExecCycles(4);
#ifdef DUMMYCPU
	uint8_t value = _memoryManager->DebugReadPort(port);
	LogMemoryOperation(port, value, MemoryOperationType::Read, MemoryType::SmsPort);
	return value;
#else
	return _memoryManager->ReadPort(port);
#endif
}

void SmsCpu::WritePort(uint8_t port, uint8_t value)
{
	ExecCycles(4);
#ifdef DUMMYCPU
	LogMemoryOperation(port, value, MemoryOperationType::Write, MemoryType::SmsPort);
#else
	_memoryManager->WritePort(port, value);
#endif
}

template<uint8_t mask>
void SmsCpu::SetStandardFlags(uint8_t value)
{
	if constexpr((mask & SmsCpuFlags::Sign) != 0) {
		SetFlagState(SmsCpuFlags::Sign, value & 0x80);
	}
	if constexpr((mask & SmsCpuFlags::Zero) != 0) {
		SetFlagState(SmsCpuFlags::Zero, value == 0);
	}
	if constexpr((mask & SmsCpuFlags::F5) != 0) {
		SetFlagState(SmsCpuFlags::F5, value & SmsCpuFlags::F5);
	}
	if constexpr((mask & SmsCpuFlags::F3) != 0) {
		SetFlagState(SmsCpuFlags::F3, value & SmsCpuFlags::F3);
	}
	if constexpr((mask & SmsCpuFlags::Parity) != 0) {
		SetFlagState(SmsCpuFlags::Parity, _parity.CheckParity(value));
	}
}

bool SmsCpu::CheckFlag(uint8_t flag)
{
	return (_state.Flags & flag) != 0;
}

void SmsCpu::SetFlag(uint8_t flag)
{
	_state.Flags |= flag;
	_state.FlagsChanged |= 0x01;
}

void SmsCpu::ClearFlag(uint8_t flag)
{
	_state.Flags &= ~flag;
	_state.FlagsChanged |= 0x01;
}

void SmsCpu::SetFlagState(uint8_t flag, bool state)
{
	if(state) {
		SetFlag(flag);
	} else {
		ClearFlag(flag);
	}
}

void SmsCpu::PushByte(uint8_t value)
{
	_state.SP--;
	Write(_state.SP, value);
}

void SmsCpu::PushWord(uint16_t value)
{
	PushByte(value >> 8);
	PushByte((uint8_t)value);
}

uint16_t SmsCpu::PopWord()
{
	uint8_t low = Read(_state.SP);
	_state.SP++;
	uint8_t high = Read(_state.SP);
	_state.SP++;
	return (high << 8) | low;
}

void SmsCpu::LD(uint8_t& dst, uint8_t value)
{
	dst = value;
}

void SmsCpu::LD_IR(uint8_t& dst, uint8_t value)
{
	dst = value;

	SetStandardFlags<0xE4>(value);
	SetFlagState(SmsCpuFlags::Parity, _state.IFF2);
	ClearFlag(SmsCpuFlags::HalfCarry);
	ClearFlag(SmsCpuFlags::AddSub);
	ExecCycles(1);
}

void SmsCpu::LD(uint16_t& dst, uint16_t value)
{
	dst = value;
}

void SmsCpu::LD(Register16& dst, uint16_t value)
{
	dst.Write(value);
}

void SmsCpu::LD_Indirect(uint16_t dst, uint8_t value)
{
	Write(dst, value);
}

void SmsCpu::LDA_Address(uint16_t addr)
{
	uint8_t value = Read(addr);
	_state.A = value;
	_state.WZ = addr + 1;
}

void SmsCpu::LD_Indirect_A(uint16_t dst)
{
	Write(dst, _state.A);
	_state.WZ = (_state.A << 8) | ((dst + 1) & 0xFF);
}

void SmsCpu::LD_IndirectImm(uint16_t dst)
{
	uint8_t val = ReadCode();
	ExecCycles(2);
	Write(dst, val);
}

void SmsCpu::LD_Indirect16(uint16_t dst, uint16_t value)
{
	Write(dst, (uint8_t)value);
	Write(dst + 1, value >> 8);
	_state.WZ = dst + 1;
}

void SmsCpu::LDReg_Indirect16(uint16_t& dst, uint16_t addr)
{
	uint8_t lo = Read(addr);
	uint8_t hi = Read(addr + 1);
	dst = lo | (hi << 8);
}

void SmsCpu::LDReg_Indirect16(Register16& dst, uint16_t addr)
{
	uint8_t lo = Read(addr);
	uint8_t hi = Read(addr + 1);
	dst.Write(lo | (hi << 8));
	_state.WZ = addr + 1;
}

void SmsCpu::INC(uint8_t& dst)
{
	SetFlagState(SmsCpuFlags::HalfCarry, ((dst ^ 1 ^ (dst + 1)) & 0x10) != 0);
	dst++;

	ClearFlag(SmsCpuFlags::AddSub);
	SetFlagState(SmsCpuFlags::Parity, dst == 0x80);
	SetStandardFlags<0xF8>(dst);
}

void SmsCpu::INC(Register16& dst)
{
	//16-bit inc does not alter flags
	ExecCycles(2);
	dst.Inc();
}

void SmsCpu::INC_SP()
{
	ExecCycles(2);
	_state.SP++;
}

void SmsCpu::INC_Indirect(uint16_t addr)
{
	uint8_t val = Read(addr);
	INC(val);
	Write(addr, val);
}

void SmsCpu::DEC(uint8_t& dst)
{
	SetFlagState(SmsCpuFlags::HalfCarry, (dst & 0x0F) == 0x00);
	dst--;

	SetFlag(SmsCpuFlags::AddSub);
	SetFlagState(SmsCpuFlags::Parity, dst == 0x7F);
	SetStandardFlags<0xF8>(dst);
}

void SmsCpu::DEC(Register16& dst)
{
	//16-bit dec does not alter flags
	ExecCycles(2);
	dst.Dec();
}

void SmsCpu::DEC_Indirect(uint16_t addr)
{
	uint8_t val = Read(addr);
	DEC(val);
	Write(addr, val);
}

void SmsCpu::DEC_SP()
{
	_state.SP--;
	ExecCycles(2);
}

void SmsCpu::ADD(uint8_t value)
{
	int result = _state.A + value;

	SetFlagState(SmsCpuFlags::HalfCarry, ((_state.A ^ value ^ result) & 0x10) != 0);
	SetFlagState(SmsCpuFlags::Parity, (_state.A ^ value ^ 0x80) & (value ^ result) & 0x80);

	_state.A = (uint8_t)result;

	SetFlagState(SmsCpuFlags::Carry, result > 0xFF);
	ClearFlag(SmsCpuFlags::AddSub);
	SetStandardFlags<0xE8>(result);
}

void SmsCpu::ADD(Register16& reg, uint16_t value)
{
	_state.WZ = reg + 1;

	int result = reg.Read() + value;

	SetFlagState(SmsCpuFlags::HalfCarry, ((reg.Read() ^ value ^ result) & 0x1000) != 0);

	reg.Write((uint16_t)result);

	SetFlagState(SmsCpuFlags::Carry, result > 0xFFFF);
	ClearFlag(SmsCpuFlags::AddSub);
	SetStandardFlags<0x28>(((uint16_t)result) >> 8);

	ExecCycles(7);
}

void SmsCpu::ADC(uint8_t value)
{
	uint8_t carry = (_state.Flags & SmsCpuFlags::Carry);
	int result = _state.A + value + carry;

	SetFlagState(SmsCpuFlags::HalfCarry, (_state.A ^ value ^ result) & 0x10);
	SetFlagState(SmsCpuFlags::Parity, (_state.A ^ value ^ 0x80) & (_state.A ^ result) & 0x80);

	_state.A = (uint8_t)result;

	SetFlagState(SmsCpuFlags::Carry, result > 0xFF);
	ClearFlag(SmsCpuFlags::AddSub);
	SetStandardFlags<0xE8>(result);
}

void SmsCpu::ADC16(uint16_t value)
{
	_state.WZ = _regHL + 1;
	uint8_t carry = (_state.Flags & SmsCpuFlags::Carry);
	int result = (int)_regHL + value + carry;

	SetFlagState(SmsCpuFlags::HalfCarry, (_regHL ^ value ^ result) & 0x1000);
	SetFlagState(SmsCpuFlags::Parity, (_regHL ^ value ^ 0x8000) & (_regHL ^ result) & 0x8000);

	_regHL.Write((uint16_t)result);

	SetFlagState(SmsCpuFlags::Carry, result > 0xFFFF);
	SetFlagState(SmsCpuFlags::Zero, _regHL == 0);
	ClearFlag(SmsCpuFlags::AddSub);
	SetStandardFlags<0xA8>(_regHL >> 8);

	ExecCycles(7);
}

void SmsCpu::SUB(uint8_t value)
{
	int result = (int)_state.A - value;

	SetFlagState(SmsCpuFlags::HalfCarry, (_state.A ^ value ^ result) & 0x10);
	SetFlagState(SmsCpuFlags::Parity, (_state.A ^ value) & (_state.A ^ result) & 0x80);

	_state.A = (uint8_t)result;

	SetFlagState(SmsCpuFlags::Carry, result < 0);
	SetFlag(SmsCpuFlags::AddSub);
	SetStandardFlags<0xE8>(result);
}

void SmsCpu::SBC(uint8_t value)
{
	uint8_t carry = (_state.Flags & SmsCpuFlags::Carry);
	int result = (int)_state.A - value - carry;

	SetFlagState(SmsCpuFlags::HalfCarry, (_state.A ^ value ^ result) & 0x10);
	SetFlagState(SmsCpuFlags::Parity, (_state.A ^ value) & (_state.A ^ result) & 0x80);

	_state.A = (uint8_t)result;

	SetFlagState(SmsCpuFlags::Carry, result < 0);
	SetFlag(SmsCpuFlags::AddSub);
	SetStandardFlags<0xE8>(result);
}

void SmsCpu::SBC16(uint16_t value)
{
	_state.WZ = _regHL + 1;
	uint8_t carry = (_state.Flags & SmsCpuFlags::Carry);
	int result = (int)_regHL - value - carry;

	SetFlagState(SmsCpuFlags::HalfCarry, (_regHL ^ value ^ result) & 0x1000);
	SetFlagState(SmsCpuFlags::Parity, (_regHL ^ value) & (_regHL ^ result) & 0x8000);

	_regHL.Write((uint16_t)result);

	SetFlagState(SmsCpuFlags::Carry, result < 0);
	SetFlagState(SmsCpuFlags::Zero, _regHL == 0);
	SetFlag(SmsCpuFlags::AddSub);
	SetStandardFlags<0xA8>(_regHL >> 8);

	ExecCycles(7);
}

void SmsCpu::AND(uint8_t value)
{
	_state.A &= value;
	UpdateLogicalOpFlags(_state.A);
	SetFlag(SmsCpuFlags::HalfCarry);
}

void SmsCpu::OR(uint8_t value)
{
	_state.A |= value;
	UpdateLogicalOpFlags(_state.A);
}

void SmsCpu::XOR(uint8_t value)
{
	_state.A ^= value;
	UpdateLogicalOpFlags(_state.A);
}

void SmsCpu::UpdateLogicalOpFlags(uint8_t value)
{
	SetStandardFlags<0xEC>(value);
	ClearFlag(SmsCpuFlags::AddSub);
	ClearFlag(SmsCpuFlags::Carry);
	ClearFlag(SmsCpuFlags::HalfCarry);
}

void SmsCpu::CP(uint8_t value)
{
	int result = (int)_state.A - value;

	SetFlagState(SmsCpuFlags::Carry, result < 0);
	SetFlagState(SmsCpuFlags::HalfCarry, ((_state.A ^ value ^ result) & 0x10) != 0);
	SetFlagState(SmsCpuFlags::Parity, (_state.A ^ value) & (_state.A ^ result) & 0x80);
	SetStandardFlags<0xC0>(result);
	SetStandardFlags<0x28>(value);
	SetFlag(SmsCpuFlags::AddSub);
}

void SmsCpu::NOP()
{

}

void SmsCpu::HALT()
{
	_state.Halted = true;
}

void SmsCpu::CPL()
{
	_state.A ^= 0xFF;
	SetFlag(SmsCpuFlags::AddSub);
	SetFlag(SmsCpuFlags::HalfCarry);
	SetStandardFlags<0x28>(_state.A);
}

void SmsCpu::RL(uint8_t& dst)
{
	uint8_t val = GetCbValue(dst);
	uint8_t carry = (uint8_t)CheckFlag(SmsCpuFlags::Carry);
	SetFlagState(SmsCpuFlags::Carry, (val & 0x80) != 0);
	SetCbValue(dst, (val << 1) | carry);

	SetStandardFlags<0xEC>(dst);
	ClearFlag(SmsCpuFlags::AddSub);
	ClearFlag(SmsCpuFlags::HalfCarry);
}

void SmsCpu::RL_Indirect(uint16_t addr)
{
	if(_cbAddress < 0) {
		uint8_t val = Read(addr);
		RL(val);
		ExecCycles(1);
		Write(addr, val);
	} else {
		uint8_t val = 0;
		RL(val);
	}
}

void SmsCpu::RLC(uint8_t& dst)
{
	uint8_t val = GetCbValue(dst);
	SetFlagState(SmsCpuFlags::Carry, (val & 0x80) != 0);
	SetCbValue(dst, (val << 1) | ((val & 0x80) >> 7));

	SetStandardFlags<0xEC>(dst);
	ClearFlag(SmsCpuFlags::AddSub);
	ClearFlag(SmsCpuFlags::HalfCarry);
}

void SmsCpu::RLC_Indirect(uint16_t addr)
{
	if(_cbAddress < 0) {
		uint8_t val = Read(addr);
		RLC(val);
		ExecCycles(1);
		Write(addr, val);
	} else {
		uint8_t val = 0;
		RLC(val);
	}
}

uint8_t SmsCpu::GetCbValue(uint8_t dst)
{
	return _cbAddress >= 0 ? Read(_cbAddress) : dst;
}

void SmsCpu::SetCbValue(uint8_t& dst, uint8_t val)
{
	if(_cbAddress >= 0) {
		ExecCycles(1);
		Write(_cbAddress, val);
	}
	dst = val;
}

void SmsCpu::RR(uint8_t& dst)
{
	uint8_t val = GetCbValue(dst);
	uint8_t carry = (uint8_t)CheckFlag(SmsCpuFlags::Carry) << 7;
	SetFlagState(SmsCpuFlags::Carry, (val & 0x01) != 0);
	SetCbValue(dst, (val >> 1) | carry);

	SetStandardFlags<0xEC>(dst);
	ClearFlag(SmsCpuFlags::AddSub);
	ClearFlag(SmsCpuFlags::HalfCarry);
}

void SmsCpu::RR_Indirect(uint16_t addr)
{
	if(_cbAddress < 0) {
		uint8_t val = Read(addr);
		RR(val);
		ExecCycles(1);
		Write(addr, val);
	} else {
		uint8_t val = 0;
		RR(val);
	}
}

void SmsCpu::RRC(uint8_t& dst)
{
	uint8_t val = GetCbValue(dst);
	SetFlagState(SmsCpuFlags::Carry, (val & 0x01) != 0);
	SetCbValue(dst, (val >> 1) | ((val & 0x01) << 7));

	SetStandardFlags<0xEC>(dst);
	ClearFlag(SmsCpuFlags::AddSub);
	ClearFlag(SmsCpuFlags::HalfCarry);
}

void SmsCpu::RRC_Indirect(uint16_t addr)
{
	if(_cbAddress < 0) {
		uint8_t val = Read(addr);
		RRC(val);
		ExecCycles(1);
		Write(addr, val);
	} else {
		uint8_t val = 0;
		RRC(val);
	}
}

void SmsCpu::RRA()
{
	uint8_t carry = (uint8_t)CheckFlag(SmsCpuFlags::Carry) << 7;
	SetFlagState(SmsCpuFlags::Carry, (_state.A & 0x01) != 0);
	_state.A = (_state.A >> 1) | carry;

	SetStandardFlags<0x28>(_state.A);
	ClearFlag(SmsCpuFlags::AddSub);
	ClearFlag(SmsCpuFlags::HalfCarry);
}

void SmsCpu::RRCA()
{
	SetFlagState(SmsCpuFlags::Carry, (_state.A & 0x01) != 0);
	_state.A = (_state.A >> 1) | ((_state.A & 0x01) << 7);

	SetStandardFlags<0x28>(_state.A);
	ClearFlag(SmsCpuFlags::AddSub);
	ClearFlag(SmsCpuFlags::HalfCarry);
}

void SmsCpu::RLCA()
{
	SetFlagState(SmsCpuFlags::Carry, (_state.A & 0x80) != 0);
	_state.A = (_state.A << 1) | ((_state.A & 0x80) >> 7);

	SetStandardFlags<0x28>(_state.A);
	ClearFlag(SmsCpuFlags::AddSub);
	ClearFlag(SmsCpuFlags::HalfCarry);
}

void SmsCpu::RLA()
{
	uint8_t carry = (uint8_t)CheckFlag(SmsCpuFlags::Carry);
	SetFlagState(SmsCpuFlags::Carry, (_state.A & 0x80) != 0);
	_state.A = (_state.A << 1) | carry;

	SetStandardFlags<0x28>(_state.A);
	ClearFlag(SmsCpuFlags::AddSub);
	ClearFlag(SmsCpuFlags::HalfCarry);
}

void SmsCpu::SRL(uint8_t& dst)
{
	uint8_t val = GetCbValue(dst);
	SetFlagState(SmsCpuFlags::Carry, (val & 0x01) != 0);
	SetCbValue(dst, (val >> 1));

	SetStandardFlags<0xEC>(dst);
	ClearFlag(SmsCpuFlags::AddSub);
	ClearFlag(SmsCpuFlags::HalfCarry);
}

void SmsCpu::SRL_Indirect(uint16_t addr)
{
	if(_cbAddress < 0) {
		uint8_t val = Read(addr);
		SRL(val);
		ExecCycles(1);
		Write(addr, val);
	} else {
		uint8_t val = 0;
		SRL(val);
	}
}

void SmsCpu::SRA(uint8_t& dst)
{
	uint8_t val = GetCbValue(dst);
	SetFlagState(SmsCpuFlags::Carry, (val & 0x01) != 0);
	SetCbValue(dst, (val & 0x80) | (val >> 1));

	SetStandardFlags<0xEC>(dst);
	ClearFlag(SmsCpuFlags::AddSub);
	ClearFlag(SmsCpuFlags::HalfCarry);
}

void SmsCpu::SRA_Indirect(uint16_t addr)
{
	if(_cbAddress < 0) {
		uint8_t val = Read(addr);
		SRA(val);
		ExecCycles(1);
		Write(addr, val);
	} else {
		uint8_t val = 0;
		SRA(val);
	}
}

void SmsCpu::SLA(uint8_t& dst)
{
	uint8_t val = GetCbValue(dst);
	SetFlagState(SmsCpuFlags::Carry, (val & 0x80) != 0);
	SetCbValue(dst, val << 1);

	SetStandardFlags<0xEC>(dst);
	ClearFlag(SmsCpuFlags::AddSub);
	ClearFlag(SmsCpuFlags::HalfCarry);
}

void SmsCpu::SLA_Indirect(uint16_t addr)
{
	if(_cbAddress < 0) {
		uint8_t val = Read(addr);
		SLA(val);
		ExecCycles(1);
		Write(addr, val);
	} else {
		uint8_t val = 0;
		SLA(val);
	}
}

void SmsCpu::SLL(uint8_t& dst)
{
	uint8_t val = GetCbValue(dst);
	SetFlagState(SmsCpuFlags::Carry, (val & 0x80) != 0);
	SetCbValue(dst, (val << 1) | 0x01);

	SetStandardFlags<0xEC>(dst);
	ClearFlag(SmsCpuFlags::AddSub);
	ClearFlag(SmsCpuFlags::HalfCarry);
}

void SmsCpu::SLL_Indirect(uint16_t addr)
{
	if(_cbAddress < 0) {
		uint8_t val = Read(addr);
		SLL(val);
		ExecCycles(1);
		Write(addr, val);
	} else {
		uint8_t val = 0;
		SLL(val);
	}
}

template<uint8_t bit>
void SmsCpu::BIT(uint8_t src)
{
	uint8_t val = GetCbValue(src);
	uint8_t andResult = (val & (1 << bit));
	SetStandardFlags<0xC4>(andResult);
	SetStandardFlags<0x28>(val);
	ClearFlag(SmsCpuFlags::AddSub);
	SetFlag(SmsCpuFlags::HalfCarry);
}

template<uint8_t bit>
void SmsCpu::BIT_Indirect(Register16& src)
{
	uint8_t val = Read(_cbAddress >= 0 ? _cbAddress : src);
	uint8_t andResult = (val & (1 << bit));
	SetStandardFlags<0xC4>(andResult);
	SetStandardFlags<0x28>(_state.WZ >> 8);
	ClearFlag(SmsCpuFlags::AddSub);
	SetFlag(SmsCpuFlags::HalfCarry);
	ExecCycles(1);
}

template<uint8_t bit>
void SmsCpu::RES(uint8_t& dst)
{
	uint8_t val = GetCbValue(dst);
	SetCbValue(dst, val & ~(1 << bit));
}

template<uint8_t bit>
void SmsCpu::RES_Indirect(uint16_t addr)
{
	if(_cbAddress < 0) {
		uint8_t val = Read(addr);
		RES<bit>(val);
		ExecCycles(1);
		Write(addr, val);
	} else {
		uint8_t val = 0;
		RES<bit>(val);
	}
}

template<uint8_t bit>
void SmsCpu::SET(uint8_t& dst)
{
	uint8_t val = GetCbValue(dst);
	SetCbValue(dst, val | (1 << bit));
}

template<uint8_t bit>
void SmsCpu::SET_Indirect(uint16_t addr)
{
	if(_cbAddress < 0) {
		uint8_t val = Read(addr);
		SET<bit>(val);
		ExecCycles(1);
		Write(addr, val);
	} else {
		uint8_t val = 0;
		SET<bit>(val);
	}
}

void SmsCpu::DAA()
{
	uint8_t originalValue = _state.A;
	if(CheckFlag(SmsCpuFlags::Carry) || _state.A > 0x99) {
		_state.A += CheckFlag(SmsCpuFlags::AddSub) ? -0x60 : 0x60;
		SetFlag(SmsCpuFlags::Carry);
	}
	if(CheckFlag(SmsCpuFlags::HalfCarry) || (_state.A & 0x0F) > 0x09) {
		_state.A += CheckFlag(SmsCpuFlags::AddSub) ? -0x06 : 0x06;
	}
	SetFlagState(SmsCpuFlags::HalfCarry, (originalValue ^ _state.A) & 0x10);
	SetStandardFlags<0xEC>(_state.A);
}

void SmsCpu::JP(uint16_t dstAddr)
{
	_state.PC = dstAddr;
}

void SmsCpu::JP(bool condition, uint16_t dstAddr)
{
	_state.WZ = dstAddr;
	if(condition) {
		_state.PC = dstAddr;
	}
}

void SmsCpu::JR(int8_t offset)
{
	ExecCycles(5);
	_state.PC += offset;
}

void SmsCpu::JR(bool condition, int8_t offset)
{
	if(condition) {
		ExecCycles(5);
		_state.PC += offset;
		_state.WZ = _state.PC;
	}
}

void SmsCpu::CALL(uint16_t dstAddr)
{
	ExecCycles(1);
	PushWord(_state.PC);
	_state.PC = dstAddr;
	_state.WZ = _state.PC;
}

void SmsCpu::CALL(bool condition, uint16_t dstAddr)
{
	_state.WZ = dstAddr;
	if(condition) {
		ExecCycles(1);
		PushWord(_state.PC);
		_state.PC = dstAddr;
	}
}

void SmsCpu::RET()
{
	_state.PC = PopWord();
	_state.WZ = _state.PC;
}

void SmsCpu::RET(bool condition)
{
	ExecCycles(1);
	if(condition) {
		_state.PC = PopWord();
		_state.WZ = _state.PC;
	}
}

void SmsCpu::RETI()
{
	_state.IFF1 = _state.IFF2;
	_state.PC = PopWord();
	_state.WZ = _state.PC;
}

void SmsCpu::RST(uint8_t value)
{
	ExecCycles(1);
	PushWord(_state.PC);
	_state.PC = value;
	_state.WZ = _state.PC;
}

void SmsCpu::POP(Register16& reg)
{
	reg.Write(PopWord());
}

void SmsCpu::PUSH(Register16& reg)
{
	ExecCycles(1);
	PushWord(reg);
}

void SmsCpu::POP_AF()
{
	_regAF.Write(PopWord());
}

void SmsCpu::SCF()
{
	SetFlag(SmsCpuFlags::Carry);
	ClearFlag(SmsCpuFlags::AddSub);
	ClearFlag(SmsCpuFlags::HalfCarry);

	//Check if the previous instruction is an instruction that affects the flags
	if(_state.FlagsChanged & 0x02) {
		SetStandardFlags<0x28>(_state.A);
	} else {
		SetStandardFlags<0x28>(_state.A | _state.Flags);
	}
}

void SmsCpu::CCF()
{
	SetFlagState(SmsCpuFlags::HalfCarry, CheckFlag(SmsCpuFlags::Carry));
	_state.Flags ^= SmsCpuFlags::Carry;
	ClearFlag(SmsCpuFlags::AddSub);
	
	//Check if the previous instruction is an instruction that affects the flags
	if(_state.FlagsChanged & 0x02) {
		SetStandardFlags<0x28>(_state.A);
	} else {
		SetStandardFlags<0x28>(_state.A | _state.Flags);
	}
}

void SmsCpu::IM(uint8_t mode)
{
	_state.IM = mode;
}

void SmsCpu::EI()
{
	//"EI also explicitely supresses maskable interrupts in the second half of its opcode fetch machine cycle."
	_state.IFF1 = true;
	_state.IFF2 = true;
}

void SmsCpu::DI()
{
	_state.IFF1 = false;
	_state.IFF2 = false;
}

void SmsCpu::SetNmiLevel(bool nmiLevel)
{
	if(!_state.NmiLevel && nmiLevel) {
		_state.NmiPending = true;
	}
	_state.NmiLevel = nmiLevel;
}

template<uint8_t prefix>
void SmsCpu::PREFIX_CB()
{
	switch(prefix) {
		default: _cbAddress = -1; break;
		case 0xDD: _state.WZ = _cbAddress = (uint16_t)(_regIX + (int8_t)ReadCode()); break;
		case 0xFD: _state.WZ = _cbAddress = (uint16_t)(_regIY + (int8_t)ReadCode()); break;
	}

	uint8_t opCode = prefix == 0 ? ReadNextOpCode() : ReadCode();
	if constexpr(prefix != 0) {
		ExecCycles(2);
	}

	switch(opCode) {
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
		case 0x30: SLL(_state.B); break;
		case 0x31: SLL(_state.C); break;
		case 0x32: SLL(_state.D); break;
		case 0x33: SLL(_state.E); break;
		case 0x34: SLL(_state.H); break;
		case 0x35: SLL(_state.L); break;
		case 0x36: SLL_Indirect(_regHL); break;
		case 0x37: SLL(_state.A); break;
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
		case 0x46: BIT_Indirect<0>(_regHL); break;
		case 0x47: BIT<0>(_state.A); break;
		case 0x48: BIT<1>(_state.B); break;
		case 0x49: BIT<1>(_state.C); break;
		case 0x4A: BIT<1>(_state.D); break;
		case 0x4B: BIT<1>(_state.E); break;
		case 0x4C: BIT<1>(_state.H); break;
		case 0x4D: BIT<1>(_state.L); break;
		case 0x4E: BIT_Indirect<1>(_regHL); break;
		case 0x4F: BIT<1>(_state.A); break;
		case 0x50: BIT<2>(_state.B); break;
		case 0x51: BIT<2>(_state.C); break;
		case 0x52: BIT<2>(_state.D); break;
		case 0x53: BIT<2>(_state.E); break;
		case 0x54: BIT<2>(_state.H); break;
		case 0x55: BIT<2>(_state.L); break;
		case 0x56: BIT_Indirect<2>(_regHL); break;
		case 0x57: BIT<2>(_state.A); break;
		case 0x58: BIT<3>(_state.B); break;
		case 0x59: BIT<3>(_state.C); break;
		case 0x5A: BIT<3>(_state.D); break;
		case 0x5B: BIT<3>(_state.E); break;
		case 0x5C: BIT<3>(_state.H); break;
		case 0x5D: BIT<3>(_state.L); break;
		case 0x5E: BIT_Indirect<3>(_regHL); break;
		case 0x5F: BIT<3>(_state.A); break;
		case 0x60: BIT<4>(_state.B); break;
		case 0x61: BIT<4>(_state.C); break;
		case 0x62: BIT<4>(_state.D); break;
		case 0x63: BIT<4>(_state.E); break;
		case 0x64: BIT<4>(_state.H); break;
		case 0x65: BIT<4>(_state.L); break;
		case 0x66: BIT_Indirect<4>(_regHL); break;
		case 0x67: BIT<4>(_state.A); break;
		case 0x68: BIT<5>(_state.B); break;
		case 0x69: BIT<5>(_state.C); break;
		case 0x6A: BIT<5>(_state.D); break;
		case 0x6B: BIT<5>(_state.E); break;
		case 0x6C: BIT<5>(_state.H); break;
		case 0x6D: BIT<5>(_state.L); break;
		case 0x6E: BIT_Indirect<5>(_regHL); break;
		case 0x6F: BIT<5>(_state.A); break;
		case 0x70: BIT<6>(_state.B); break;
		case 0x71: BIT<6>(_state.C); break;
		case 0x72: BIT<6>(_state.D); break;
		case 0x73: BIT<6>(_state.E); break;
		case 0x74: BIT<6>(_state.H); break;
		case 0x75: BIT<6>(_state.L); break;
		case 0x76: BIT_Indirect<6>(_regHL); break;
		case 0x77: BIT<6>(_state.A); break;
		case 0x78: BIT<7>(_state.B); break;
		case 0x79: BIT<7>(_state.C); break;
		case 0x7A: BIT<7>(_state.D); break;
		case 0x7B: BIT<7>(_state.E); break;
		case 0x7C: BIT<7>(_state.H); break;
		case 0x7D: BIT<7>(_state.L); break;
		case 0x7E: BIT_Indirect<7>(_regHL); break;
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

void SmsCpu::InitPostBiosState()
{
	_state.SP = 0xDFF0;
	_regAF.Write(0xAB42);
	_regBC.Write(0x0002);
	_regDE.Write(0xA51B);
	_regHL.Write(0x0000);
	_state.AltB = 0x02;
	_state.AltD = 0xC7;
	_state.AltH = 0xC7;
	_state.AltL = 0x5F;
	_state.IM = 1;
	_state.R = 29;
}

void SmsCpu::Serialize(Serializer& s)
{
	SV(_state.CycleCount);
	SV(_state.PC);
	SV(_state.SP);

	SV(_state.A);
	SV(_state.Flags);
	SV(_state.B);
	SV(_state.C);
	SV(_state.D);
	SV(_state.E);
	SV(_state.H);
	SV(_state.L);
	SV(_state.IXL);
	SV(_state.IXH);
	SV(_state.IYL);
	SV(_state.IYH);
	SV(_state.I);
	SV(_state.R);

	SV(_state.AltA);
	SV(_state.AltFlags);
	SV(_state.AltB);
	SV(_state.AltC);
	SV(_state.AltD);
	SV(_state.AltE);
	SV(_state.AltH);
	SV(_state.AltL);

	SV(_state.ActiveIrqs);
	SV(_state.NmiLevel);
	SV(_state.NmiPending);
	SV(_state.Halted);

	SV(_state.IFF1);
	SV(_state.IFF2);

	SV(_state.IM);
	SV(_state.FlagsChanged);
	SV(_state.WZ);
}