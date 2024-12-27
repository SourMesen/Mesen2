#include "pch.h"
#include "SNES/Spc.h"
#include "SNES/SnesMemoryManager.h"
#include "Shared/Emulator.h"
#include "Utilities/HexUtilities.h"

void Spc::Run()
{
	if(!_enabled) {
		//Used to temporarily disable the SPC when overclocking is enabled
		return;
	} else if(_state.StopState != SnesCpuStopState::Running) {
		//STOP or SLEEP were executed - execution is stopped forever.
		_emu->ProcessHaltedCpu<CpuType::Spc>();
		return;
	}

	//Minus 1 because each call to ProcessCycle increments _state.Cycle by 2
	int64_t targetCycle = (int64_t)(_memoryManager->GetMasterClock() * _clockRatio) - 1;
	while((int64_t)_state.Cycle < targetCycle) {
		ProcessCycle();
	}
}

void Spc::ProcessCycle()
{
	if(_opStep == SpcOpStep::ReadOpCode) {
#ifndef DUMMYSPC
		_emu->ProcessInstruction<CpuType::Spc>();
#endif 
		_opCode = GetOpCode();
		_opStep = SpcOpStep::Addressing;
		_opSubStep = 0;
	} else {
		Exec();
	}

	if(_pendingCpuRegUpdate) {
		//There appears to be a delay between the moment the CPU writes
		//to a register and when the SPC can see the new value
		//This makes the SPC see the newly written value after a 1 SPC cycle delay
		//This allows Kishin Douji Zenki to boot without freezing
		for(int i = 0; i < 4; i++) {
			_state.CpuRegs[i] = _state.NewCpuRegs[i];
		}
		_pendingCpuRegUpdate = false;
	}
}

void Spc::EndOp()
{
	_opStep = SpcOpStep::ReadOpCode;
}

void Spc::EndAddr()
{
	_opStep = SpcOpStep::AfterAddressing;
	_opSubStep = 0;
}

