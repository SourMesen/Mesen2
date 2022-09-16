#include "pch.h"
#include "SNES/Coprocessors/CX4/Cx4.h"
#include "SNES/SnesCpu.h"
#include "SNES/Debugger/Cx4DisUtils.h"
#include "Utilities/HexUtilities.h"

static constexpr int shiftLut[4] = { 0 , 1, 8, 16 };

extern const uint32_t _dataRom[1024];

void Cx4::Exec(uint16_t opCode)
{
	uint8_t op = (opCode >> 8) & 0xFC;
	uint8_t param1 = (opCode >> 8) & 0x03;
	uint8_t param2 = opCode & 0xFF;

	switch(op) {
		case 0x00: NOP(); break;
		case 0x04: NOP(); break; //???
		case 0x08: Branch(true, param1, param2); break;
		case 0x0C: Branch(_state.Zero, param1, param2); break;

		case 0x10: Branch(_state.Carry, param1, param2); break;
		case 0x14: Branch(_state.Negative, param1, param2); break;
		case 0x18: Branch(_state.Overflow, param1, param2); break;
		case 0x1C: WAIT(); break;

		case 0x20: NOP(); break; //???
		case 0x24: Skip(param1, param2); break;
		case 0x28: JSR(true, param1, param2); break;
		case 0x2C: JSR(_state.Zero, param1, param2); break;

		case 0x30: JSR(_state.Carry, param1, param2); break;
		case 0x34: JSR(_state.Negative, param1, param2); break;
		case 0x38: JSR(_state.Overflow, param1, param2); break;
		case 0x3C: RTS(); break;

		case 0x40: IncMar(); break;
		case 0x44: NOP(); break; //???
		case 0x48: CMPR(param1, param2); break;
		case 0x4C: CMPR_Imm(param1, param2); break;

		case 0x50: CMP(param1, param2); break;
		case 0x54: CMP_Imm(param1, param2); break;
		case 0x58: SignExtend(param1); break;
		case 0x5C: NOP(); break; //???

		case 0x60: Load(param1, param2); break;
		case 0x64: Load_Imm(param1, param2); break;
		case 0x68: ReadRam(param1); break;
		case 0x6C: ReadRam_Imm(param1, param2); break;

		case 0x70: ReadRom(); break;
		case 0x74: ReadRom_Imm((param1 << 8) | param2); break;
		case 0x78: NOP(); break;
		case 0x7C: LoadP(param1, param2); break;

		case 0x80: ADD(param1, param2); break;
		case 0x84: ADD_Imm(param1, param2); break;
		case 0x88: SUBR(param1, param2); break;
		case 0x8C: SUBR_Imm(param1, param2); break;

		case 0x90: SUB(param1, param2); break;
		case 0x94: SUB_Imm(param1, param2); break;
		case 0x98: SMUL(param2); break;
		case 0x9C: SMUL_Imm(param2); break;

		case 0xA0: XNOR(param1, param2); break;
		case 0xA4: XNOR_Imm(param1, param2); break;
		case 0xA8: XOR(param1, param2); break;
		case 0xAC: XOR_Imm(param1, param2); break;

		case 0xB0: AND(param1, param2); break;
		case 0xB4: AND_Imm(param1, param2); break;
		case 0xB8: OR(param1, param2); break;
		case 0xBC: OR_Imm(param1, param2); break;

		case 0xC0: SHR(param2); break;
		case 0xC4: SHR_Imm(param2); break;
		case 0xC8: ASR(param2); break;
		case 0xCC: ASR_Imm(param2); break;

		case 0xD0: ROR(param2); break;
		case 0xD4: ROR_Imm(param2); break;
		case 0xD8: SHL(param2); break;
		case 0xDC: SHL_Imm(param2); break;

		case 0xE0: Store(param1, param2); break;
		case 0xE4: NOP(); break; //???
		case 0xE8: WriteRam(param1); break;
		case 0xEC: WriteRam_Imm(param1, param2); break;

		case 0xF0: Swap(param2 & 0x0F); break;
		case 0xF4: NOP(); break; //???
		case 0xF8: NOP(); break; //???
		case 0xFC: Stop(); break;
	}

	Step(1);
}

