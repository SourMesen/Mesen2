#include "stdafx.h"
#include "Spc.h"
#include "../Utilities/HexUtilities.h"

void Spc::Exec()
{
	uint8_t opCode = GetOpCode();
	switch(opCode) {
		case 0x00: NOP(); break;
		case 0x01: TCALL<0>(); break;
		case 0x02: Addr_Dir(); SET1<0>(); break;
		case 0x03: Addr_Dir(); BBS<0>(); break;
		case 0x04: Addr_Dir(); OR_Acc(); break;
		case 0x05: Addr_Abs(); OR_Acc(); break;
		case 0x06: Addr_IndX(); OR_Acc(); break;
		case 0x07: Addr_DirIdxXInd(); OR_Acc(); break;
		case 0x08: Addr_Imm(); OR_Acc(); _immediateMode = false; break;
		case 0x09: Addr_DirToDir(); OR(); break;
		case 0x0A: Addr_AbsBit(); OR1(); break;
		case 0x0B: Addr_Dir(); ASL(); break;
		case 0x0C: Addr_Abs(); ASL(); break;
		case 0x0D: PHP(); break;
		case 0x0E: Addr_Abs(); TSET1(); break;
		case 0x0F: BRK(); break;
		case 0x10: Addr_Rel(); BPL(); break;
		case 0x11: TCALL<1>(); break;
		case 0x12: Addr_Dir(); CLR1<0>(); break;
		case 0x13: Addr_Dir(); BBC<0>(); break;
		case 0x14: Addr_DirIdxX(); OR_Acc(); break;
		case 0x15: Addr_AbsIdxX(); OR_Acc(); break;
		case 0x16: Addr_AbsIdxY(); OR_Acc(); break;
		case 0x17: Addr_DirIndIdxY(); OR_Acc(); break;
		case 0x18: Addr_DirImm(); OR(); break;
		case 0x19: Addr_IndXToIndY(); OR(); break;
		case 0x1A: Addr_Dir(); DECW(); break;
		case 0x1B: Addr_DirIdxX(); ASL(); break;
		case 0x1C: ASL_Acc(); break;
		case 0x1D: DEX(); break;
		case 0x1E: Addr_Abs(); CPX(); break;
		case 0x1F: Addr_AbsIdxXInd(); JMP(); break;
		case 0x20: CLRP(); break;
		case 0x21: TCALL<2>(); break;
		case 0x22: Addr_Dir(); SET1<1>(); break;
		case 0x23: Addr_Dir(); BBS<1>(); break;
		case 0x24: Addr_Dir(); AND_Acc(); break;
		case 0x25: Addr_Abs(); AND_Acc(); break;
		case 0x26: Addr_IndX(); AND_Acc(); break;
		case 0x27: Addr_DirIdxXInd(); AND_Acc(); break;
		case 0x28: Addr_Imm(); AND_Acc(); _immediateMode = false; break;
		case 0x29: Addr_DirToDir(); AND(); break;
		case 0x2A: Addr_AbsBit(); NOR1(); break;
		case 0x2B: Addr_Dir(); ROL(); break;
		case 0x2C: Addr_Abs(); ROL(); break;
		case 0x2D: PHA(); break;
		case 0x2E: Addr_Dir(); CBNE(); break;
		case 0x2F: Addr_Rel(); BRA(); break;
		case 0x30: Addr_Rel(); BMI(); break;
		case 0x31: TCALL<3>(); break;
		case 0x32: Addr_Dir(); CLR1<1>(); break;
		case 0x33: Addr_Dir(); BBC<1>(); break;
		case 0x34: Addr_DirIdxX(); AND_Acc(); break;
		case 0x35: Addr_AbsIdxX(); AND_Acc(); break;
		case 0x36: Addr_AbsIdxY(); AND_Acc(); break;
		case 0x37: Addr_DirIndIdxY(); AND_Acc(); break;
		case 0x38: Addr_DirImm(); AND(); break;
		case 0x39: Addr_IndXToIndY(); AND(); break;
		case 0x3A: Addr_Dir(); INCW(); break;
		case 0x3B: Addr_DirIdxX(); ROL(); break;
		case 0x3C: ROL_Acc(); break;
		case 0x3D: INX(); break;
		case 0x3E: Addr_Dir(); CPX(); break;
		case 0x3F: Addr_Abs(); JSR(); break;
		case 0x40: SETP(); break;
		case 0x41: TCALL<4>(); break;
		case 0x42: Addr_Dir(); SET1<2>(); break;
		case 0x43: Addr_Dir(); BBS<2>(); break;
		case 0x44: Addr_Dir(); EOR_Acc(); break;
		case 0x45: Addr_Abs(); EOR_Acc(); break;
		case 0x46: Addr_IndX(); EOR_Acc(); break;
		case 0x47: Addr_DirIdxXInd(); EOR_Acc(); break;
		case 0x48: Addr_Imm(); EOR_Acc(); _immediateMode = false; break;
		case 0x49: Addr_DirToDir(); EOR(); break;
		case 0x4A: Addr_AbsBit(); AND1(); break;
		case 0x4B: Addr_Dir(); LSR(); break;
		case 0x4C: Addr_Abs(); LSR(); break;
		case 0x4D: PHX(); break;
		case 0x4E: Addr_Abs(); TCLR1(); break;
		case 0x4F: PCALL(); break;
		case 0x50: Addr_Rel(); BVC(); break;
		case 0x51: TCALL<5>(); break;
		case 0x52: Addr_Dir(); CLR1<2>(); break;
		case 0x53: Addr_Dir(); BBC<2>(); break;
		case 0x54: Addr_DirIdxX(); EOR_Acc(); break;
		case 0x55: Addr_AbsIdxX(); EOR_Acc(); break;
		case 0x56: Addr_AbsIdxY(); EOR_Acc(); break;
		case 0x57: Addr_DirIndIdxY(); EOR_Acc(); break;
		case 0x58: Addr_DirImm(); EOR(); break;
		case 0x59: Addr_IndXToIndY(); EOR(); break;
		case 0x5A: Addr_Dir(); CMPW(); break;
		case 0x5B: Addr_DirIdxX(); LSR(); break;
		case 0x5C: LSR_Acc(); break;
		case 0x5D: TAX(); break;
		case 0x5E: Addr_Abs(); CPY(); break;
		case 0x5F: Addr_Abs(); JMP(); break;
		case 0x60: CLRC(); break;
		case 0x61: TCALL<6>(); break;
		case 0x62: Addr_Dir(); SET1<3>(); break;
		case 0x63: Addr_Dir(); BBS<3>(); break;
		case 0x64: Addr_Dir(); CMP_Acc(); break;
		case 0x65: Addr_Abs(); CMP_Acc(); break;
		case 0x66: Addr_IndX(); CMP_Acc(); break;
		case 0x67: Addr_DirIdxXInd(); CMP_Acc(); break;
		case 0x68: Addr_Imm(); CMP_Acc(); _immediateMode = false; break;
		case 0x69: Addr_DirToDir(); CMP(); break;
		case 0x6A: Addr_AbsBit(); NAND1(); break;
		case 0x6B: Addr_Dir(); ROR(); break;
		case 0x6C: Addr_Abs(); ROR(); break;
		case 0x6D: PHY(); break;
		case 0x6E: Addr_Dir(); DBNZ(); break;
		case 0x6F: RTS(); break;
		case 0x70: Addr_Rel(); BVS(); break;
		case 0x71: TCALL<7>(); break;
		case 0x72: Addr_Dir(); CLR1<3>(); break;
		case 0x73: Addr_Dir(); BBC<3>(); break;
		case 0x74: Addr_DirIdxX(); CMP_Acc(); break;
		case 0x75: Addr_AbsIdxX(); CMP_Acc(); break;
		case 0x76: Addr_AbsIdxY(); CMP_Acc(); break;
		case 0x77: Addr_DirIndIdxY(); CMP_Acc(); break;
		case 0x78: Addr_DirImm(); CMP(); break;
		case 0x79: Addr_IndXToIndY(); CMP(); break;
		case 0x7A: Addr_Dir(); ADDW(); break;
		case 0x7B: Addr_DirIdxX(); ROR(); break;
		case 0x7C: ROR_Acc(); break;
		case 0x7D: TXA(); break;
		case 0x7E: Addr_Dir(); CPY(); break;
		case 0x7F: RTI(); break;
		case 0x80: SETC(); break;
		case 0x81: TCALL<8>(); break;
		case 0x82: Addr_Dir(); SET1<4>(); break;
		case 0x83: Addr_Dir(); BBS<4>(); break;
		case 0x84: Addr_Dir(); ADC_Acc(); break;
		case 0x85: Addr_Abs(); ADC_Acc(); break;
		case 0x86: Addr_IndX(); ADC_Acc(); break;
		case 0x87: Addr_DirIdxXInd(); ADC_Acc(); break;
		case 0x88: Addr_Imm(); ADC_Acc(); _immediateMode = false; break;
		case 0x89: Addr_DirToDir(); ADC(); break;
		case 0x8A: Addr_AbsBit(); EOR1(); break;
		case 0x8B: Addr_Dir(); DEC(); break;
		case 0x8C: Addr_Abs(); DEC(); break;
		case 0x8D: Addr_Imm(); LDY(); _immediateMode = false; break;
		case 0x8E: PLP(); break;
		case 0x8F: Addr_DirImm(); DummyRead(_operandB); MOV(); break;
		case 0x90: Addr_Rel(); BCC(); break;
		case 0x91: TCALL<9>(); break;
		case 0x92: Addr_Dir(); CLR1<4>(); break;
		case 0x93: Addr_Dir(); BBC<4>(); break;
		case 0x94: Addr_DirIdxX(); ADC_Acc(); break;
		case 0x95: Addr_AbsIdxX(); ADC_Acc(); break;
		case 0x96: Addr_AbsIdxY(); ADC_Acc(); break;
		case 0x97: Addr_DirIndIdxY(); ADC_Acc(); break;
		case 0x98: Addr_DirImm(); ADC(); break;
		case 0x99: Addr_IndXToIndY(); ADC(); break;
		case 0x9A: Addr_Dir(); SUBW(); break;
		case 0x9B: Addr_DirIdxX(); DEC(); break;
		case 0x9C: DEC_Acc(); break;
		case 0x9D: TSX(); break;
		case 0x9E: DIV(); break;
		case 0x9F: XCN(); break;
		case 0xA0: EI(); break;
		case 0xA1: TCALL<10>(); break;
		case 0xA2: Addr_Dir(); SET1<5>(); break;
		case 0xA3: Addr_Dir(); BBS<5>(); break;
		case 0xA4: Addr_Dir(); SBC_Acc(); break;
		case 0xA5: Addr_Abs(); SBC_Acc(); break;
		case 0xA6: Addr_IndX(); SBC_Acc(); break;
		case 0xA7: Addr_DirIdxXInd(); SBC_Acc(); break;
		case 0xA8: Addr_Imm(); SBC_Acc(); _immediateMode = false; break;
		case 0xA9: Addr_DirToDir(); SBC(); break;
		case 0xAA: Addr_AbsBit(); LDC(); break;
		case 0xAB: Addr_Dir(); INC(); break;
		case 0xAC: Addr_Abs(); INC(); break;
		case 0xAD: Addr_Imm(); CPY(); _immediateMode = false; break;
		case 0xAE: PLA(); break;
		case 0xAF: Addr_IndX(); STA_AutoIncX(); break;
		case 0xB0: Addr_Rel(); BCS(); break;
		case 0xB1: TCALL<11>(); break;
		case 0xB2: Addr_Dir(); CLR1<5>(); break;
		case 0xB3: Addr_Dir(); BBC<5>(); break;
		case 0xB4: Addr_DirIdxX(); SBC_Acc(); break;
		case 0xB5: Addr_AbsIdxX(); SBC_Acc(); break;
		case 0xB6: Addr_AbsIdxY(); SBC_Acc(); break;
		case 0xB7: Addr_DirIndIdxY(); SBC_Acc(); break;
		case 0xB8: Addr_DirImm(); SBC(); break;
		case 0xB9: Addr_IndXToIndY(); SBC(); break;
		case 0xBA: Addr_Dir(); LDW(); break;
		case 0xBB: Addr_DirIdxX(); INC(); break;
		case 0xBC: INC_Acc(); break;
		case 0xBD: TXS(); break;
		case 0xBE: DAS(); break;
		case 0xBF: Addr_IndX(); LDA_AutoIncX(); break;
		case 0xC0: DI(); break;
		case 0xC1: TCALL<12>(); break;
		case 0xC2: Addr_Dir(); SET1<6>(); break;
		case 0xC3: Addr_Dir(); BBS<6>(); break;
		case 0xC4: Addr_Dir(); STA(); break;
		case 0xC5: Addr_Abs(); STA(); break;
		case 0xC6: Addr_IndX(); STA(); break;
		case 0xC7: Addr_DirIdxXInd(); STA(); break;
		case 0xC8: Addr_Imm(); CPX(); _immediateMode = false; break;
		case 0xC9: Addr_Abs(); STX(); break;
		case 0xCA: Addr_AbsBit(); STC(); break;
		case 0xCB: Addr_Dir(); STY(); break;
		case 0xCC: Addr_Abs(); STY(); break;
		case 0xCD: Addr_Imm(); LDX(); _immediateMode = false; break;
		case 0xCE: PLX(); break;
		case 0xCF: MUL(); break;
		case 0xD0: Addr_Rel(); BNE(); break;
		case 0xD1: TCALL<13>(); break;
		case 0xD2: Addr_Dir(); CLR1<6>(); break;
		case 0xD3: Addr_Dir(); BBC<6>(); break;
		case 0xD4: Addr_DirIdxX(); STA(); break;
		case 0xD5: Addr_AbsIdxX(); STA(); break;
		case 0xD6: Addr_AbsIdxY(); STA(); break;
		case 0xD7: Addr_DirIndIdxY(); STA(); break;
		case 0xD8: Addr_Dir(); STX(); break;
		case 0xD9: Addr_DirIdxY(); STX(); break;
		case 0xDA: Addr_Dir(); STW(); break;
		case 0xDB: Addr_DirIdxX(); STY(); break;
		case 0xDC: DEY(); break;
		case 0xDD: TYA(); break;
		case 0xDE: Addr_DirIdxX(); CBNE(); break;
		case 0xDF: DAA(); break;
		case 0xE0: CLRV(); break;
		case 0xE1: TCALL<14>(); break;
		case 0xE2: Addr_Dir(); SET1<7>(); break;
		case 0xE3: Addr_Dir(); BBS<7>(); break;
		case 0xE4: Addr_Dir(); LDA(); break;
		case 0xE5: Addr_Abs(); LDA(); break;
		case 0xE6: Addr_IndX(); LDA(); break;
		case 0xE7: Addr_DirIdxXInd(); LDA(); break;
		case 0xE8: Addr_Imm(); LDA(); _immediateMode = false; break;
		case 0xE9: Addr_Abs(); LDX(); break;
		case 0xEA: Addr_AbsBit(); NOT1(); break;
		case 0xEB: Addr_Dir(); LDY(); break;
		case 0xEC: Addr_Abs(); LDY(); break;
		case 0xED: NOTC(); break;
		case 0xEE: PLY(); break;
		case 0xEF: SLEEP(); break;
		case 0xF0: Addr_Rel(); BEQ(); break;
		case 0xF1: TCALL<15>(); break;
		case 0xF2: Addr_Dir(); CLR1<7>(); break;
		case 0xF3: Addr_Dir(); BBC<7>(); break;
		case 0xF4: Addr_DirIdxX(); LDA(); break;
		case 0xF5: Addr_AbsIdxX(); LDA(); break;
		case 0xF6: Addr_AbsIdxY(); LDA(); break;
		case 0xF7: Addr_DirIndIdxY(); LDA(); break;
		case 0xF8: Addr_Dir(); LDX(); break;
		case 0xF9: Addr_DirIdxY(); LDX(); break;
		case 0xFA: Addr_DirToDir(); MOV(); break;
		case 0xFB: Addr_DirIdxX(); LDY(); break;
		case 0xFC: INY(); break;
		case 0xFD: TAY(); break;
		case 0xFE: DBNZ_Y(); break;
		case 0xFF: STOP(); break;
	}
}