void Spc::Exec()
{
	switch(_opCode) {
		case 0x00: NOP(); break;
		case 0x01: TCALL<0>(); break;
		case 0x02: Addr_Dir(); SET1<0>(); break;
		case 0x03: Addr_Dir(); BBS<0>(); break;
		case 0x04: Addr_Dir(); OR_Acc(); break;
		case 0x05: Addr_Abs(); OR_Acc(); break;
		case 0x06: Addr_IndX(); OR_Acc(); break;
		case 0x07: Addr_DirIdxXInd(); OR_Acc(); break;
		case 0x08: Addr_Imm(); OR_Imm(); break;
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
		case 0x28: Addr_Imm(); AND_Imm(); break;
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
		case 0x48: Addr_Imm(); EOR_Imm(); break;
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
		case 0x68: Addr_Imm(); CMP_Imm(); break;
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
		case 0x88: Addr_Imm(); ADC_Imm(); break;
		case 0x89: Addr_DirToDir(); ADC(); break;
		case 0x8A: Addr_AbsBit(); EOR1(); break;
		case 0x8B: Addr_Dir(); DEC(); break;
		case 0x8C: Addr_Abs(); DEC(); break;
		case 0x8D: Addr_Imm(); LDY_Imm(); break;
		case 0x8E: PLP(); break;
		case 0x8F: Addr_DirImm(); MOV_Imm(); break;
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
		case 0xA8: Addr_Imm(); SBC_Imm(); break;
		case 0xA9: Addr_DirToDir(); SBC(); break;
		case 0xAA: Addr_AbsBit(); LDC(); break;
		case 0xAB: Addr_Dir(); INC(); break;
		case 0xAC: Addr_Abs(); INC(); break;
		case 0xAD: Addr_Imm(); CPY_Imm(); break;
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
		case 0xC8: Addr_Imm(); CPX_Imm(); break;
		case 0xC9: Addr_Abs(); STX(); break;
		case 0xCA: Addr_AbsBit(); STC(); break;
		case 0xCB: Addr_Dir(); STY(); break;
		case 0xCC: Addr_Abs(); STY(); break;
		case 0xCD: Addr_Imm(); LDX_Imm(); break;
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
		case 0xE8: Addr_Imm(); LDA_Imm(); break;
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

	if(_opStep == SpcOpStep::AfterAddressing) {
		_opStep = SpcOpStep::Operation;
	}
}

//*****************
// ADDRESSING MODES
//*****************
void Spc::Addr_Dir()
{
	if(_opStep == SpcOpStep::Addressing) {
		_operandA = GetDirectAddress(ReadOperandByte());
		EndAddr();
	}
}

void Spc::Addr_DirIdxX()
{
	if(_opStep == SpcOpStep::Addressing) {
		switch(_opSubStep++) {
			case 0: _operandA = GetDirectAddress(ReadOperandByte() + _state.X); break;
			case 1: Idle(); EndAddr(); break;
		}
	}
}

void Spc::Addr_DirIdxY()
{
	if(_opStep == SpcOpStep::Addressing) {
		switch(_opSubStep++) {
			case 0: _operandA = GetDirectAddress(ReadOperandByte() + _state.Y); break;
			case 1: Idle(); EndAddr(); break;
		}
	}
}

void Spc::Addr_DirToDir()
{
	if(_opStep == SpcOpStep::Addressing) {
		switch(_opSubStep++) {
			case 0: _tmp1 = GetDirectAddress(ReadOperandByte()); break;
			case 1: _operandA = Read(_tmp1); break;
			case 2: _operandB = GetDirectAddress(ReadOperandByte()); EndAddr(); break;
		}
	}
}

void Spc::Addr_DirImm()
{
	if(_opStep == SpcOpStep::Addressing) {
		switch(_opSubStep++) {
			case 0: _operandA = ReadOperandByte(); break;
			case 1: _operandB = GetDirectAddress(ReadOperandByte()); EndAddr(); break;
		}
	}
}

void Spc::Addr_DirIdxXInd()
{
	if(_opStep == SpcOpStep::Addressing) {
		switch(_opSubStep++) {
			case 0: _tmp1 = GetDirectAddress(ReadOperandByte() + _state.X); break;
			case 1: Idle(); break;
			case 2: _tmp2 = Read(_tmp1); break;
			case 3:
				_tmp3 = Read(GetDirectAddress(_tmp1 + 1));
				_operandA = (_tmp3 << 8) | _tmp2;
				EndAddr(); 
				break;
		}
	}
}

void Spc::Addr_DirIndIdxY()
{
	if(_opStep == SpcOpStep::Addressing) {
		switch(_opSubStep++) {
			case 0: _tmp1 = GetDirectAddress(ReadOperandByte()); break;
			case 1: _tmp2 = Read(_tmp1); break;
			case 2: _tmp3 = Read(GetDirectAddress(_tmp1 + 1)); break;
			case 4:
				Idle();
				_operandA = ((_tmp3 << 8) | _tmp2) + _state.Y;
				EndAddr(); 
				break;
		}
	}
}

void Spc::Addr_IndX()
{
	if(_opStep == SpcOpStep::Addressing) {
		DummyRead();
		_operandA = GetDirectAddress(_state.X);
		EndAddr();
	}
}

void Spc::Addr_IndXToIndY()
{
	if(_opStep == SpcOpStep::Addressing) {
		switch(_opSubStep++) {
			case 0: DummyRead(); break;
			case 1:
				_operandA = Read(GetDirectAddress(_state.Y));
				_operandB = GetDirectAddress(_state.X);
				EndAddr();
				break;
		}
	}
}

void Spc::Addr_Abs()
{
	if(_opStep == SpcOpStep::Addressing) {
		switch(_opSubStep++) {
			case 0: _tmp1 = ReadOperandByte(); break;
			case 1:
				_tmp2 = ReadOperandByte();
				_operandA = ((_tmp2 << 8) | _tmp1);
				EndAddr();
				break;
		}
	}
}

void Spc::Addr_AbsBit()
{
	if(_opStep == SpcOpStep::Addressing) {
		switch(_opSubStep++) {
			case 0: _tmp1 = ReadOperandByte(); break;
			case 1:
				_tmp2 = ReadOperandByte();
				_operandA = ((_tmp2 << 8) | _tmp1);
				_operandB = _operandA >> 13;
				_operandA = _operandA & 0x1FFF;
				EndAddr();
				break;
		}
	}
}

void Spc::Addr_AbsIdxX()
{
	if(_opStep == SpcOpStep::Addressing) {
		switch(_opSubStep++) {
			case 0: _tmp1 = ReadOperandByte(); break;
			case 1:
				_tmp2 = ReadOperandByte();
				_operandA = ((_tmp2 << 8) | _tmp1);
				break;

			case 2:
				Idle();
				_operandA += _state.X;
				EndAddr();
				break;
		}
	}
}

void Spc::Addr_AbsIdxY()
{
	if(_opStep == SpcOpStep::Addressing) {
		switch(_opSubStep++) {
			case 0: _tmp1 = ReadOperandByte(); break;
			case 1:
				_tmp2 = ReadOperandByte();
				_operandA = ((_tmp2 << 8) | _tmp1);
				break;
			
			case 2:
				Idle();
				_operandA += _state.Y;
				EndAddr();
				break;
		}
	}
}

void Spc::Addr_AbsIdxXInd()
{
	//Used by JMP only
	if(_opStep == SpcOpStep::Addressing) {
		switch(_opSubStep++) {
			case 0: _tmp1 = ReadOperandByte(); break;
			case 1: _tmp2 = ReadOperandByte(); break;
			case 2: Idle(); break;
			case 3:
				_tmp3 = ((_tmp2 << 8) | _tmp1);
				_tmp1 = Read(_tmp3 + _state.X);
				break;

			case 4:
				_operandA = (Read(_tmp3 + _state.X + 1) << 8) | _tmp1;
				EndAddr();
				break;
		}
	}
}

void Spc::Addr_Rel()
{
	if(_opStep == SpcOpStep::Addressing) {
		_operandA = ReadOperandByte();
		EndAddr();
	}
}

void Spc::Addr_Imm()
{
	if(_opStep == SpcOpStep::Addressing) {
		_operandA = ReadOperandByte();
		EndAddr();
	}
}

//*****************
// INSTRUCTIONS
//*****************
void Spc::STA()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0: Read(_operandA); break;
			case 1: Write(_operandA, _state.A); EndOp(); break;
		}
	}
}

