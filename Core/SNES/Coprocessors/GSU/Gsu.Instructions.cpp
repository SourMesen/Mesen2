#include "pch.h"
#include "SNES/Coprocessors/GSU/Gsu.h"
#include "SNES/SnesCpu.h"
#include "SNES/SnesMemoryManager.h"

void Gsu::STOP()
{
	if(!_state.IrqDisabled) {
		_state.SFR.Irq = true;
		_cpu->SetIrqSource(SnesIrqSource::Coprocessor);
	}
	_state.ProgramReadBuffer = 0x01; //Run a NOP first when the GSU is restarted
	_state.SFR.Running = false;
	ResetFlags();

	UpdateRunningState();
}

void Gsu::NOP()
{
	ResetFlags();
}

void Gsu::CACHE()
{
	if(_state.CacheBase != (_state.R[15] & 0xFFF0)) {
		_state.CacheBase = _state.R[15] & 0xFFF0;
		InvalidateCache();
	}
	ResetFlags();
}

void Gsu::Branch(bool branch)
{
	int8_t offset = (int8_t)ReadOperand();
	if(branch) {
		WriteRegister(15, _state.R[15] + offset);
	}
}

void Gsu::BRA()
{
	Branch(true);
}

void Gsu::BLT()
{
	Branch(_state.SFR.Sign != _state.SFR.Overflow);
}

void Gsu::BGE()
{
	Branch(_state.SFR.Sign == _state.SFR.Overflow);
}

void Gsu::BNE()
{
	Branch(!_state.SFR.Zero);
}

void Gsu::BEQ()
{
	Branch(_state.SFR.Zero);
}

void Gsu::BPL()
{
	Branch(!_state.SFR.Sign);
}

void Gsu::BMI()
{
	Branch(_state.SFR.Sign);
}

void Gsu::BCC()
{
	Branch(!_state.SFR.Carry);
}

void Gsu::BCS()
{
	Branch(_state.SFR.Carry);
}

void Gsu::BVC()
{
	Branch(!_state.SFR.Overflow);
}

void Gsu::BVS()
{
	Branch(_state.SFR.Overflow);
}

void Gsu::JMP(uint8_t reg)
{
	if(_state.SFR.Alt1) {
		//LJMP
		_state.ProgramBank = _state.R[reg] & 0x7F;
		WriteRegister(15, ReadSrcReg());
		
		_state.CacheBase = _state.R[15] & 0xFFF0;
		InvalidateCache();
	} else {
		//JMP
		WriteRegister(15, _state.R[reg]);
	}
	ResetFlags();
}

void Gsu::TO(uint8_t reg)
{
	if(_state.SFR.Prefix) {
		//MOVE
		WriteRegister(reg, ReadSrcReg());
		ResetFlags();
	} else {
		//TO
		_state.DestReg = reg;
	}
}

void Gsu::FROM(uint8_t reg)
{
	if(_state.SFR.Prefix) {
		//MOVES
		WriteDestReg(_state.R[reg]);
		_state.SFR.Overflow = (_state.R[reg] & 0x80) != 0;
		_state.SFR.Sign = (_state.R[reg] & 0x8000) != 0;
		_state.SFR.Zero = (_state.R[reg] == 0);
		ResetFlags();
	} else {
		//FROM
		_state.SrcReg = reg;
	}
}

void Gsu::WITH(uint8_t reg)
{
	_state.SrcReg = reg;
	_state.DestReg = reg;
	_state.SFR.Prefix = true;
}

void Gsu::STORE(uint8_t reg)
{
	_state.RamAddress = _state.R[reg];
	WriteRam(_state.RamAddress, (uint8_t)ReadSrcReg());
	if(!_state.SFR.Alt1) {
		WriteRam(_state.RamAddress ^ 0x01, ReadSrcReg() >> 8);
	}
	ResetFlags();
}