//*****************
// ADDRESSING MODES
//*****************
void Spc::Addr_Dir()
{
	_operandA = GetDirectAddress(ReadOperandByte());
}

void Spc::Addr_DirIdxX()
{
	_operandA = GetDirectAddress(ReadOperandByte() + _state.X);
	Idle();
}

void Spc::Addr_DirIdxY()
{
	_operandA = GetDirectAddress(ReadOperandByte() + _state.Y);
	Idle();
}

void Spc::Addr_DirToDir()
{
	_operandA = Read(GetDirectAddress(ReadOperandByte()));
	_operandB = GetDirectAddress(ReadOperandByte());
}

void Spc::Addr_DirImm()
{
	_operandA = ReadOperandByte();
	_operandB = GetDirectAddress(ReadOperandByte());
}

void Spc::Addr_DirIdxXInd()
{
	uint16_t addr = GetDirectAddress(ReadOperandByte() + _state.X);
	Idle();
	uint8_t lsb = Read(addr);
	uint8_t msb = Read(GetDirectAddress(addr + 1));
	_operandA = (msb << 8) | lsb;
}

void Spc::Addr_DirIndIdxY()
{
	uint16_t addr = GetDirectAddress(ReadOperandByte());
	uint8_t lsb = Read(addr);
	uint8_t msb = Read(GetDirectAddress(addr + 1));
	Idle();
	_operandA = ((msb << 8) | lsb) + _state.Y;
}