void Spc::STX()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0: Read(_operandA); break;
			case 1: Write(_operandA, _state.X); EndOp(); break;
		}
	}
}

void Spc::STY()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0: Read(_operandA); break;
			case 1: Write(_operandA, _state.Y); EndOp(); break;
		}
	}
}

void Spc::STW()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0: DummyRead(_operandA); break;
			case 1: Write(_operandA, _state.A); break;
			case 2:
				uint16_t msbAddress = GetDirectAddress(_operandA + 1);
				Write(msbAddress, _state.Y);
				EndOp();
				break;
		}
	}
}

void Spc::STA_AutoIncX()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0: Idle(); break;
			case 1:
				Write(_operandA, _state.A);
				_state.X++;
				EndOp();
				break;
		}
	}
}

void Spc::LDA_AutoIncX()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0:
				_state.A = Read(_operandA);
				SetZeroNegativeFlags(_state.A);
				break;

			case 1:
				Idle();
				_state.X++;
				EndOp();
				break;
		}
	}
}

void Spc::LDA()
{
	if(_opStep == SpcOpStep::Operation) {
		_state.A = GetByteValue();
		SetZeroNegativeFlags(_state.A);
		EndOp();
	}
}

void Spc::LDA_Imm()
{
	if(_opStep == SpcOpStep::AfterAddressing) {
		_state.A = (uint8_t)_operandA;
		SetZeroNegativeFlags(_state.A);
		EndOp();
	}
}

void Spc::LDX()
{
	if(_opStep == SpcOpStep::Operation) {
		_state.X = GetByteValue();
		SetZeroNegativeFlags(_state.X);
		EndOp();
	}
}

void Spc::LDX_Imm()
{
	if(_opStep == SpcOpStep::AfterAddressing) {
		_state.X = (uint8_t)_operandA;
		SetZeroNegativeFlags(_state.X);
		EndOp();
	}
}

void Spc::LDY()
{
	if(_opStep == SpcOpStep::Operation) {
		_state.Y = GetByteValue();
		SetZeroNegativeFlags(_state.Y);
		EndOp();
	}
}

void Spc::LDY_Imm()
{
	if(_opStep == SpcOpStep::AfterAddressing) {
		_state.Y = (uint8_t)_operandA;
		SetZeroNegativeFlags(_state.Y);
		EndOp();
	}
}

void Spc::LDW()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0: _tmp1 = Read(_operandA); break;
			case 1: Idle(); break;
			case 2:
				uint8_t msb = Read(GetDirectAddress(_operandA + 1));
				uint16_t value = (msb << 8) | _tmp1;
				_state.A = (uint8_t)value;
				_state.Y = (value >> 8);
				SetZeroNegativeFlags16(value);
				EndOp();
				break;
		}
	}
}

void Spc::Transfer(uint8_t &dst, uint8_t src)
{
	DummyRead();
	dst = src;
	SetZeroNegativeFlags(src);
	EndOp();
}

void Spc::TXA()
{
	Transfer(_state.A, _state.X);
}

void Spc::TYA()
{
	Transfer(_state.A, _state.Y);
}

void Spc::TAX()
{
	Transfer(_state.X, _state.A);
}

void Spc::TAY()
{
	Transfer(_state.Y, _state.A);
}

void Spc::TSX()
{
	Transfer(_state.X, _state.SP);
}

void Spc::TXS()
{
	DummyRead();
	_state.SP = _state.X;
	EndOp();
}

void Spc::MOV()
{
	if(_opStep == SpcOpStep::Operation) {
		Write(_operandB, (uint8_t)_operandA);
		EndOp();
	}
}

void Spc::MOV_Imm()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0: DummyRead(_operandB); break;
			case 1: Write(_operandB, (uint8_t)_operandA); EndOp(); break;
		}
	}
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
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0:
				_tmp1 = (uint8_t)_operandA;
				_tmp2 = Read(_operandB);
				break;

			case 1:
				Write(_operandB, Add((uint8_t)_tmp2, (uint8_t)_tmp1));
				EndOp();
				break;
		}
	}
}

void Spc::ADC_Acc()
{
	if(_opStep == SpcOpStep::Operation) {
		_state.A = Add(_state.A, GetByteValue());
		EndOp();
	}
}

void Spc::ADC_Imm()
{
	if(_opStep == SpcOpStep::AfterAddressing) {
		_state.A = Add(_state.A, (uint8_t)_operandA);
		EndOp();
	}
}