void Gsu::LOAD(uint8_t reg)
{
	_state.RamAddress = _state.R[reg];
	uint16_t value = ReadRamBuffer(_state.RamAddress);
	if(!_state.SFR.Alt1) {
		value |= ReadRamBuffer(_state.RamAddress ^ 0x01) << 8;
	}
	WriteDestReg(value);
	ResetFlags();
}

void Gsu::LOOP()
{
	_state.R[12]--;
	_state.SFR.Zero = (_state.R[12] == 0);
	_state.SFR.Sign = (_state.R[12] & 0x8000) != 0;
	
	//Loop until counter hits zero
	if(!_state.SFR.Zero) {
		WriteRegister(15, _state.R[13]);
	}

	ResetFlags();
}

void Gsu::ALT1()
{
	_state.SFR.Prefix = false;
	_state.SFR.Alt1 = true;
}

void Gsu::ALT2()
{
	_state.SFR.Prefix = false;
	_state.SFR.Alt2 = true;
}

void Gsu::ALT3()
{
	_state.SFR.Prefix = false;
	_state.SFR.Alt1 = true;
	_state.SFR.Alt2 = true;
}

void Gsu::MERGE()
{
	uint16_t value = (_state.R[7] & 0xFF00) | (_state.R[8] >> 8);
	WriteDestReg(value);
	_state.SFR.Carry = (value & 0xE0E0) != 0;
	_state.SFR.Overflow = (value & 0xC0C0) != 0;
	_state.SFR.Sign = (value & 0x8080) != 0;
	_state.SFR.Zero = (value & 0xF0F0) != 0;
	ResetFlags();
}

void Gsu::SWAP()
{
	uint16_t value = (ReadSrcReg() >> 8) | (ReadSrcReg() << 8);
	WriteDestReg(value);
	_state.SFR.Zero = value == 0;
	_state.SFR.Sign = (value & 0x8000) != 0;
	ResetFlags();
}

void Gsu::Add(uint8_t reg)
{
	uint16_t operand;
	if(_state.SFR.Alt2) {
		//Immediate value
		operand = reg;
	} else {
		operand = _state.R[reg];
	}

	uint32_t result = ReadSrcReg() + operand;
	if(_state.SFR.Alt1) {
		//ADC - Add with carry
		result += (uint8_t)_state.SFR.Carry;
	}

	_state.SFR.Carry = (result & 0x10000) != 0;
	_state.SFR.Overflow = ~(ReadSrcReg() ^ operand) & (operand ^ result) & 0x8000;
	_state.SFR.Sign = (result & 0x8000) != 0;
	_state.SFR.Zero = (result & 0xFFFF) == 0;

	WriteDestReg(result);
	ResetFlags();
}

void Gsu::SubCompare(uint8_t reg)
{
	uint16_t operand;
	if(_state.SFR.Alt2 && !_state.SFR.Alt1) {
		//Immediate value, SUB #val
		operand = reg;
	} else {
		operand = _state.R[reg];
	}

	int32_t result = ReadSrcReg() - operand;
	if(!_state.SFR.Alt2 && _state.SFR.Alt1) {
		//SBC - SUB with carry
		result -= _state.SFR.Carry ? 0 : 1;
	}

	_state.SFR.Carry = (result >= 0);
	_state.SFR.Overflow = ((ReadSrcReg() ^ operand) & (ReadSrcReg() ^ result) & 0x8000) != 0;
	_state.SFR.Sign = (result & 0x8000) != 0;
	_state.SFR.Zero = (result & 0xFFFF) == 0;

	if(!_state.SFR.Alt2 || !_state.SFR.Alt1) {
		//SUB/SBC, other CMP (and no write occurs for CMP)
		WriteDestReg(result);
	}
	ResetFlags();
}