void Spc::Addr_IndX()
{
	DummyRead();
	_operandA = GetDirectAddress(_state.X);
}

void Spc::Addr_IndXToIndY()
{
	DummyRead();
	_operandA = Read(GetDirectAddress(_state.Y));
	_operandB = GetDirectAddress(_state.X);
}

void Spc::Addr_Abs()
{
	uint8_t lsb = ReadOperandByte();
	uint8_t msb = ReadOperandByte();
	_operandA = ((msb << 8) | lsb);
}

void Spc::Addr_AbsBit()
{
	Addr_Abs();
	_operandB = _operandA >> 13;
	_operandA = _operandA & 0x1FFF;
}

void Spc::Addr_AbsIdxX()
{
	Addr_Abs();
	Idle();
	_operandA += _state.X;
}

void Spc::Addr_AbsIdxY()
{
	Addr_Abs();
	Idle();
	_operandA += _state.Y;
}

void Spc::Addr_AbsIdxXInd()
{
	//Used by JMP only
	uint8_t lsb = ReadOperandByte();
	uint8_t msb = ReadOperandByte();
	Idle();
	uint16_t addr = ((msb << 8) | lsb);
	uint8_t addrLsb = Read(addr + _state.X);
	uint8_t addrMsb = Read(addr + _state.X + 1);
	_operandA = (addrMsb << 8) | addrLsb;
}