void Spc::ADDW()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0: _tmp1 = Read(_operandA); break;
			case 1: Idle(); break;
			case 2:
				uint8_t msb = Read(GetDirectAddress(_operandA + 1));
				uint16_t value = ((msb << 8) | _tmp1);

				uint8_t lowCarry = (_tmp1 + _state.A) > 0xFF ? 1 : 0;
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

				EndOp();
		}
	}
}

void Spc::SBC()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0:
				_tmp1 = (uint8_t)_operandA;
				_tmp2 = Read(_operandB);
				break;

			case 1:
				Write(_operandB, Sub((uint8_t)_tmp2, (uint8_t)_tmp1));
				EndOp();
				break;
		}
	}
}

void Spc::SBC_Acc()
{
	if(_opStep == SpcOpStep::Operation) {
		_state.A = Sub(_state.A, GetByteValue());
		EndOp();
	}
}

void Spc::SBC_Imm()
{
	if(_opStep == SpcOpStep::AfterAddressing) {
		_state.A = Sub(_state.A, (uint8_t)_operandA);
		EndOp();
	}
}

void Spc::SUBW()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0: _tmp1 = Read(_operandA); break;
			case 1: Idle(); break;
			case 2:
				uint8_t msb = Read(GetDirectAddress(_operandA + 1));
				uint16_t value = ((msb << 8) | _tmp1);
				uint16_t ya = (_state.Y << 8) | _state.A;

				uint32_t l = _state.A - _tmp1;
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
				EndOp();
				break;
		}
	}
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
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0: Compare(Read(_operandB), (uint8_t)_operandA); break;
			case 1: Idle(); EndOp(); break;
		}
	}
}

void Spc::CMP_Acc()
{
	if(_opStep == SpcOpStep::Operation) {
		Compare(_state.A, GetByteValue());
		EndOp();
	}
}

void Spc::CMP_Imm()
{
	if(_opStep == SpcOpStep::AfterAddressing) {
		Compare(_state.A, (uint8_t)_operandA);
		EndOp();
	}
}

void Spc::CPX()
{
	if(_opStep == SpcOpStep::Operation) {
		Compare(_state.X, GetByteValue());
		EndOp();
	}
}

void Spc::CPX_Imm()
{
	if(_opStep == SpcOpStep::AfterAddressing) {
		Compare(_state.X, (uint8_t)_operandA);
		EndOp();
	}
}

void Spc::CPY()
{
	if(_opStep == SpcOpStep::Operation) {
		Compare(_state.Y, GetByteValue());
		EndOp();
	}
}

void Spc::CPY_Imm()
{
	if(_opStep == SpcOpStep::AfterAddressing) {
		Compare(_state.Y, (uint8_t)_operandA);
		EndOp();
	}
}

void Spc::CMPW()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0: _tmp1 = Read(_operandA); break;
			case 1:
				uint8_t msb = Read(GetDirectAddress(_operandA + 1));
				uint16_t value = ((msb << 8) | _tmp1);

				uint16_t ya = (_state.Y << 8) | _state.A;

				if(ya >= value) {
					SetFlags(SpcFlags::Carry);
				} else {
					ClearFlags(SpcFlags::Carry);
				}

				uint16_t result = ya - value;
				SetZeroNegativeFlags16(result);
				EndOp();
				break;
		}
	}
}

void Spc::INC()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0: _tmp1 = Read(_operandA) + 1; break;
			case 1:
				Write(_operandA, (uint8_t)_tmp1);
				SetZeroNegativeFlags((uint8_t)_tmp1);
				EndOp();
				break;
		}
	}
}

void Spc::INC_Acc()
{
	DummyRead();
	_state.A++;
	SetZeroNegativeFlags(_state.A);
	EndOp();
}

void Spc::INX()
{
	DummyRead();
	_state.X++;
	SetZeroNegativeFlags(_state.X);
	EndOp();
}

void Spc::INY()
{
	DummyRead();
	_state.Y++;
	SetZeroNegativeFlags(_state.Y);
	EndOp();
}

void Spc::INCW()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0: _tmp1 = Read(_operandA); break;
			case 1: Write(_operandA, _tmp1 + 1); break;

			case 2: {
				_tmp2 = GetDirectAddress(_operandA + 1);
				uint8_t msb = Read(_tmp2);
				_tmp3 = ((msb << 8) | _tmp1) + 1;
				break;
			}

			case 3:
				Write(_tmp2, _tmp3 >> 8);
				SetZeroNegativeFlags16(_tmp3);
				EndOp();
				break;
		}
	}
}

void Spc::DEC()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0: _tmp1 = Read(_operandA) - 1; break;
			case 1:
				Write(_operandA, (uint8_t)_tmp1);
				SetZeroNegativeFlags((uint8_t)_tmp1);
				EndOp();
				break;
		}
	}
}

void Spc::DEC_Acc()
{
	DummyRead();
	_state.A--;
	SetZeroNegativeFlags(_state.A);
	EndOp();
}

void Spc::DEX()
{
	DummyRead();
	_state.X--;
	SetZeroNegativeFlags(_state.X);
	EndOp();
}

void Spc::DEY()
{
	DummyRead();
	_state.Y--;
	SetZeroNegativeFlags(_state.Y);
	EndOp();
}