uint32_t Cx4::GetSourceValue(uint8_t src)
{
	switch(src & 0x7F) {
		case 0x00: return _state.A;
		case 0x01: return (_state.Mult >> 24) & 0xFFFFFF;
		case 0x02: return _state.Mult & 0xFFFFFF;
		case 0x03: return _state.MemoryDataReg;
		case 0x08: return _state.RomBuffer;
		case 0x0C: return (_state.RamBuffer[2] << 16) | (_state.RamBuffer[1] << 8) | _state.RamBuffer[0];
		case 0x13: return _state.MemoryAddressReg;
		case 0x1C: return _state.DataPointerReg;
		case 0x20: return _state.PC;
		case 0x28: return _state.P;

		case 0x2E:
			_state.Bus.Enabled = true;
			_state.Bus.Reading = true;
			_state.Bus.DelayCycles = 1 + _state.RomAccessDelay;
			_state.Bus.Address = _state.MemoryAddressReg;
			return 0;

		case 0x2F:
			_state.Bus.Enabled = true;
			_state.Bus.Reading = true;
			_state.Bus.DelayCycles = 1 + _state.RamAccessDelay;
			_state.Bus.Address = _state.MemoryAddressReg;
			return 0;

		case 0x50: return 0x000000;
		case 0x51: return 0xFFFFFF;
		case 0x52: return 0x00FF00;
		case 0x53: return 0xFF0000;
		case 0x54: return 0x00FFFF;
		case 0x55: return 0xFFFF00;
		case 0x56: return 0x800000;
		case 0x57: return 0x7FFFFF;
		case 0x58: return 0x008000;
		case 0x59: return 0x007FFF;
		case 0x5A: return 0xFF7FFF;
		case 0x5B: return 0xFFFF7F;
		case 0x5C: return 0x010000;
		case 0x5D: return 0xFEFFFF;
		case 0x5E: return 0x000100;
		case 0x5F: return 0x00FEFF;

		case 0x60: case 0x70: return _state.Regs[0];
		case 0x61: case 0x71: return _state.Regs[1];
		case 0x62: case 0x72: return _state.Regs[2];
		case 0x63: case 0x73: return _state.Regs[3];
		case 0x64: case 0x74: return _state.Regs[4];
		case 0x65: case 0x75: return _state.Regs[5];
		case 0x66: case 0x76: return _state.Regs[6];
		case 0x67: case 0x77: return _state.Regs[7];
		case 0x68: case 0x78: return _state.Regs[8];
		case 0x69: case 0x79: return _state.Regs[9];
		case 0x6A: case 0x7A: return _state.Regs[10];
		case 0x6B: case 0x7B: return _state.Regs[11];
		case 0x6C: case 0x7C: return _state.Regs[12];
		case 0x6D: case 0x7D: return _state.Regs[13];
		case 0x6E: case 0x7E: return _state.Regs[14];
		case 0x6F: case 0x7F: return _state.Regs[15];
	}

	return 0;
}

void Cx4::WriteRegister(uint8_t reg, uint32_t value)
{
	value &= 0xFFFFFF;
	switch(reg & 0x7F) {
		case 0x01: _state.Mult = (_state.Mult & 0xFFFFFF) | ((uint64_t)value << 24); break;
		case 0x02: _state.Mult = (_state.Mult & 0xFFFFFF000000) | value; break;
		case 0x03: _state.MemoryDataReg = value; break;
		case 0x08: _state.RomBuffer = value; break;
		case 0x0C: 
			_state.RamBuffer[0] = value;
			_state.RamBuffer[1] = value >> 8;
			_state.RamBuffer[2] = value >> 16;
			break;

		case 0x13: _state.MemoryAddressReg = value; break;
		case 0x1C: _state.DataPointerReg = value; break;
		case 0x20: _state.PC = value; break;
		case 0x28: _state.P = (value & 0x7FFF); break;
		
		case 0x2E:
			_state.Bus.Enabled = true;
			_state.Bus.Writing = true;
			_state.Bus.DelayCycles = 1 + _state.RomAccessDelay;
			_state.Bus.Address = _state.MemoryAddressReg;
			break;

		case 0x2F:
			_state.Bus.Enabled = true;
			_state.Bus.Writing = true;
			_state.Bus.DelayCycles = 1 + _state.RamAccessDelay;
			_state.Bus.Address = _state.MemoryAddressReg;
			break;

		case 0x60: case 0x70: _state.Regs[0] = value; break;
		case 0x61: case 0x71: _state.Regs[1] = value; break;
		case 0x62: case 0x72: _state.Regs[2] = value; break;
		case 0x63: case 0x73: _state.Regs[3] = value; break;
		case 0x64: case 0x74: _state.Regs[4] = value; break;
		case 0x65: case 0x75: _state.Regs[5] = value; break;
		case 0x66: case 0x76: _state.Regs[6] = value; break;
		case 0x67: case 0x77: _state.Regs[7] = value; break;
		case 0x68: case 0x78: _state.Regs[8] = value; break;
		case 0x69: case 0x79: _state.Regs[9] = value; break;
		case 0x6A: case 0x7A: _state.Regs[10] = value; break;
		case 0x6B: case 0x7B: _state.Regs[11] = value; break;
		case 0x6C: case 0x7C: _state.Regs[12] = value; break;
		case 0x6D: case 0x7D: _state.Regs[13] = value; break;
		case 0x6E: case 0x7E: _state.Regs[14] = value; break;
		case 0x6F: case 0x7F: _state.Regs[15] = value; break;
	}
}

void Cx4::SetA(uint32_t value)
{
	_state.A = value & 0xFFFFFF;
}

void Cx4::NOP()
{
}

void Cx4::WAIT()
{
	if(_state.Bus.Enabled) {
		Step(_state.Bus.DelayCycles);
	}
}

void Cx4::Skip(uint8_t flagToCheck, uint8_t skipIfSet)
{
	bool skip;
	switch(flagToCheck) {
		default:
		case 0: skip = _state.Overflow == (skipIfSet & 0x01); break;
		case 1: skip = _state.Carry == (skipIfSet & 0x01); break;
		case 2: skip = _state.Zero == (skipIfSet & 0x01); break;
		case 3: skip = _state.Negative == (skipIfSet & 0x01); break;
	}

	if(skip) {
		_state.PC++;
		if(_state.PC == 0) {
			SwitchCachePage();
		}
		Step(1);
	}
}

void Cx4::Branch(bool branch, uint8_t far, uint8_t dest)
{
	if(branch) {
		if(far) {
			_state.PB = _state.P;
		}
		_state.PC = dest;
		Step(2);
	}
}