void Spc::Addr_Rel()
{
	_operandA = ReadOperandByte();
}

void Spc::Addr_Imm()
{
	_immediateMode = true;
	_operandA = ReadOperandByte();
}

//*****************
// INSTRUCTIONS
//*****************
void Spc::STA()
{
	Read(_operandA);
	Write(_operandA, _state.A);
}

void Spc::STX()
{
	Read(_operandA);
	Write(_operandA, _state.X);
}

void Spc::STY()
{
	Read(_operandA);
	Write(_operandA, _state.Y);
}

void Spc::STW()
{
	DummyRead(_operandA);

	Write(_operandA, _state.A);

	uint16_t msbAddress = GetDirectAddress(_operandA + 1);
	Write(msbAddress, _state.Y);
}

void Spc::STA_AutoIncX()
{
	Idle();
	Write(_operandA, _state.A);
	_state.X++;
}

void Spc::LDA_AutoIncX()
{
	_state.A = Read(_operandA);
	SetZeroNegativeFlags(_state.A);
	Idle();
	_state.X++;
}

void Spc::LDA()
{
	_state.A = GetByteValue();
	SetZeroNegativeFlags(_state.A);
}

void Spc::LDX()
{
	_state.X = GetByteValue();
	SetZeroNegativeFlags(_state.X);
}

void Spc::LDY()
{
	_state.Y = GetByteValue();
	SetZeroNegativeFlags(_state.Y);
}

void Spc::LDW()
{
	uint8_t lsb = Read(_operandA);
	Idle();
	uint8_t msb = Read(GetDirectAddress(_operandA + 1));

	uint16_t value = (msb << 8) | lsb;
	_state.A = (uint8_t)value;
	_state.Y = (value >> 8);
	SetZeroNegativeFlags16(value);
}

void Spc::TXA()
{
	DummyRead();
	_state.A = _state.X;
	SetZeroNegativeFlags(_state.A);
}

void Spc::TYA()
{
	DummyRead();
	_state.A = _state.Y;
	SetZeroNegativeFlags(_state.A);
}

void Spc::TAX()
{
	DummyRead();
	_state.X = _state.A;
	SetZeroNegativeFlags(_state.X);
}