void Spc::DECW()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0: _tmp1 = Read(_operandA); break;
			case 1: Write(_operandA, _tmp1 - 1); break;

			case 2: {
				_tmp2 = GetDirectAddress(_operandA + 1);
				uint8_t msb = Read(_tmp2);
				_tmp3 = ((msb << 8) | _tmp1) - 1;
				break;
			}

			case 3:
				Write(_tmp2, _tmp3 >> 8);
				SetZeroNegativeFlags16(_tmp3);
				EndOp();
				break;
		}
	}
}

void Spc::MUL()
{
	switch(_opSubStep++) {
		case 0: DummyRead(); break;
		case 1: Idle(); break;
		case 2: Idle(); break;
		case 3: Idle(); break;
		case 4: Idle(); break;
		case 5: Idle(); break;
		case 6: Idle(); break;
		case 7:
			Idle();
			uint16_t result = _state.Y * _state.A;
			_state.Y = result >> 8;
			_state.A = (uint8_t)result;
			SetZeroNegativeFlags(_state.Y);
			EndOp();
			break;
	}
}

void Spc::DIV()
{
	switch(_opSubStep++) {
		case 0: DummyRead(); break;
		case 1: Idle(); break;
		case 2: Idle(); break;
		case 3: Idle(); break;
		case 4: Idle(); break;
		case 5: Idle(); break;
		case 6: Idle(); break;
		case 7: Idle(); break;
		case 8: Idle(); break;
		case 9: Idle(); break;
		case 11:
			Idle();
			uint32_t ya = (_state.Y << 8) | _state.A;
			uint32_t sub = _state.X << 9;

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
			}


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
			EndOp();
			break;
	}
}

void Spc::DAA()
{
	switch(_opSubStep++) {
		case 0: Idle(); break;
		case 1:
			Idle();
			if(CheckFlag(SpcFlags::Carry) || _state.A > 0x99) {
				_state.A += 0x60;
				SetFlags(SpcFlags::Carry);
			}

			if(CheckFlag(SpcFlags::HalfCarry) || ((_state.A & 0x0F) > 9)) {
				_state.A += 6;
			}

			SetZeroNegativeFlags(_state.A);
			EndOp();
			break;
	}
}

void Spc::DAS()
{
	switch(_opSubStep++) {
		case 0: Idle(); break;
		case 1:
			Idle();

			if(!CheckFlag(SpcFlags::Carry) || _state.A > 0x99) {
				_state.A -= 0x60;
				ClearFlags(SpcFlags::Carry);
			}

			if(!CheckFlag(SpcFlags::HalfCarry) || ((_state.A & 0x0F) > 9)) {
				_state.A -= 6;
			}

			SetZeroNegativeFlags(_state.A);
			EndOp();
			break;
	}
}

void Spc::AND()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0: _tmp1 = _operandA & Read(_operandB); break;
			case 1:
				Write(_operandB, (uint8_t)_tmp1);
				SetZeroNegativeFlags((uint8_t)_tmp1);
				EndOp();
				break;
		}
	}
}

void Spc::AND_Acc()
{
	if(_opStep == SpcOpStep::Operation) {
		_state.A &= GetByteValue();
		SetZeroNegativeFlags(_state.A);
		EndOp();
	}
}

void Spc::AND_Imm()
{
	if(_opStep == SpcOpStep::AfterAddressing) {
		_state.A &= _operandA;
		SetZeroNegativeFlags(_state.A);
		EndOp();
	}
}

void Spc::OR()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0: _tmp1 = _operandA | Read(_operandB); break;
			case 1:
				Write(_operandB, (uint8_t)_tmp1);
				SetZeroNegativeFlags((uint8_t)_tmp1);
				EndOp();
				break;
		}
	}
}

void Spc::OR_Acc()
{
	if(_opStep == SpcOpStep::Operation) {
		_state.A |= GetByteValue();
		SetZeroNegativeFlags(_state.A);
		EndOp();
	}
}

void Spc::OR_Imm()
{
	if(_opStep == SpcOpStep::AfterAddressing) {
		_state.A |= _operandA;
		SetZeroNegativeFlags(_state.A);
		EndOp();
	}
}

void Spc::EOR()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0: _tmp1 = _operandA ^ Read(_operandB); break;
			case 1:
				Write(_operandB, (uint8_t)_tmp1);
				SetZeroNegativeFlags((uint8_t)_tmp1);
				EndOp();
				break;
		}
	}
}

void Spc::EOR_Acc()
{
	if(_opStep == SpcOpStep::Operation) {
		_state.A ^= GetByteValue();
		SetZeroNegativeFlags(_state.A);
		EndOp();
	}
}

void Spc::EOR_Imm()
{
	if(_opStep == SpcOpStep::AfterAddressing) {
		_state.A ^= _operandA;
		SetZeroNegativeFlags(_state.A);
		EndOp();
	}
}

void Spc::SetCarry(uint8_t carry)
{
	_state.PS = (_state.PS & ~SpcFlags::Carry) | carry;
}