void Cx4::JSR(bool branch, uint8_t far, uint8_t dest)
{
	if(branch) {
		PushPC();
		if(far) {
			_state.PB = _state.P;
		}
		_state.PC = dest;
		Step(2);
	}
}

void Cx4::RTS()
{
	PullPC();
	Step(2);
}

void Cx4::PushPC()
{
	_state.Stack[_state.SP] = (_state.PB << 8) | _state.PC;
	_state.SP = (_state.SP + 1) & 0x07;
}

void Cx4::PullPC()
{
	_state.SP = (_state.SP - 1) & 0x07;
	uint32_t value = _state.Stack[_state.SP];
	_state.PB = (value >> 8) & 0x7FFF;
	_state.PC = value & 0xFF;
}

void Cx4::SetZeroNegativeFlags()
{
	_state.Zero = _state.A == 0;
	_state.Negative = _state.A & 0x800000;
}

uint32_t Cx4::AddValues(uint32_t a, uint32_t b)
{
	uint32_t result = a + b;

	_state.Carry = result > 0xFFFFFF;
	_state.Negative = result & 0x800000;
	_state.Overflow = ~(a ^ b) & (a ^ result) & 0x800000;
	_state.Zero = (result & 0xFFFFFF) == 0;

	return result & 0xFFFFFF;
}

uint32_t Cx4::Subtract(uint32_t a, uint32_t b)
{
	int32_t result = a - b;

	_state.Carry = result >= 0;
	_state.Negative = result & 0x800000;
	_state.Overflow = ~(a ^ b) & (a ^ result) & 0x800000;
	_state.Zero = result == 0;

	return result & 0xFFFFFF;
}

void Cx4::CMPR(uint8_t shift, uint8_t src)
{
	Subtract(GetSourceValue(src), _state.A << shiftLut[shift]);
}

void Cx4::CMPR_Imm(uint8_t shift, uint8_t imm)
{
	Subtract(imm, _state.A << shiftLut[shift]);
}

void Cx4::CMP(uint8_t shift, uint8_t src)
{
	Subtract(_state.A << shiftLut[shift], GetSourceValue(src));
}

void Cx4::CMP_Imm(uint8_t shift, uint8_t imm)
{
	Subtract(_state.A << shiftLut[shift], imm);
}

void Cx4::SignExtend(uint8_t mode)
{
	if(mode == 1) {
		_state.A = ((uint32_t)(int8_t)_state.A) & 0xFFFFFF;
		_state.Negative = _state.A & 0x800000;
		_state.Zero = _state.A == 0;
	} else if(mode == 2) {
		_state.A = ((uint32_t)(int16_t)_state.A) & 0xFFFFFF;
		_state.Negative = _state.A & 0x800000;
		_state.Zero = _state.A == 0;
	}
}

void Cx4::Load(uint8_t dest, uint8_t src)
{
	switch(dest) {
		case 0: _state.A = GetSourceValue(src); break;
		case 1: _state.MemoryDataReg = GetSourceValue(src); break;
		case 2: _state.MemoryAddressReg = GetSourceValue(src); break;
		case 3: _state.P = GetSourceValue(src) & 0x7FFF; break;
	}
}

void Cx4::Load_Imm(uint8_t dest, uint8_t imm)
{
	switch(dest) {
		case 0: _state.A = imm; break;
		case 1: _state.MemoryDataReg = imm; break;
		case 2: _state.MemoryAddressReg = imm; break;
		case 3: _state.P = imm; break;
	}
}

void Cx4::ADD(uint8_t shift, uint8_t src)
{
	_state.A = AddValues(_state.A << shiftLut[shift], GetSourceValue(src));
}

void Cx4::ADD_Imm(uint8_t shift, uint8_t imm)
{
	_state.A = AddValues(_state.A << shiftLut[shift], imm);
}

void Cx4::SUB(uint8_t shift, uint8_t src)
{
	_state.A = Subtract(_state.A << shiftLut[shift], GetSourceValue(src));
}

void Cx4::SUB_Imm(uint8_t shift, uint8_t imm)
{
	_state.A = Subtract(_state.A << shiftLut[shift], imm);
}

void Cx4::SUBR(uint8_t shift, uint8_t src)
{
	_state.A = Subtract(GetSourceValue(src), _state.A << shiftLut[shift]);
}

void Cx4::SUBR_Imm(uint8_t shift, uint8_t imm)
{
	_state.A = Subtract(imm, _state.A << shiftLut[shift]);
}

void Cx4::SMUL(uint8_t src)
{
	int64_t srcValue = ((int32_t)GetSourceValue(src) << 8) >> 8;
	_state.Mult = (srcValue * (((int32_t)_state.A << 8) >> 8)) & 0xFFFFFFFFFFFF;
}

void Cx4::SMUL_Imm(uint8_t imm)
{
	_state.Mult = ((int64_t)imm * (((int32_t)_state.A << 8) >> 8)) & 0xFFFFFFFFFFFF;
}

void Cx4::AND(uint8_t shift, uint8_t src)
{
	SetA((_state.A << shiftLut[shift]) & GetSourceValue(src));
	SetZeroNegativeFlags();
}

void Cx4::AND_Imm(uint8_t shift, uint8_t imm)
{
	SetA((_state.A << shiftLut[shift]) & imm);
	SetZeroNegativeFlags();
}