void Spc::TAY()
{
	DummyRead();
	_state.Y = _state.A;
	SetZeroNegativeFlags(_state.Y);
}

void Spc::TSX()
{
	DummyRead();
	_state.X = _state.SP;
	SetZeroNegativeFlags(_state.X);
}

void Spc::TXS()
{
	DummyRead();
	_state.SP = _state.X;
}

void Spc::MOV()
{
	Write(_operandB, (uint8_t)_operandA);
}

uint8_t Spc::Add(uint8_t a, uint8_t b)
{
	uint32_t result = a + b + (_state.PS & SpcFlags::Carry);
	uint8_t subResult = (a & 0x0F) + (_state.PS & SpcFlags::Carry);

	ClearFlags(SpcFlags::Carry | SpcFlags::Negative | SpcFlags::Zero | SpcFlags::Overflow | SpcFlags::HalfCarry);

	if(~(a ^ b) & (a ^ result) & 0x80) {
		SetFlags(SpcFlags::Overflow);
	}
	if(result > 0xFF) {
		SetFlags(SpcFlags::Carry);
	}
	if(((result & 0x0F) - subResult) & 0x10) {
		SetFlags(SpcFlags::HalfCarry);
	}
	SetZeroNegativeFlags((uint8_t)result);

	return (uint8_t)result;
}

uint8_t Spc::Sub(uint8_t a, uint8_t b)
{
	uint32_t carryCalc = a - b - ((_state.PS & SpcFlags::Carry) ^ 0x01);
	uint8_t result = Add(a, ~b);

	if(carryCalc <= 0xFF) {
		SetFlags(SpcFlags::Carry);
	} else {
		ClearFlags(SpcFlags::Carry);
	}

	return (uint8_t)result;
}

void Spc::ADC()
{
	uint8_t val1 = (uint8_t)_operandA;
	uint8_t val2 = Read(_operandB);
	Write(_operandB, Add(val2, val1));
}

void Spc::ADC_Acc()
{
	_state.A = Add(_state.A, GetByteValue());
}

void Spc::ADDW()
{
	uint8_t lsb = Read(_operandA);
	Idle();
	uint8_t msb = Read(GetDirectAddress(_operandA + 1));
	uint16_t value = ((msb << 8) | lsb);

	uint8_t lowCarry = (lsb + _state.A) > 0xFF ? 1 : 0;
	ClearFlags(SpcFlags::Carry | SpcFlags::HalfCarry | SpcFlags::Overflow);
	if(((_state.Y & 0x0F) + (msb & 0x0F) + lowCarry) & 0x10) {
		SetFlags(SpcFlags::HalfCarry);
	}

	uint16_t ya = (_state.Y << 8) | _state.A;
	uint32_t result = ya + value;
	if(result > 0xFFFF) {
		SetFlags(SpcFlags::Carry);
	}
	SetZeroNegativeFlags16(result);

	if(~(ya ^ value) & (ya ^ result) & 0x8000) {
		SetFlags(SpcFlags::Overflow);
	}

	_state.Y = result >> 8;
	_state.A = (uint8_t)result;
}

void Spc::SBC()
{
	uint8_t val1 = (uint8_t)_operandA;
	uint8_t val2 = Read(_operandB);
	Write(_operandB, Sub(val2, val1));
}

void Spc::SBC_Acc()
{
	_state.A = Sub(_state.A, GetByteValue());
}

void Spc::SUBW()
{
	uint8_t lsb = Read(_operandA);
	Idle();
	uint8_t msb = Read(GetDirectAddress(_operandA + 1));
	uint16_t value = ((msb << 8) | lsb);
	uint16_t ya = (_state.Y << 8) | _state.A;

	uint32_t l = _state.A - lsb;
	uint8_t carry = l > 0xFF ? 1 : 0;
	uint32_t h = _state.Y - msb - carry;

	ClearFlags(SpcFlags::Carry | SpcFlags::HalfCarry | SpcFlags::Overflow);
	if(h <= 0xFF) {
		SetFlags(SpcFlags::Carry);
	}

	if((((_state.Y & 0x0F) - (msb & 0x0F) - carry) & 0x10) == 0) {
		SetFlags(SpcFlags::HalfCarry);
	}

	_state.Y = h;
	_state.A = l;

	uint16_t result = (_state.Y << 8) | _state.A;

	if((ya ^ value) & (ya ^ result) & 0x8000) {
		SetFlags(SpcFlags::Overflow);
	}

	SetZeroNegativeFlags16(result);
}

void Spc::Compare(uint8_t a, uint8_t b)
{
	if(a >= b) {
		SetFlags(SpcFlags::Carry);
	} else {
		ClearFlags(SpcFlags::Carry);
	}

	uint8_t result = a - b;
	SetZeroNegativeFlags(result);
}

void Spc::CMP()
{
	Compare(Read(_operandB), (uint8_t)_operandA);
	Idle();
}

void Spc::CMP_Acc()
{
	Compare(_state.A, GetByteValue());
}

void Spc::CPX()
{
	Compare(_state.X, GetByteValue());
}

void Spc::CPY()
{
	Compare(_state.Y, GetByteValue());
}

void Spc::CMPW()
{
	uint8_t lsb = Read(_operandA);
	uint8_t msb = Read(GetDirectAddress(_operandA + 1));
	uint16_t value = ((msb << 8) | lsb);

	uint16_t ya = (_state.Y << 8) | _state.A;

	if(ya >= value) {
		SetFlags(SpcFlags::Carry);
	} else {
		ClearFlags(SpcFlags::Carry);
	}

	uint16_t result = ya - value;
	SetZeroNegativeFlags16(result);
}