void Spc::OR1()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0:
			{
				uint8_t carry = _state.PS & SpcFlags::Carry;
				carry |= (Read(_operandA) >> _operandB) & 0x01;
				SetCarry(carry);
				break;
			}

			case 1:
				Idle();
				EndOp();
				break;
		}
	}
}

void Spc::NOR1()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0:
			{
				uint8_t carry = _state.PS & SpcFlags::Carry;
				carry |= ~((Read(_operandA) >> _operandB)) & 0x01;
				SetCarry(carry);
				break;
			}

			case 1:
				Idle();
				EndOp();
				break;
		}
	}
}

void Spc::AND1()
{
	if(_opStep == SpcOpStep::Operation) {
		uint8_t carry = _state.PS & SpcFlags::Carry;
		carry &= (Read(_operandA) >> _operandB) & 0x01;
		SetCarry(carry);
		EndOp();
	}
}

void Spc::NAND1()
{
	if(_opStep == SpcOpStep::Operation) {
		uint8_t carry = _state.PS & SpcFlags::Carry;
		carry &= ~((Read(_operandA) >> _operandB)) & 0x01;
		SetCarry(carry);
		EndOp();
	}
}

void Spc::EOR1()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0:
			{
				uint8_t carry = _state.PS & SpcFlags::Carry;
				carry ^= (Read(_operandA) >> _operandB) & 0x01;
				SetCarry(carry);
				break;
			}

			case 1:
				Idle();
				EndOp(); 
				break;
		}
	}
}

void Spc::NOT1()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0: _tmp1 = Read(_operandA); break;
			case 1:
				uint8_t mask = (1 << _operandB);
				Write(_operandA, _tmp1 ^ mask);
				EndOp();
				break;
		}
	}
}

void Spc::STC()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0: _tmp1 = Read(_operandA); break;
			case 1: Idle(); break;
			case 2:
				uint8_t mask = (1 << _operandB);
				uint8_t carry = (_state.PS & SpcFlags::Carry) << _operandB;
				Write(_operandA, (_tmp1 & ~mask) | carry);
				EndOp();
				break;
		}
	}
}

void Spc::LDC()
{
	if(_opStep == SpcOpStep::Operation) {
		uint8_t carry = (Read(_operandA) >> _operandB) & 0x01;
		SetCarry(carry);
		EndOp();
	}
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
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0: _tmp1 = ShiftLeft(Read(_operandA)); break;
			case 1:
				Write(_operandA, (uint8_t)_tmp1);
				EndOp();
				break;
		}
	}
}

void Spc::ASL_Acc()
{
	DummyRead();
	_state.A = ShiftLeft(_state.A);
	EndOp();
}

void Spc::LSR()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0: _tmp1 = ShiftRight(Read(_operandA)); break;
			case 1:
				Write(_operandA, (uint8_t)_tmp1);
				EndOp();
				break;
		}
	}
}

void Spc::LSR_Acc()
{
	DummyRead();
	_state.A = ShiftRight(_state.A);
	EndOp();
}

void Spc::ROL()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0: _tmp1 = RollLeft(Read(_operandA)); break;
			case 1:
				Write(_operandA, (uint8_t)_tmp1);
				EndOp();
				break;
		}
	}
}

void Spc::ROL_Acc()
{
	DummyRead();
	_state.A = RollLeft(_state.A);
	EndOp();
}

void Spc::ROR()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0: _tmp1 = RollRight(Read(_operandA)); break;
			case 1:
				Write(_operandA, (uint8_t)_tmp1);
				EndOp();
				break;
		}
	}
}

void Spc::ROR_Acc()
{
	DummyRead();
	_state.A = RollRight(_state.A);
	EndOp();
}

void Spc::XCN()
{
	switch(_opSubStep++) {
		case 0: DummyRead(); break;
		case 1: Idle(); break;
		case 2: Idle(); break;
		case 3:
			Idle();
			_state.A = (_state.A >> 4) | (_state.A << 4);
			SetZeroNegativeFlags(_state.A);
			EndOp(); 
			break;
	}
}

void Spc::Branch()
{
	switch(_opSubStep++) {
		case 0: Idle(); break;
		case 1:
			Idle();
			int8_t offset = (int8_t)_operandA;
			_state.PC = _state.PC + offset;
			EndOp();
			break;
	}
}

void Spc::BRA()
{
	if(_opStep == SpcOpStep::Operation) {
		Branch();
	}
}

void Spc::BEQ()
{
	if(_opStep == SpcOpStep::Operation) {
		if(CheckFlag(SpcFlags::Zero)) {
			Branch();
		} else {
			EndOp();
		}
	}
}

void Spc::BNE()
{
	if(_opStep == SpcOpStep::Operation) {
		if(!CheckFlag(SpcFlags::Zero)) {
			Branch();
		} else {
			EndOp();
		}
	}
}

void Spc::BCS()
{
	if(_opStep == SpcOpStep::Operation) {
		if(CheckFlag(SpcFlags::Carry)) {
			Branch();
		} else {
			EndOp();
		}
	}
}