void Cx4::OR(uint8_t shift, uint8_t src)
{
	SetA((_state.A << shiftLut[shift]) | GetSourceValue(src));
	SetZeroNegativeFlags();
}

void Cx4::OR_Imm(uint8_t shift, uint8_t imm)
{
	SetA((_state.A << shiftLut[shift]) | imm);
	SetZeroNegativeFlags();
}

void Cx4::XOR(uint8_t shift, uint8_t src)
{
	SetA((_state.A << shiftLut[shift]) ^ GetSourceValue(src));
	SetZeroNegativeFlags();
}

void Cx4::XOR_Imm(uint8_t shift, uint8_t imm)
{
	SetA((_state.A << shiftLut[shift]) ^ imm);
	SetZeroNegativeFlags();
}

void Cx4::XNOR(uint8_t shift, uint8_t src)
{
	SetA(~(_state.A << shiftLut[shift]) ^ GetSourceValue(src));
	SetZeroNegativeFlags();
}

void Cx4::XNOR_Imm(uint8_t shift, uint8_t imm)
{
	SetA(~(_state.A << shiftLut[shift]) ^ imm);
	SetZeroNegativeFlags();
}

void Cx4::SHR(uint8_t src)
{
	uint8_t shift = GetSourceValue(src) & 0x1F;
	if(shift < 24) {
		SetA(_state.A >> shift);
	}
	SetZeroNegativeFlags();
}

void Cx4::SHR_Imm(uint8_t imm)
{
	uint8_t shift = imm & 0x1F;
	if(shift < 24) {
		SetA(_state.A >> shift);
	}
	SetZeroNegativeFlags();
}

void Cx4::ASR(uint8_t src)
{
	uint8_t shift = GetSourceValue(src) & 0x1F;
	if(shift < 24) {
		SetA((((int32_t)_state.A << 8) >> 8) >> shift);
	}
	SetZeroNegativeFlags();
}

void Cx4::ASR_Imm(uint8_t imm)
{
	uint8_t shift = imm & 0x1F;
	if(shift < 24) {
		SetA((((int32_t)_state.A << 8) >> 8) >> shift);
	}
	SetZeroNegativeFlags();
}

void Cx4::SHL(uint8_t src)
{
	uint8_t shift = GetSourceValue(src) & 0x1F;
	if(shift < 24) {
		SetA(_state.A << shift);
	}
	SetZeroNegativeFlags();
}

void Cx4::SHL_Imm(uint8_t imm)
{
	uint8_t shift = imm & 0x1F;
	if(shift < 24) {
		SetA(_state.A << shift);
	}
	SetZeroNegativeFlags();
}

void Cx4::ROR(uint8_t src)
{
	uint8_t shift = GetSourceValue(src) & 0x1F;
	if(shift < 24) {
		SetA((_state.A >> shift) | (_state.A << (24 - shift)));
	}
	SetZeroNegativeFlags();
}

void Cx4::ROR_Imm(uint8_t imm)
{
	uint8_t shift = imm & 0x1F;
	if(shift < 24) {
		SetA((_state.A >> shift) | (_state.A << (24 - shift)));
	}
	SetZeroNegativeFlags();
}

void Cx4::ReadRom()
{
	_state.RomBuffer = _dataRom[_state.A & 0x3FF];
}

void Cx4::ReadRom_Imm(uint16_t imm)
{
	_state.RomBuffer = _dataRom[imm & 0x3FF];
}

void Cx4::ReadRam(uint8_t byteIndex)
{
	if(byteIndex >= 3) {
		return;
	}

	uint16_t addr = _state.A & 0xFFF;
	if(addr >= 0xC00) {
		addr -= 0x400;
	}

	_state.RamBuffer[byteIndex] = _dataRam[addr];
}

void Cx4::ReadRam_Imm(uint8_t byteIndex, uint8_t imm)
{
	if(byteIndex >= 3) {
		return;
	}

	uint16_t addr = (_state.DataPointerReg + imm) & 0xFFF;
	if(addr >= 0xC00) {
		addr -= 0x400;
	}
	_state.RamBuffer[byteIndex] = _dataRam[addr];
}

void Cx4::WriteRam(uint8_t byteIndex)
{
	if(byteIndex >= 3) {
		return;
	}

	uint16_t addr = _state.A & 0xFFF;
	if(addr >= 0xC00) {
		addr -= 0x400;
	}

	_dataRam[addr] = _state.RamBuffer[byteIndex];
}

void Cx4::WriteRam_Imm(uint8_t byteIndex, uint8_t imm)
{
	if(byteIndex >= 3) {
		return;
	}

	uint16_t addr = (_state.DataPointerReg + imm) & 0xFFF;
	if(addr >= 0xC00) {
		addr -= 0x400;
	}
	_dataRam[addr] = _state.RamBuffer[byteIndex];
}

void Cx4::LoadP(uint8_t byteIndex, uint8_t imm)
{
	switch(byteIndex) {
		case 0: _state.P = (_state.P & 0x7F00) | imm; break;
		case 1: _state.P = (_state.P & 0xFF) | ((imm & 0x7F) << 8); break;
		default: break; //nop
	}
}

void Cx4::Swap(uint8_t reg)
{
	uint32_t tmp = _state.A;
	_state.A = _state.Regs[reg];
	_state.Regs[reg] = tmp;
}