void Spc::INC()
{
	uint8_t result = Read(_operandA) + 1;
	Write(_operandA, result);
	SetZeroNegativeFlags(result);
}

void Spc::INC_Acc()
{
	DummyRead();
	_state.A++;
	SetZeroNegativeFlags(_state.A);
}

void Spc::INX()
{
	DummyRead();
	_state.X++;
	SetZeroNegativeFlags(_state.X);
}

void Spc::INY()
{
	DummyRead();
	_state.Y++;
	SetZeroNegativeFlags(_state.Y);
}

void Spc::INCW()
{
	uint8_t lsb = Read(_operandA);
	Write(_operandA, lsb + 1);

	uint16_t msbAddress = GetDirectAddress(_operandA + 1);
	uint8_t msb = Read(msbAddress);
	uint16_t value = ((msb << 8) | lsb) + 1;
	Write(msbAddress, value >> 8);
	SetZeroNegativeFlags16(value);
}

void Spc::DEC()
{
	uint8_t result = Read(_operandA) - 1;
	Write(_operandA, result);
	SetZeroNegativeFlags(result);
}

void Spc::DEC_Acc()
{
	DummyRead();
	_state.A--;
	SetZeroNegativeFlags(_state.A);
}

void Spc::DEX()
{
	DummyRead();
	_state.X--;
	SetZeroNegativeFlags(_state.X);
}

void Spc::DEY()
{
	DummyRead();
	_state.Y--;
	SetZeroNegativeFlags(_state.Y);
}

void Spc::DECW()
{
	uint8_t lsb = Read(_operandA);
	Write(_operandA, lsb - 1);

	uint16_t msbAddress = GetDirectAddress(_operandA + 1);
	uint8_t msb = Read(msbAddress);
	uint16_t value = ((msb << 8) | lsb) - 1;
	Write(msbAddress, value >> 8);
	SetZeroNegativeFlags16(value);
}

void Spc::MUL()
{
	uint16_t result = _state.Y * _state.A;
	_state.Y = result >> 8;
	_state.A = (uint8_t)result;

	DummyRead();
	Idle();
	Idle();
	Idle();
	Idle();
	Idle();
	Idle();
	Idle();

	SetZeroNegativeFlags(_state.Y);
}

void Spc::DIV()
{
	uint32_t ya = (_state.Y << 8) | _state.A;
	uint32_t sub = _state.X << 9;

	DummyRead();

	for(int i = 0; i < 9; i++) {
		if(ya & 0x10000) {
			ya = ((ya << 1) | 0x01) & 0x1FFFF;
		} else {
			ya = (ya << 1) & 0x1FFFF;
		}

		if(ya >= sub) {
			ya ^= 0x01;
		}

		if(ya & 0x01) {
			ya = (ya - sub) & 0x1FFFF;
		}

		Idle();
	}

	//12 cycles total
	Idle();

	if((_state.Y & 0x0F) >= (_state.X & 0x0F)) {
		SetFlags(SpcFlags::HalfCarry);
	} else {
		ClearFlags(SpcFlags::HalfCarry);
	}

	_state.A = (uint8_t)ya;
	_state.Y = ya >> 9;

	if(ya & 0x100) {
		SetFlags(SpcFlags::Overflow);
	} else {
		ClearFlags(SpcFlags::Overflow);
	}

	SetZeroNegativeFlags(_state.A);
}

void Spc::DAA()
{
	Idle();
	Idle();

	if(CheckFlag(SpcFlags::Carry) || _state.A > 0x99) {
		_state.A += 0x60;
		SetFlags(SpcFlags::Carry);
	}

	if(CheckFlag(SpcFlags::HalfCarry) || ((_state.A & 0x0F) > 9)) {
		_state.A += 6;
	}

	SetZeroNegativeFlags(_state.A);
}

void Spc::DAS()
{
	Idle();
	Idle();

	if(!CheckFlag(SpcFlags::Carry) || _state.A > 0x99) {
		_state.A -= 0x60;
		ClearFlags(SpcFlags::Carry);
	}

	if(!CheckFlag(SpcFlags::HalfCarry) || ((_state.A & 0x0F) > 9)) {
		_state.A -= 6;
	}

	SetZeroNegativeFlags(_state.A);
}

void Spc::AND()
{
	uint8_t result = _operandA & Read(_operandB);
	Write(_operandB, result);
	SetZeroNegativeFlags(result);
}

void Spc::AND_Acc()
{
	_state.A &= GetByteValue();
	SetZeroNegativeFlags(_state.A);
}

void Spc::OR()
{
	uint8_t result = _operandA | Read(_operandB);
	Write(_operandB, result);
	SetZeroNegativeFlags(result);
}

void Spc::OR_Acc()
{
	_state.A |= GetByteValue();
	SetZeroNegativeFlags(_state.A);
}

void Spc::EOR()
{
	uint8_t result = _operandA ^ Read(_operandB);
	Write(_operandB, result);
	SetZeroNegativeFlags(result);
}

void Spc::EOR_Acc()
{
	_state.A ^= GetByteValue();
	SetZeroNegativeFlags(_state.A);
}

void Spc::SetCarry(uint8_t carry)
{
	_state.PS = (_state.PS & ~SpcFlags::Carry) | carry;
}