void Spc::BCC()
{
	if(_opStep == SpcOpStep::Operation) {
		if(!CheckFlag(SpcFlags::Carry)) {
			Branch();
		} else {
			EndOp();
		}
	}
}

void Spc::BVS()
{
	if(_opStep == SpcOpStep::Operation) {
		if(CheckFlag(SpcFlags::Overflow)) {
			Branch();
		} else {
			EndOp();
		}
	}
}

void Spc::BVC()
{
	if(_opStep == SpcOpStep::Operation) {
		if(!CheckFlag(SpcFlags::Overflow)) {
			Branch();
		} else {
			EndOp();
		}
	}
}

void Spc::BMI()
{
	if(_opStep == SpcOpStep::Operation) {
		if(CheckFlag(SpcFlags::Negative)) {
			Branch();
		} else {
			EndOp();
		}
	}
}

void Spc::BPL()
{
	if(_opStep == SpcOpStep::Operation) {
		if(!CheckFlag(SpcFlags::Negative)) {
			Branch();
		} else {
			EndOp();
		}
	}
}

template<uint8_t bit>
void Spc::SET1()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0: _tmp1 = Read(_operandA); break;
			case 1:
				Write(_operandA, _tmp1 | (1 << bit));
				EndOp(); 
				break;
		}
	}
}

template<uint8_t bit>
void Spc::CLR1()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0: _tmp1 = Read(_operandA); break;
			case 1:
				Write(_operandA, _tmp1 & ~(1 << bit));
				EndOp();
				break;
		}
	}
}

template<uint8_t bit>
void Spc::BBS()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0: _tmp1 = Read(_operandA); break;
			case 1: Idle(); break;
			case 2:
				_tmp2 = ReadOperandByte();
				if(!(_tmp1 & (1 << bit))) {
					EndOp();
				}
				break;

			case 3: Idle(); break;
			case 4:
				Idle();
				_state.PC += (int8_t)_tmp2;
				EndOp();
				break;
		}
	}
}

template<uint8_t bit>
void Spc::BBC()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0: _tmp1 = Read(_operandA); break;
			case 1: Idle(); break;
			case 2:
				_tmp2 = ReadOperandByte();
				if(_tmp1 & (1 << bit)) {
					EndOp();
				}
				break;

			case 3: Idle(); break;
			case 4:
				Idle();
				_state.PC += (int8_t)_tmp2;
				EndOp();
				break;
		}
	}
}

void Spc::CBNE()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0: _tmp1 = Read(_operandA); break;
			case 1: Idle(); break;
			case 2:
				_tmp2  = ReadOperandByte();
				if(_state.A == _tmp1) {
					EndOp();
				}
				break;

			case 3: Idle(); break;

			case 4:
				Idle();
				_state.PC += (int8_t)_tmp2;
				EndOp();
				break;
		}
	}
}

void Spc::DBNZ()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0: _tmp1 = Read(_operandA) - 1; break;
			case 1: Write(_operandA, (uint8_t)_tmp1); break;
			case 2:
				_tmp2 = ReadOperandByte();
				if(!_tmp1) {
					EndOp();
				}
				break;

			case 3: Idle(); break;
			case 4:
				Idle();
				_state.PC += (int8_t)_tmp2;
				EndOp();
				break;
		}
	}
}

void Spc::DBNZ_Y()
{
	switch(_opSubStep++) {
		case 0: DummyRead(); break;
		case 1: Idle(); break;
		case 2:
			_state.Y--;
			_tmp2 = ReadOperandByte();
			if(!_state.Y) {
				EndOp();
			}
			break;

		case 3: Idle(); break;
		case 4:
			Idle();
			_state.PC += (int8_t)_tmp2;
			EndOp();
			break;
	}
}

void Spc::JMP()
{
	if(_opStep == SpcOpStep::AfterAddressing) {
		_state.PC = _operandA;
		EndOp();
	}
}

void Spc::NOTC()
{
	switch(_opSubStep++) {
		case 0: DummyRead(); break;
		case 1:
			Idle();
			if(CheckFlag(SpcFlags::Carry)) {
				ClearFlags(SpcFlags::Carry);
			} else {
				SetFlags(SpcFlags::Carry);
			}
			EndOp(); 
			break;
	}
}

void Spc::CLRC()
{
	DummyRead();
	ClearFlags(SpcFlags::Carry);
	EndOp();
}

void Spc::CLRP()
{
	DummyRead();
	ClearFlags(SpcFlags::DirectPage);
	EndOp();
}

void Spc::CLRV()
{
	DummyRead();
	ClearFlags(SpcFlags::Overflow | SpcFlags::HalfCarry);
	EndOp();
}

void Spc::SETC()
{
	DummyRead();
	SetFlags(SpcFlags::Carry);
	EndOp();
}

void Spc::SETP()
{
	DummyRead();
	SetFlags(SpcFlags::DirectPage);
	EndOp();
}

void Spc::EI()
{
	switch(_opSubStep++) {
		case 0: DummyRead(); break;
		case 1:
			SetFlags(SpcFlags::IrqEnable);
			Idle();
			EndOp();
			break;
	}
}