void Gsu::MULT(uint8_t reg)
{
	uint16_t operand;
	if(_state.SFR.Alt2) {
		//Immediate value
		operand = reg;
	} else {
		operand = _state.R[reg];
	}

	uint16_t value;
	if(_state.SFR.Alt1) {
		//UMULT - Unsigned multiply
		value = (uint16_t)((uint8_t)ReadSrcReg() * (uint8_t)operand);
	} else {
		//MULT - Signed multiply
		value = (uint16_t)((int8_t)ReadSrcReg() * (int8_t)operand);
	}
	
	WriteDestReg(value);
	_state.SFR.Sign = (value & 0x8000) != 0;
	_state.SFR.Zero = value == 0;

	ResetFlags();

	Step(_state.HighSpeedMode ? 1 : 2);
}

void Gsu::FMultLMult()
{
	uint32_t multResult = (int16_t)ReadSrcReg() * (int16_t)_state.R[6];

	if(_state.SFR.Alt1) {
		//LMULT - "16x16 signed multiply", LSB in R4, MSB in DREG
		_state.R[4] = multResult;
	}

	uint16_t value = multResult >> 16;
	WriteDestReg(value);

	_state.SFR.Carry = (multResult & 0x8000);
	_state.SFR.Sign = (value & 0x8000);
	_state.SFR.Zero = (value == 0);
	ResetFlags();

	Step((_state.HighSpeedMode ? 3 : 7) * (_state.ClockSelect ? 1 : 2));
}

void Gsu::AndBitClear(uint8_t reg)
{
	uint16_t operand;
	if(_state.SFR.Alt2) {
		//Immediate value
		operand = reg;
	} else {
		operand = _state.R[reg];
	}

	uint16_t value;
	if(_state.SFR.Alt1) {
		//Bit clear
		value = ReadSrcReg() & ~operand;
	} else {
		//AND
		value = ReadSrcReg() & operand;
	}

	WriteDestReg(value);
	_state.SFR.Zero = value == 0;
	_state.SFR.Sign = (value & 0x8000) != 0;
	ResetFlags();
}

void Gsu::SBK()
{
	//"Store word data, last RAM address used"
	WriteRam(_state.RamAddress, (uint8_t)ReadSrcReg());
	WriteRam(_state.RamAddress ^ 1, ReadSrcReg() >> 8);
	ResetFlags();
}

void Gsu::LINK(uint8_t value)
{
	//"Link Return Address"
	_state.R[11] = _state.R[15] + value;
	ResetFlags();
}

void Gsu::SignExtend()
{
	int16_t value = (int8_t)ReadSrcReg();
	WriteDestReg((uint16_t)value);
	_state.SFR.Zero = value == 0;
	_state.SFR.Sign = (value & 0x8000) != 0;
	ResetFlags();
}

void Gsu::NOT()
{
	uint16_t value = ~ReadSrcReg();
	WriteDestReg(value);
	_state.SFR.Zero = value == 0;
	_state.SFR.Sign = (value & 0x8000) != 0;
	ResetFlags();
}

void Gsu::LSR()
{
	uint16_t src = ReadSrcReg();
	_state.SFR.Carry = (src & 0x01) != 0;
	
	uint16_t dst = src >> 1;
	WriteDestReg(dst);
	_state.SFR.Zero = dst == 0;
	_state.SFR.Sign = (dst & 0x8000) != 0;
	ResetFlags();
}

void Gsu::ROL()
{
	uint16_t src = ReadSrcReg();

	uint16_t dst = (src << 1) | (uint8_t)_state.SFR.Carry;
	_state.SFR.Carry = (src & 0x8000) != 0;

	WriteDestReg(dst);
	_state.SFR.Zero = dst == 0;
	_state.SFR.Sign = (dst & 0x8000) != 0;
	ResetFlags();
}

void Gsu::ASR()
{
	uint16_t src = ReadSrcReg();
	_state.SFR.Carry = (src & 0x01) != 0;

	uint16_t dst = (int16_t)src >> 1;
	if(_state.SFR.Alt1) {
		dst += (src + 1) >> 16;
	}

	WriteDestReg(dst);
	_state.SFR.Zero = dst == 0;
	_state.SFR.Sign = (dst & 0x8000) != 0;
	ResetFlags();
}