void Spc::OR1()
{
	uint8_t carry = _state.PS & SpcFlags::Carry;
	carry |= (Read(_operandA) >> _operandB) & 0x01;
	SetCarry(carry);
	Idle();
}

void Spc::NOR1()
{
	uint8_t carry = _state.PS & SpcFlags::Carry;
	carry |= ~((Read(_operandA) >> _operandB)) & 0x01;
	SetCarry(carry);
	Idle();
}

void Spc::AND1()
{
	uint8_t carry = _state.PS & SpcFlags::Carry;
	carry &= (Read(_operandA) >> _operandB) & 0x01;
	SetCarry(carry);
}

void Spc::NAND1()
{
	uint8_t carry = _state.PS & SpcFlags::Carry;
	carry &= ~((Read(_operandA) >> _operandB)) & 0x01;
	SetCarry(carry);
}

void Spc::EOR1()
{
	uint8_t carry = _state.PS & SpcFlags::Carry;
	carry ^= (Read(_operandA) >> _operandB) & 0x01;
	SetCarry(carry);
	Idle();
}

void Spc::NOT1()
{
	uint8_t value = Read(_operandA);
	uint8_t mask = (1 << _operandB);
	Write(_operandA, value ^ mask);
}

void Spc::STC()
{
	uint8_t value = Read(_operandA);
	uint8_t mask = (1 << _operandB);
	uint8_t carry = (_state.PS & SpcFlags::Carry) << _operandB;
	Idle();
	Write(_operandA, (value & ~mask) | carry);
}

void Spc::LDC()
{
	uint8_t carry = (Read(_operandA) >> _operandB) & 0x01;
	SetCarry(carry);
}

uint8_t Spc::ShiftLeft(uint8_t value)
{
	uint8_t result = value << 1;
	if(value & (1 << 7)) {
		SetFlags(SpcFlags::Carry);
	} else {
		ClearFlags(SpcFlags::Carry);
	}
	SetZeroNegativeFlags(result);
	return result;
}

uint8_t Spc::RollLeft(uint8_t value)
{
	uint8_t result = value << 1 | (_state.PS & SpcFlags::Carry);
	if(value & (1 << 7)) {
		SetFlags(SpcFlags::Carry);
	} else {
		ClearFlags(SpcFlags::Carry);
	}
	SetZeroNegativeFlags(result);
	return result;
}

uint8_t Spc::ShiftRight(uint8_t value)
{
	uint8_t result = value >> 1;
	if(value & 0x01) {
		SetFlags(SpcFlags::Carry);
	} else {
		ClearFlags(SpcFlags::Carry);
	}
	SetZeroNegativeFlags(result);
	return result;
}

uint8_t Spc::RollRight(uint8_t value)
{
	uint8_t result = value >> 1 | ((_state.PS & 0x01) << 7);
	if(value & 0x01) {
		SetFlags(SpcFlags::Carry);
	} else {
		ClearFlags(SpcFlags::Carry);
	}
	SetZeroNegativeFlags(result);
	return result;
}

void Spc::ASL()
{
	uint8_t result = ShiftLeft(Read(_operandA));
	Write(_operandA, result);
}

void Spc::ASL_Acc()
{
	DummyRead();
	_state.A = ShiftLeft(_state.A);
}

void Spc::LSR()
{
	uint8_t result = ShiftRight(Read(_operandA));
	Write(_operandA, result);
}

void Spc::LSR_Acc()
{
	DummyRead();
	_state.A = ShiftRight(_state.A);
}

void Spc::ROL()
{
	uint8_t result = RollLeft(Read(_operandA));
	Write(_operandA, result);
}

void Spc::ROL_Acc()
{
	DummyRead();
	_state.A = RollLeft(_state.A);
}

void Spc::ROR()
{
	uint8_t result = RollRight(Read(_operandA));
	Write(_operandA, result);
}

void Spc::ROR_Acc()
{
	DummyRead();
	_state.A = RollRight(_state.A);
}

void Spc::XCN()
{
	DummyRead();
	Idle();
	Idle();
	Idle();
	_state.A = (_state.A >> 4) | (_state.A << 4);
	SetZeroNegativeFlags(_state.A);
}

void Spc::Branch()
{
	Idle();
	Idle();
	int8_t offset = (int8_t)_operandA;
	_state.PC = _state.PC + offset;
}

void Spc::BRA()
{
	Branch();
}

void Spc::BEQ()
{
	if(CheckFlag(SpcFlags::Zero)) {
		Branch();
	}
}

void Spc::BNE()
{
	if(!CheckFlag(SpcFlags::Zero)) {
		Branch();
	}
}

void Spc::BCS()
{
	if(CheckFlag(SpcFlags::Carry)) {
		Branch();
	}
}

void Spc::BCC()
{
	if(!CheckFlag(SpcFlags::Carry)) {
		Branch();
	}
}

void Spc::BVS()
{
	if(CheckFlag(SpcFlags::Overflow)) {
		Branch();
	}
}

void Spc::BVC()
{
	if(!CheckFlag(SpcFlags::Overflow)) {
		Branch();
	}
}

void Spc::BMI()
{
	if(CheckFlag(SpcFlags::Negative)) {
		Branch();
	}
}

void Spc::BPL()
{
	if(!CheckFlag(SpcFlags::Negative)) {
		Branch();
	}
}