void Spc::DI()
{
	switch(_opSubStep++) {
		case 0: DummyRead(); break;
		case 1:
			ClearFlags(SpcFlags::IrqEnable);
			Idle();
			EndOp();
			break;
	}
}

void Spc::TSET1()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0: _tmp1 = Read(_operandA); break;
			case 1: DummyRead(_operandA); break;
			case 2:
				Write(_operandA, _tmp1 | _state.A);
				SetZeroNegativeFlags(_state.A - _tmp1);
				EndOp();
				break;
		}
	}
}

void Spc::TCLR1()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0: _tmp1 = Read(_operandA); break;
			case 1: DummyRead(_operandA); break;
			case 2:
				Write(_operandA, _tmp1 & ~_state.A);
				SetZeroNegativeFlags(_state.A - _tmp1);
				EndOp();
				break;
		}
	}
}

template<uint8_t offset>
void Spc::TCALL()
{
	constexpr uint16_t vectorAddr = 0xFFDE - (offset * 2);
	switch(_opSubStep++) {
		case 0: DummyRead(); break;
		case 1: Idle(); break;
		case 2: Push(_state.PC >> 8); break;
		case 3: Push((uint8_t)_state.PC); break;
		case 4: Idle(); break;
		case 5: _tmp1 = Read(vectorAddr); break;
		case 6:
			_state.PC = (Read(vectorAddr + 1) << 8) | _tmp1;
			EndOp();
			break;
	}
}

void Spc::PCALL()
{
	switch(_opSubStep++) {
		case 0: _tmp1 = ReadOperandByte(); break;
		case 1: Idle(); break;
		case 2: Push(_state.PC >> 8); break;
		case 3: Push((uint8_t)_state.PC); break;
		case 4:
			Idle();
			_state.PC = 0xFF00 | _tmp1;
			EndOp();
			break;
	}
}

void Spc::JSR()
{
	if(_opStep == SpcOpStep::Operation) {
		switch(_opSubStep++) {
			case 0: Idle(); break;
			case 1: Push(_state.PC >> 8); break;
			case 2: Push((uint8_t)_state.PC); break;
			case 3: Idle(); break;
			case 4:
				Idle();
				_state.PC = _operandA;
				EndOp();
				break;
		}
	}
}

void Spc::RTS()
{
	switch(_opSubStep++) {
		case 0: DummyRead(); break;
		case 1: Idle(); break;
		case 2: _tmp1 = Pop(); break;
		case 3:
			_state.PC = (Pop() << 8) | _tmp1;
			EndOp(); 
			break;
	}
}

void Spc::RTI()
{
	switch(_opSubStep++) {
		case 0: DummyRead(); break;
		case 1: Idle(); break;
		case 2: _state.PS = Pop(); break;
		case 3: _tmp1 = Pop(); break;
		case 4:
			_state.PC = (Pop() << 8) | _tmp1;
			EndOp();
			break;
	}
}

void Spc::BRK()
{
	switch(_opSubStep++) {
		case 0: Idle(); break;
		case 1: Push(_state.PC >> 8); break;
		case 2: Push((uint8_t)_state.PC); break;
		case 3: Push(_state.PS); break;
		case 4: Idle(); break;
		case 5: _tmp1 = Read(0xFFDE); break;
		case 6:
			uint8_t msb = Read(0xFFDF);
			_state.PC = (msb << 8) | _tmp1;

			SetFlags(SpcFlags::Break);
			ClearFlags(SpcFlags::IrqEnable);
			EndOp();
			break;
	}
}

void Spc::PushOperation(uint8_t value)
{
	switch(_opSubStep++) {
		case 0: DummyRead(); break;
		case 1: Push(value); break;
		case 2:
			Idle();
			EndOp();
			break;
	}
}

void Spc::PullOperation(uint8_t &dst)
{
	switch(_opSubStep++) {
		case 0: DummyRead(); break;
		case 1: Idle(); break;
		case 2:
			dst = Pop();
			EndOp();
			break;
	}
}

void Spc::PHA()
{
	PushOperation(_state.A);
}

void Spc::PHX()
{
	PushOperation(_state.X);
}

void Spc::PHY()
{
	PushOperation(_state.Y);
}

void Spc::PHP()
{
	PushOperation(_state.PS);
}

void Spc::PLA()
{
	PullOperation(_state.A);
}

void Spc::PLX()
{
	PullOperation(_state.X);
}

void Spc::PLY()
{
	PullOperation(_state.Y);
}

void Spc::PLP()
{
	PullOperation(_state.PS);
}

void Spc::NOP()
{
	DummyRead();
	EndOp();
}

void Spc::SLEEP()
{
	//WAI
	_state.StopState = SnesCpuStopState::WaitingForIrq;
	EndOp();
	ExitExecLoop();
}

void Spc::STOP()
{
	//STP
	_state.StopState = SnesCpuStopState::Stopped;
	EndOp();
	ExitExecLoop();
}