void Gsu::ROR()
{
	uint16_t src = ReadSrcReg();
	
	uint16_t dst = (src >> 1) | ((int)_state.SFR.Carry << 15);
	_state.SFR.Carry = (src & 0x01) != 0;

	WriteDestReg(dst);
	_state.SFR.Zero = dst == 0;
	_state.SFR.Sign = (dst & 0x8000) != 0;
	ResetFlags();
}

void Gsu::LOB()
{
	//"Value of low byte of register"
	uint8_t value = (uint8_t)ReadSrcReg();
	WriteDestReg(value);
	_state.SFR.Zero = value == 0;
	_state.SFR.Sign = (value & 0x80) != 0;
	ResetFlags();
}

void Gsu::HIB()
{
	//"Value of high byte of register"
	uint8_t value = (uint8_t)(ReadSrcReg() >> 8);
	WriteDestReg(value);
	_state.SFR.Zero = value == 0;
	_state.SFR.Sign = (value & 0x80) != 0;
	ResetFlags();
}

void Gsu::IbtSmsLms(uint8_t reg)
{
	if(_state.SFR.Alt1) {
		//LMS - "Load word data from RAM, short address"
		_state.RamAddress = ReadOperand() << 1;
		uint8_t lsb = ReadRamBuffer(_state.RamAddress);
		uint8_t msb = ReadRamBuffer(_state.RamAddress | 0x01);

		WriteRegister(reg, (msb << 8) | lsb);
	} else if(_state.SFR.Alt2) {
		//SMS - "Store word data to RAM, short address"
		_state.RamAddress = ReadOperand() << 1;
		WriteRam(_state.RamAddress, (uint8_t)_state.R[reg]);
		WriteRam(_state.RamAddress | 0x01, _state.R[reg] >> 8);
	} else {
		//IBT - "Load immediate byte data"
		WriteRegister(reg, (int8_t)ReadOperand());
	}
	ResetFlags();
}

void Gsu::IwtLmSm(uint8_t reg)
{
	if(_state.SFR.Alt1) {
		//LM - Load memory
		_state.RamAddress = ReadOperand();
		_state.RamAddress |= ReadOperand() << 8;

		uint8_t lsb = ReadRamBuffer(_state.RamAddress);
		uint8_t msb = ReadRamBuffer(_state.RamAddress ^ 0x01);
		WriteRegister(reg, (msb << 8) | lsb);
	} else if(_state.SFR.Alt2) {
		//SM - Store Memory
		_state.RamAddress = ReadOperand();
		_state.RamAddress |= ReadOperand() << 8;

		WriteRam(_state.RamAddress, (uint8_t)_state.R[reg]);
		WriteRam(_state.RamAddress ^ 0x01, _state.R[reg] >> 8);
	} else {
		//IWT - Load immediate word
		uint8_t lsb = ReadOperand();
		uint8_t msb = ReadOperand();
		WriteRegister(reg, (msb << 8) | lsb);
	}
	ResetFlags();
}

void Gsu::OrXor(uint8_t operand)
{
	uint16_t operandValue;
	if(_state.SFR.Alt2) {
		//Immediate value
		operandValue = operand;
	} else {
		//Indirect register value
		operandValue = _state.R[operand];
	}

	uint16_t value;
	if(_state.SFR.Alt1) {
		//XOR
		value = ReadSrcReg() ^ operandValue;
	} else {
		//OR
		value = ReadSrcReg() | operandValue;
	}
	WriteDestReg(value);
	_state.SFR.Zero = value == 0;
	_state.SFR.Sign = (value & 0x8000) != 0;
	ResetFlags();
}

void Gsu::INC(uint8_t reg)
{
	WriteRegister(reg, _state.R[reg] + 1);
	_state.SFR.Zero = _state.R[reg] == 0;
	_state.SFR.Sign = (_state.R[reg] & 0x8000) != 0;
	ResetFlags();
}