template<uint8_t bit>
void Spc::SET1()
{
	uint8_t value = Read(_operandA);
	Write(_operandA, value | (1 << bit));
}

template<uint8_t bit>
void Spc::CLR1()
{
	uint8_t value = Read(_operandA);
	Write(_operandA, value & ~(1 << bit));
}

template<uint8_t bit>
void Spc::BBS()
{
	uint8_t compareValue = Read(_operandA);
	Idle();

	int8_t offset = ReadOperandByte();
	if(compareValue & (1 << bit)) {
		Idle();
		Idle();
		_state.PC += offset;
	}
}

template<uint8_t bit>
void Spc::BBC()
{
	uint8_t compareValue = Read(_operandA);
	Idle();

	int8_t offset = ReadOperandByte();
	if((compareValue & (1 << bit)) == 0) {
		Idle();
		Idle();
		_state.PC += offset;
	}
}

void Spc::CBNE()
{
	uint8_t compareValue = Read(_operandA);
	Idle();

	int8_t offset = ReadOperandByte();
	if(_state.A != compareValue) {
		Idle();
		Idle();
		_state.PC = _state.PC + offset;
	}
}

void Spc::DBNZ()
{
	uint8_t value = Read(_operandA);

	value--;
	Write(_operandA, value);

	int8_t offset = ReadOperandByte();
	if(value) {
		Idle();
		Idle();
		_state.PC = _state.PC + offset;
	}
}

void Spc::DBNZ_Y()
{
	DummyRead();
	Idle();

	_state.Y--;

	int8_t offset = ReadOperandByte();
	if(_state.Y) {
		Idle();
		Idle();
		_state.PC = _state.PC + offset;
	}
}


void Spc::JMP()
{
	_state.PC = _operandA;
}

void Spc::NOTC()
{
	DummyRead();
	Idle();
	if(CheckFlag(SpcFlags::Carry)) {
		ClearFlags(SpcFlags::Carry);
	} else {
		SetFlags(SpcFlags::Carry);
	}
}

void Spc::CLRC()
{
	DummyRead();
	ClearFlags(SpcFlags::Carry);
}

void Spc::CLRP()
{
	DummyRead();
	ClearFlags(SpcFlags::DirectPage);
}

void Spc::CLRV()
{
	DummyRead();
	ClearFlags(SpcFlags::Overflow | SpcFlags::HalfCarry);
}

void Spc::SETC()
{
	DummyRead();
	SetFlags(SpcFlags::Carry);
}

void Spc::SETP()
{
	DummyRead();
	SetFlags(SpcFlags::DirectPage);
}

void Spc::EI()
{
	DummyRead();
	SetFlags(SpcFlags::IrqEnable);
	Idle();
}

void Spc::DI()
{
	DummyRead();
	ClearFlags(SpcFlags::IrqEnable);
	Idle();
}

void Spc::TSET1()
{
	uint8_t value = Read(_operandA);
	DummyRead(_operandA);

	Write(_operandA, value | _state.A);
	SetZeroNegativeFlags(_state.A - value);
}

void Spc::TCLR1()
{
	uint8_t value = Read(_operandA);
	DummyRead(_operandA);

	Write(_operandA, value & ~_state.A);
	SetZeroNegativeFlags(_state.A - value);
}

template<uint8_t offset>
void Spc::TCALL()
{
	DummyRead();
	Idle();
	PushWord(_state.PC);
	Idle();

	constexpr uint16_t vectorAddr = 0xFFDE - (offset * 2);
	_state.PC = ReadWord(vectorAddr);
}

void Spc::PCALL()
{
	uint8_t offset = ReadOperandByte();
	Idle();
	PushWord(_state.PC);

	Idle();
	_state.PC = 0xFF00 | offset;
}

void Spc::JSR()
{
	Idle();
	PushWord(_state.PC);
	Idle();
	Idle();
	_state.PC = _operandA;
}

void Spc::RTS()
{
	DummyRead();
	Idle();
	_state.PC = PopWord();
}

void Spc::RTI()
{
	DummyRead();
	Idle();
	_state.PS = Pop();
	_state.PC = PopWord();
}

void Spc::BRK()
{
	DummyRead();
	PushWord(_state.PC);
	Push(_state.PS);
	Idle();

	uint8_t lsb = Read(0xFFDE);
	uint8_t msb = Read(0xFFDF);
	_state.PC = (msb << 8) | lsb;

	SetFlags(SpcFlags::Break);
	ClearFlags(SpcFlags::IrqEnable);
}

void Spc::PHA()
{
	DummyRead();
	Push(_state.A);
	Idle();
}

void Spc::PHX()
{
	DummyRead();
	Push(_state.X);
	Idle();
}

void Spc::PHY()
{
	DummyRead();
	Push(_state.Y);
	Idle();
}

void Spc::PHP()
{
	DummyRead();
	Push(_state.PS);
	Idle();
}

void Spc::PLA()
{
	DummyRead();
	Idle();
	_state.A = Pop();
}

void Spc::PLX()
{
	DummyRead();
	Idle();
	_state.X = Pop();
}

void Spc::PLY()
{
	DummyRead();
	Idle();
	_state.Y = Pop();
}

void Spc::PLP()
{
	DummyRead();
	Idle();
	_state.PS = Pop();
}

void Spc::NOP()
{
	DummyRead();
}

void Spc::SLEEP()
{
	//WAI
}

void Spc::STOP()
{
	//STP
}