void Cx4::Store(uint8_t src, uint8_t dst)
{
	switch(src) {
		case 0: WriteRegister(dst, _state.A); break;
		case 1: WriteRegister(dst, _state.MemoryDataReg); break;
		default: break; //nop
	}
}

void Cx4::Stop()
{
	_state.Stopped = true;
	if(!_state.IrqDisabled) {
		_state.IrqFlag = true;
		_cpu->SetIrqSource(SnesIrqSource::Coprocessor);
	}
}

void Cx4::IncMar()
{
	_state.MemoryAddressReg = (_state.MemoryAddressReg + 1) & 0xFFFFFF;
}

//Data ROM that contains precalculated math lookup tables for sines, cosines, division, etc.
const uint32_t _dataRom[1024] = {
	0xFFFFFF, 0x800000, 0x400000, 0x2AAAAA, 0x200000, 0x199999, 0x155555, 0x124924, 0x100000, 0x0E38E3, 0x0CCCCC, 0x0BA2E8, 0x0AAAAA, 0x09D89D, 0x092492, 0x088888,
	0x080000, 0x078787, 0x071C71, 0x06BCA1, 0x066666, 0x061861, 0x05D174, 0x0590B2, 0x055555, 0x051EB8, 0x04EC4E, 0x04BDA1, 0x049249, 0x0469EE, 0x044444, 0x042108,
	0x040000, 0x03E0F8, 0x03C3C3, 0x03A83A, 0x038E38, 0x03759F, 0x035E50, 0x034834, 0x033333, 0x031F38, 0x030C30, 0x02FA0B, 0x02E8BA, 0x02D82D, 0x02C859, 0x02B931,
	0x02AAAA, 0x029CBC, 0x028F5C, 0x028282, 0x027627, 0x026A43, 0x025ED0, 0x0253C8, 0x024924, 0x023EE0, 0x0234F7, 0x022B63, 0x022222, 0x02192E, 0x021084, 0x020820,
	0x020000, 0x01F81F, 0x01F07C, 0x01E913, 0x01E1E1, 0x01DAE6, 0x01D41D, 0x01CD85, 0x01C71C, 0x01C0E0, 0x01BACF, 0x01B4E8, 0x01AF28, 0x01A98E, 0x01A41A, 0x019EC8,
	0x019999, 0x01948B, 0x018F9C, 0x018ACB, 0x018618, 0x018181, 0x017D05, 0x0178A4, 0x01745D, 0x01702E, 0x016C16, 0x016816, 0x01642C, 0x016058, 0x015C98, 0x0158ED,
	0x015555, 0x0151D0, 0x014E5E, 0x014AFD, 0x0147AE, 0x01446F, 0x014141, 0x013E22, 0x013B13, 0x013813, 0x013521, 0x01323E, 0x012F68, 0x012C9F, 0x0129E4, 0x012735,
	0x012492, 0x0121FB, 0x011F70, 0x011CF0, 0x011A7B, 0x011811, 0x0115B1, 0x01135C, 0x011111, 0x010ECF, 0x010C97, 0x010A68, 0x010842, 0x010624, 0x010410, 0x010204,
	0x010000, 0x00FE03, 0x00FC0F, 0x00FA23, 0x00F83E, 0x00F660, 0x00F489, 0x00F2B9, 0x00F0F0, 0x00EF2E, 0x00ED73, 0x00EBBD, 0x00EA0E, 0x00E865, 0x00E6C2, 0x00E525,
	0x00E38E, 0x00E1FC, 0x00E070, 0x00DEE9, 0x00DD67, 0x00DBEB, 0x00DA74, 0x00D901, 0x00D794, 0x00D62B, 0x00D4C7, 0x00D368, 0x00D20D, 0x00D0B6, 0x00CF64, 0x00CE16,
	0x00CCCC, 0x00CB87, 0x00CA45, 0x00C907, 0x00C7CE, 0x00C698, 0x00C565, 0x00C437, 0x00C30C, 0x00C1E4, 0x00C0C0, 0x00BFA0, 0x00BE82, 0x00BD69, 0x00BC52, 0x00BB3E,
	0x00BA2E, 0x00B921, 0x00B817, 0x00B70F, 0x00B60B, 0x00B509, 0x00B40B, 0x00B30F, 0x00B216, 0x00B11F, 0x00B02C, 0x00AF3A, 0x00AE4C, 0x00AD60, 0x00AC76, 0x00AB8F,
	0x00AAAA, 0x00A9C8, 0x00A8E8, 0x00A80A, 0x00A72F, 0x00A655, 0x00A57E, 0x00A4A9, 0x00A3D7, 0x00A306, 0x00A237, 0x00A16B, 0x00A0A0, 0x009FD8, 0x009F11, 0x009E4C,
	0x009D89, 0x009CC8, 0x009C09, 0x009B4C, 0x009A90, 0x0099D7, 0x00991F, 0x009868, 0x0097B4, 0x009701, 0x00964F, 0x0095A0, 0x0094F2, 0x009445, 0x00939A, 0x0092F1,
	0x009249, 0x0091A2, 0x0090FD, 0x00905A, 0x008FB8, 0x008F17, 0x008E78, 0x008DDA, 0x008D3D, 0x008CA2, 0x008C08, 0x008B70, 0x008AD8, 0x008A42, 0x0089AE, 0x00891A,
	0x008888, 0x0087F7, 0x008767, 0x0086D9, 0x00864B, 0x0085BF, 0x008534, 0x0084A9, 0x008421, 0x008399, 0x008312, 0x00828C, 0x008208, 0x008184, 0x008102, 0x008080,
	0x000000, 0x100000, 0x16A09E, 0x1BB67A, 0x200000, 0x23C6EF, 0x27311C, 0x2A54FF, 0x2D413C, 0x300000, 0x3298B0, 0x3510E5, 0x376CF5, 0x39B056, 0x3BDDD4, 0x3DF7BD,
	0x400000, 0x41F83D, 0x43E1DB, 0x45BE0C, 0x478DDE, 0x49523A, 0x4B0BF1, 0x4CBBB9, 0x4E6238, 0x500000, 0x519595, 0x532370, 0x54A9FE, 0x5629A2, 0x57A2B7, 0x591590,
	0x5A8279, 0x5BE9BA, 0x5D4B94, 0x5EA843, 0x600000, 0x6152FE, 0x62A170, 0x63EB83, 0x653160, 0x667332, 0x67B11D, 0x68EB44, 0x6A21CA, 0x6B54CD, 0x6C846C, 0x6DB0C2,
	0x6ED9EB, 0x700000, 0x712318, 0x72434A, 0x7360AD, 0x747B54, 0x759354, 0x76A8BF, 0x77BBA8, 0x78CC1F, 0x79DA34, 0x7AE5F9, 0x7BEF7A, 0x7CF6C8, 0x7DFBEF, 0x7EFEFD,
	0x800000, 0x80FF01, 0x81FC0F, 0x82F734, 0x83F07B, 0x84E7EE, 0x85DD98, 0x86D182, 0x87C3B6, 0x88B43D, 0x89A31F, 0x8A9066, 0x8B7C19, 0x8C6641, 0x8D4EE4, 0x8E360B,
	0x8F1BBC, 0x900000, 0x90E2DB, 0x91C456, 0x92A475, 0x938341, 0x9460BD, 0x953CF1, 0x9617E2, 0x96F196, 0x97CA11, 0x98A159, 0x997773, 0x9A4C64, 0x9B2031, 0x9BF2DE,
	0x9CC470, 0x9D94EB, 0x9E6454, 0x9F32AF, 0xA00000, 0xA0CC4A, 0xA19792, 0xA261DC, 0xA32B2A, 0xA3F382, 0xA4BAE6, 0xA5815A, 0xA646E1, 0xA70B7E, 0xA7CF35, 0xA89209,
	0xA953FD, 0xAA1513, 0xAAD550, 0xAB94B4, 0xAC5345, 0xAD1103, 0xADCDF2, 0xAE8A15, 0xAF456E, 0xB00000, 0xB0B9CC, 0xB172D6, 0xB22B20, 0xB2E2AC, 0xB3997C, 0xB44F93,
	0xB504F3, 0xB5B99D, 0xB66D95, 0xB720DC, 0xB7D375, 0xB88560, 0xB936A0, 0xB9E738, 0xBA9728, 0xBB4673, 0xBBF51A, 0xBCA320, 0xBD5086, 0xBDFD4E, 0xBEA979, 0xBF5509,
	0xC00000, 0xC0AA5F, 0xC15428, 0xC1FD5C, 0xC2A5FD, 0xC34E0D, 0xC3F58C, 0xC49C7D, 0xC542E1, 0xC5E8B8, 0xC68E05, 0xC732C9, 0xC7D706, 0xC87ABB, 0xC91DEB, 0xC9C098,
	0xCA62C1, 0xCB0469, 0xCBA591, 0xCC463A, 0xCCE664, 0xCD8612, 0xCE2544, 0xCEC3FC, 0xCF623A, 0xD00000, 0xD09D4E, 0xD13A26, 0xD1D689, 0xD27277, 0xD30DF3, 0xD3A8FC,
	0xD44394, 0xD4DDBC, 0xD57774, 0xD610BE, 0xD6A99B, 0xD7420B, 0xD7DA0F, 0xD871A9, 0xD908D8, 0xD99F9F, 0xDA35FE, 0xDACBF5, 0xDB6185, 0xDBF6B0, 0xDC8B76, 0xDD1FD8,
	0xDDB3D7, 0xDE4773, 0xDEDAAD, 0xDF6D86, 0xE00000, 0xE09219, 0xE123D4, 0xE1B530, 0xE24630, 0xE2D6D2, 0xE36719, 0xE3F704, 0xE48694, 0xE515CB, 0xE5A4A8, 0xE6332D,
	0xE6C15A, 0xE74F2F, 0xE7DCAD, 0xE869D6, 0xE8F6A9, 0xE98326, 0xEA0F50, 0xEA9B26, 0xEB26A8, 0xEBB1D9, 0xEC3CB7, 0xECC743, 0xED517F, 0xEDDB6A, 0xEE6506, 0xEEEE52,
	0xEF7750, 0xF00000, 0xF08861, 0xF11076, 0xF1983E, 0xF21FBA, 0xF2A6EA, 0xF32DCF, 0xF3B469, 0xF43AB9, 0xF4C0C0, 0xF5467D, 0xF5CBF2, 0xF6511E, 0xF6D602, 0xF75A9F,
	0xF7DEF5, 0xF86305, 0xF8E6CE, 0xF96A52, 0xF9ED90, 0xFA708A, 0xFAF33F, 0xFB75B1, 0xFBF7DF, 0xFC79CA, 0xFCFB72, 0xFD7CD8, 0xFDFDFB, 0xFE7EDE, 0xFEFF7F, 0xFF7FDF,
	0x000000, 0x03243A, 0x064855, 0x096C32, 0x0C8FB2, 0x0FB2B7, 0x12D520, 0x15F6D0, 0x1917A6, 0x1C3785, 0x1F564E, 0x2273E1, 0x259020, 0x28AAED, 0x2BC428, 0x2EDBB3,
	0x31F170, 0x350540, 0x381704, 0x3B269F, 0x3E33F2, 0x413EE0, 0x444749, 0x474D10, 0x4A5018, 0x4D5043, 0x504D72, 0x534789, 0x563E69, 0x5931F7, 0x5C2214, 0x5F0EA4,
	0x61F78A, 0x64DCA9, 0x67BDE5, 0x6A9B20, 0x6D7440, 0x704927, 0x7319BA, 0x75E5DD, 0x78AD74, 0x7B7065, 0x7E2E93, 0x80E7E4, 0x839C3C, 0x864B82, 0x88F59A, 0x8B9A6B,
	0x8E39D9, 0x90D3CC, 0x93682A, 0x95F6D9, 0x987FBF, 0x9B02C5, 0x9D7FD1, 0x9FF6CA, 0xA26799, 0xA4D224, 0xA73655, 0xA99414, 0xABEB49, 0xAE3BDD, 0xB085BA, 0xB2C8C9,
	0xB504F3, 0xB73A22, 0xB96841, 0xBB8F3A, 0xBDAEF9, 0xBFC767, 0xC1D870, 0xC3E200, 0xC5E403, 0xC7DE65, 0xC9D112, 0xCBBBF7, 0xCD9F02, 0xCF7A1F, 0xD14D3D, 0xD31848,
	0xD4DB31, 0xD695E4, 0xD84852, 0xD9F269, 0xDB941A, 0xDD2D53, 0xDEBE05, 0xE04621, 0xE1C597, 0xE33C59, 0xE4AA59, 0xE60F87, 0xE76BD7, 0xE8BF3B, 0xEA09A6, 0xEB4B0B,
	0xEC835E, 0xEDB293, 0xEED89D, 0xEFF573, 0xF10908, 0xF21352, 0xF31447, 0xF40BDD, 0xF4FA0A, 0xF5DEC6, 0xF6BA07, 0xF78BC5, 0xF853F7, 0xF91297, 0xF9C79D, 0xFA7301,
	0xFB14BE, 0xFBACCD, 0xFC3B27, 0xFCBFC9, 0xFD3AAB, 0xFDABCB, 0xFE1323, 0xFE70AF, 0xFEC46D, 0xFF0E57, 0xFF4E6D, 0xFF84AB, 0xFFB10F, 0xFFD397, 0xFFEC43, 0xFFFB10,
	0x000000, 0x00A2F9, 0x0145F6, 0x01E8F8, 0x028C01, 0x032F14, 0x03D234, 0x047564, 0x0518A5, 0x05BBFB, 0x065F68, 0x0702EF, 0x07A692, 0x084A54, 0x08EE38, 0x099240,
	0x0A366E, 0x0ADAC7, 0x0B7F4C, 0x0C2401, 0x0CC8E7, 0x0D6E02, 0x0E1355, 0x0EB8E3, 0x0F5EAE, 0x1004B9, 0x10AB08, 0x11519E, 0x11F87D, 0x129FA9, 0x134725, 0x13EEF4,
	0x149719, 0x153F99, 0x15E875, 0x1691B2, 0x173B53, 0x17E55C, 0x188FD1, 0x193AB4, 0x19E60A, 0x1A91D8, 0x1B3E20, 0x1BEAE7, 0x1C9831, 0x1D4602, 0x1DF45F, 0x1EA34C,
	0x1F52CE, 0x2002EA, 0x20B3A3, 0x216500, 0x221705, 0x22C9B8, 0x237D1E, 0x24313C, 0x24E618, 0x259BB9, 0x265224, 0x27095F, 0x27C171, 0x287A61, 0x293436, 0x29EEF6,
	0x2AAAAA, 0x2B6759, 0x2C250A, 0x2CE3C7, 0x2DA398, 0x2E6485, 0x2F2699, 0x2FE9DC, 0x30AE59, 0x31741B, 0x323B2C, 0x330398, 0x33CD6B, 0x3498B1, 0x356578, 0x3633CE,
	0x3703C1, 0x37D560, 0x38A8BB, 0x397DE4, 0x3A54EC, 0x3B2DE6, 0x3C08E6, 0x3CE601, 0x3DC54D, 0x3EA6E3, 0x3F8ADC, 0x407152, 0x415A62, 0x42462C, 0x4334D0, 0x442671,
	0x451B37, 0x46134A, 0x470ED6, 0x480E0C, 0x491120, 0x4A184C, 0x4B23CD, 0x4C33EA, 0x4D48EC, 0x4E6327, 0x4F82F9, 0x50A8C9, 0x51D50A, 0x53083F, 0x5442FC, 0x5585EA,
	0x56D1CC, 0x582782, 0x598815, 0x5AF4BC, 0x5C6EED, 0x5DF86C, 0x5F9369, 0x6142A3, 0x6309A5, 0x64ED1E, 0x66F381, 0x692617, 0x6B9322, 0x6E52A5, 0x71937C, 0x75CEB4,
	0x000000, 0x000324, 0x000648, 0x00096D, 0x000C93, 0x000FBA, 0x0012E2, 0x00160B, 0x001936, 0x001C63, 0x001F93, 0x0022C4, 0x0025F9, 0x002930, 0x002C6B, 0x002FA9,
	0x0032EB, 0x003632, 0x00397C, 0x003CCB, 0x00401F, 0x004379, 0x0046D8, 0x004A3D, 0x004DA8, 0x005119, 0x005492, 0x005811, 0x005B99, 0x005F28, 0x0062C0, 0x006660,
	0x006A09, 0x006DBC, 0x00717A, 0x007541, 0x007914, 0x007CF2, 0x0080DC, 0x0084D2, 0x0088D5, 0x008CE6, 0x009105, 0x009533, 0x009970, 0x009DBE, 0x00A21C, 0x00A68B,
	0x00AB0D, 0x00AFA2, 0x00B44B, 0x00B909, 0x00BDDC, 0x00C2C6, 0x00C7C8, 0x00CCE3, 0x00D218, 0x00D767, 0x00DCD3, 0x00E25D, 0x00E806, 0x00EDCF, 0x00F3BB, 0x00F9CA,
	0x010000, 0x01065C, 0x010CE2, 0x011394, 0x011A73, 0x012183, 0x0128C6, 0x01303E, 0x0137EF, 0x013FDC, 0x014808, 0x015077, 0x01592D, 0x01622D, 0x016B7D, 0x017522,
	0x017F21, 0x018980, 0x019444, 0x019F76, 0x01AB1C, 0x01B73E, 0x01C3E7, 0x01D11F, 0x01DEF1, 0x01ED69, 0x01FC95, 0x020C83, 0x021D44, 0x022EE9, 0x024186, 0x025533,
	0x026A09, 0x028025, 0x0297A7, 0x02B0B5, 0x02CB78, 0x02E823, 0x0306EC, 0x032815, 0x034BEB, 0x0372C6, 0x039D10, 0x03CB47, 0x03FE02, 0x0435F7, 0x047405, 0x04B93F,
	0x0506FF, 0x055EF9, 0x05C35D, 0x063709, 0x06BDCF, 0x075CE6, 0x081B97, 0x09046D, 0x0A2736, 0x0B9CC6, 0x0D8E81, 0x1046E9, 0x145AFF, 0x1B2671, 0x28BC48, 0x517BB5,
	0xFFFFFF, 0xFFFB10, 0xFFEC43, 0xFFD397, 0xFFB10F, 0xFF84AB, 0xFF4E6D, 0xFF0E57, 0xFEC46D, 0xFE70AF, 0xFE1323, 0xFDABCB, 0xFD3AAB, 0xFCBFC9, 0xFC3B27, 0xFBACCD,
	0xFB14BE, 0xFA7301, 0xF9C79D, 0xF91297, 0xF853F7, 0xF78BC5, 0xF6BA07, 0xF5DEC6, 0xF4FA0A, 0xF40BDD, 0xF31447, 0xF21352, 0xF10908, 0xEFF573, 0xEED89D, 0xEDB293,
	0xEC835E, 0xEB4B0B, 0xEA09A6, 0xE8BF3B, 0xE76BD7, 0xE60F87, 0xE4AA59, 0xE33C59, 0xE1C597, 0xE04621, 0xDEBE05, 0xDD2D53, 0xDB941A, 0xD9F269, 0xD84852, 0xD695E4,
	0xD4DB31, 0xD31848, 0xD14D3D, 0xCF7A1F, 0xCD9F02, 0xCBBBF7, 0xC9D112, 0xC7DE65, 0xC5E403, 0xC3E200, 0xC1D870, 0xBFC767, 0xBDAEF9, 0xBB8F3A, 0xB96841, 0xB73A22,
	0xB504F3, 0xB2C8C9, 0xB085BA, 0xAE3BDD, 0xABEB49, 0xA99414, 0xA73655, 0xA4D224, 0xA26799, 0x9FF6CA, 0x9D7FD1, 0x9B02C5, 0x987FBF, 0x95F6D9, 0x93682A, 0x90D3CC,
	0x8E39D9, 0x8B9A6B, 0x88F59A, 0x864B82, 0x839C3C, 0x80E7E4, 0x7E2E93, 0x7B7065, 0x78AD74, 0x75E5DD, 0x7319BA, 0x704927, 0x6D7440, 0x6A9B20, 0x67BDE5, 0x64DCA9,
	0x61F78A, 0x5F0EA4, 0x5C2214, 0x5931F7, 0x563E69, 0x534789, 0x504D72, 0x4D5043, 0x4A5018, 0x474D10, 0x444749, 0x413EE0, 0x3E33F2, 0x3B269F, 0x381704, 0x350540,
	0x31F170, 0x2EDBB3, 0x2BC428, 0x28AAED, 0x259020, 0x2273E1, 0x1F564E, 0x1C3785, 0x1917A6, 0x15F6D0, 0x12D520, 0x0FB2B7, 0x0C8FB2, 0x096C32, 0x064855, 0x03243A
};