void Gsu::DEC(uint8_t reg)
{
	WriteRegister(reg, _state.R[reg] - 1);
	_state.SFR.Zero = _state.R[reg] == 0;
	_state.SFR.Sign = (_state.R[reg] & 0x8000) != 0;
	ResetFlags();
}

void Gsu::GetCRamBRomB()
{
	if(!_state.SFR.Alt2) {
		//GETC - "Get byte from ROM to color register"
		_state.ColorReg = GetColor(ReadRomBuffer());
	} else if(!_state.SFR.Alt1) {
		//RAMB - "Set RAM data bank"
		WaitRamOperation();
		_state.RamBank = ReadSrcReg() & 0x01;
	} else {
		//ROMB - "Set ROM data bank"
		WaitRomOperation();
		_state.RomBank = ReadSrcReg() & 0x7F;
	}
	ResetFlags();
}

void Gsu::GETB()
{
	if(_state.SFR.Alt2 && _state.SFR.Alt1) {
		//GETBS - "Get signed byte from ROM buffer"
		WriteDestReg((int8_t)ReadRomBuffer());
	} else if(_state.SFR.Alt2) {
		//GETBL - "Get low byte from ROM buffer"
		WriteDestReg((ReadSrcReg() & 0xFF00) | ReadRomBuffer());
	} else if(_state.SFR.Alt1) {
		//GETBH - "Get high byte from ROM buffer"
		WriteDestReg((ReadSrcReg() & 0xFF) | (ReadRomBuffer() << 8));
	} else {
		//GETB - "Get byte from ROM buffer"
		WriteDestReg(ReadRomBuffer());
	}
	ResetFlags();
}

void Gsu::PlotRpix()
{
	if(_state.SFR.Alt1) {
		//RPIX - "Read pixel color"
		uint8_t value = ReadPixel((uint8_t)_state.R[1], (uint8_t)_state.R[2]);
		_state.SFR.Zero = (value == 0);
		_state.SFR.Sign = (value & 0x8000);
		WriteDestReg(value);
	} else {
		//PLOT
		DrawPixel((uint8_t)_state.R[1], (uint8_t)_state.R[2]);
		_state.R[1]++;
	}
	ResetFlags();
}

void Gsu::ColorCMode()
{
	if(_state.SFR.Alt1) {
		//CMODE - "Set plot mode"
		uint8_t value = (uint8_t)ReadSrcReg();
		_state.PlotTransparent = (value & 0x01) != 0;
		_state.PlotDither = (value & 0x02) != 0;
		_state.ColorHighNibble = (value & 0x04) != 0;
		_state.ColorFreezeHigh = (value & 0x08) != 0;
		_state.ObjMode = (value & 0x10) != 0;
	} else {
		//COLOR - "Set plot color"
		_state.ColorReg = GetColor((uint8_t)ReadSrcReg());
	}
	ResetFlags();
}

uint16_t Gsu::GetTileIndex(uint8_t x, uint8_t y)
{
	switch(_state.ObjMode ? 3 : _state.ScreenHeight) {
		default:
		case 0: return ((x & 0xF8) << 1) + ((y & 0xF8) >> 3); break;
		case 1: return ((x & 0xF8) << 1) + ((x & 0xF8) >> 1) + ((y & 0xF8) >> 3); break;
		case 2: return ((x & 0xF8) << 1) + ((x & 0xF8) << 0) + ((y & 0xF8) >> 3); break;
		case 3: return ((y & 0x80) << 2) + ((x & 0x80) << 1) + ((y & 0x78) << 1) + ((x & 0x78) >> 3); break;
	}
}

uint32_t Gsu::GetTileAddress(uint8_t x, uint8_t y)
{
	uint16_t tileIndex = GetTileIndex(x, y);
	return (0x700000 | (_state.ScreenBase << 10)) + (tileIndex * (_state.PlotBpp << 3)) + ((y & 0x07) * 2);
}

uint8_t Gsu::ReadPixel(uint8_t x, uint8_t y)
{
	WritePixelCache(_state.SecondaryCache);
	WritePixelCache(_state.PrimaryCache);

	uint32_t tileAddress = GetTileAddress(x, y);
	
	x = (x & 7) ^ 7;

	uint8_t data = 0;
	for(int i = 0; i < _state.PlotBpp; i++) {
		//Select which byte to read/write based on the current bit (0/1, 16/17, 32/33, 48/49)
		uint8_t byteOffset = ((i >> 1) << 4) + (i & 0x01);
		data |= ((ReadGsu(tileAddress + byteOffset, MemoryOperationType::Read) >> x) & 1) << i;
		Step(_state.ClockSelect ? 5 : 6);
	}

	return data;
}

bool Gsu::IsTransparentPixel()
{
	uint8_t color = _state.ColorFreezeHigh ? (_state.ColorReg & 0x0F) : _state.ColorReg;

	switch(_state.PlotBpp) {
		default:
		case 2: return (color & 0x03) == 0;
		case 4: return (color & 0x0F) == 0;
		case 8: return color == 0;
	}
}

void Gsu::DrawPixel(uint8_t x, uint8_t y)
{
	if(!_state.PlotTransparent && IsTransparentPixel()) {
		return;
	}

	uint8_t color = _state.ColorReg;
	if(_state.PlotDither && _state.PlotBpp != 8) {
		if((x ^ y) & 0x01) {
			color >>= 4;
		}
		color &= 0x0F;
	}

	if(_state.PrimaryCache.X != (x & 0xF8) || _state.PrimaryCache.Y != y) {
		FlushPrimaryCache(x, y);
	}

	uint8_t xOffset = (x & 7) ^ 7;
	_state.PrimaryCache.Pixels[xOffset] = color;
	_state.PrimaryCache.ValidBits |= (1 << xOffset);
	if(_state.PrimaryCache.ValidBits == 0xFF) {
		FlushPrimaryCache(x, y);
	}
}

void Gsu::FlushPrimaryCache(uint8_t x, uint8_t y)
{
	WritePixelCache(_state.SecondaryCache);
	_state.SecondaryCache = _state.PrimaryCache;
	_state.PrimaryCache.ValidBits = 0;
	_state.PrimaryCache.X = x & 0xF8;
	_state.PrimaryCache.Y = y;
}

void Gsu::WritePixelCache(GsuPixelCache &cache)
{
	if(cache.ValidBits == 0) {
		return;
	}

	uint32_t tileAddress = GetTileAddress(cache.X, cache.Y);

	for(int i = 0; i < _state.PlotBpp; i++) {
		uint8_t value = 0;
		for(int x = 0; x < 8; x++) {
			value |= ((cache.Pixels[x] >> i) & 0x01) << x;
		}

		//Select which byte to read/write based on the current bit (0/1, 16/17, 32/33, 48/49)
		uint8_t byte = ((i >> 1) << 4) + (i & 0x01);

		if(cache.ValidBits != 0xFF) {
			//Read the pixels in memory before to merge them before writing the cache's content to memory
			Step(_state.ClockSelect ? 5 : 6);
			value &= cache.ValidBits;
			value |= ReadGsu(tileAddress + byte, MemoryOperationType::Read) & ~cache.ValidBits;
		}

		Step(_state.ClockSelect ? 5 : 6);
		WaitForRamAccess();
		WriteGsu(tileAddress + byte, value, MemoryOperationType::Write);
	}

	cache.ValidBits = 0;
}

uint8_t Gsu::GetColor(uint8_t value)
{
	if(_state.ColorHighNibble) {
		return (_state.ColorReg & 0xF0) | (value >> 4);
	}
	if(_state.ColorFreezeHigh) {
		return (_state.ColorReg & 0xF0) | (value & 0x0F);
	}

	return value;
}