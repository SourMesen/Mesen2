void SnesCpu::PowerOn()
{
	_state = {};
	_state.PC = GetResetVector();
	_state.SP = 0x1FF;
	_state.PS = ProcFlags::IrqDisable;
	_state.EmulationMode = true;
	_state.NmiFlagCounter = 0;
	_state.StopState = SnesCpuStopState::Running;
	_state.IrqSource = (uint8_t)SnesIrqSource::None;
	_state.PrevIrqSource = (uint8_t)SnesIrqSource::None;
	SetFlags(ProcFlags::MemoryMode8);
	SetFlags(ProcFlags::IndexMode8);
}

void SnesCpu::Reset()
{
	SetFlags(ProcFlags::IrqDisable | ProcFlags::MemoryMode8 | ProcFlags::IndexMode8);
	ClearFlags(ProcFlags::Decimal);
	_state.EmulationMode = true;

	_state.DBR = 0;
	_state.D = 0;
	_state.K = 0;
	_state.Y &= 0xFF;
	_state.X &= 0xFF;
	_state.CycleCount = 0;

	_state.PC = GetResetVector();
	SetSP(_state.SP);

	_state.NmiFlagCounter = 0;
	_state.StopState = SnesCpuStopState::Running;
	_state.IrqSource = (uint8_t)SnesIrqSource::None;
	_state.PrevIrqSource = (uint8_t)SnesIrqSource::None;
}

void SnesCpu::RunOp()
{
	switch(GetOpCode()) {
		case 0x00: AddrMode_Imm8(); BRK(); break;
		case 0x01: AddrMode_DirIdxIndX(); ORA(); break;
		case 0x02: AddrMode_Imm8(); COP(); break;
		case 0x03: AddrMode_StkRel(); ORA(); break;
		case 0x04: AddrMode_Dir(); TSB(); break;
		case 0x05: AddrMode_Dir(); ORA(); break;
		case 0x06: AddrMode_Dir(); ASL(); break;
		case 0x07: AddrMode_DirIndLng(); ORA(); break;
		case 0x08: PHP(); break;
		case 0x09: AddrMode_ImmM(); ORA(); break;
		case 0x0A: AddrMode_Acc(); ASL_Acc(); break;
		case 0x0B: PHD(); break;
		case 0x0C: AddrMode_Abs(); TSB(); break;
		case 0x0D: AddrMode_Abs(); ORA(); break;
		case 0x0E: AddrMode_Abs(); ASL(); break;
		case 0x0F: AddrMode_AbsLng(); ORA(); break;
		case 0x10: AddrMode_Rel(); BPL(); break;
		case 0x11: AddrMode_DirIndIdxY(false); ORA(); break;
		case 0x12: AddrMode_DirInd(); ORA(); break;
		case 0x13: AddrMode_StkRelIndIdxY(); ORA(); break;
		case 0x14: AddrMode_Dir(); TRB(); break;
		case 0x15: AddrMode_DirIdxX(); ORA(); break;
		case 0x16: AddrMode_DirIdxX(); ASL(); break;
		case 0x17: AddrMode_DirIndLngIdxY(); ORA(); break;
		case 0x18: AddrMode_Imp(); CLC(); break;
		case 0x19: AddrMode_AbsIdxY(false); ORA(); break;
		case 0x1A: AddrMode_Acc(); INC_Acc(); break;
		case 0x1B: AddrMode_Imp(); TCS(); break;
		case 0x1C: AddrMode_Abs(); TRB(); break;
		case 0x1D: AddrMode_AbsIdxX(false); ORA(); break;
		case 0x1E: AddrMode_AbsIdxX(true); ASL(); break;
		case 0x1F: AddrMode_AbsLngIdxX(); ORA(); break;
		case 0x20: AddrMode_AbsJmp(); Idle(); JSR(); break;
		case 0x21: AddrMode_DirIdxIndX(); AND(); break;
		case 0x22: JSL(); break;
		case 0x23: AddrMode_StkRel(); AND(); break;
		case 0x24: AddrMode_Dir(); BIT(); break;
		case 0x25: AddrMode_Dir(); AND(); break;
		case 0x26: AddrMode_Dir(); ROL(); break;
		case 0x27: AddrMode_DirIndLng(); AND(); break;
		case 0x28: PLP(); break;
		case 0x29: AddrMode_ImmM(); AND(); break;
		case 0x2A: AddrMode_Acc(); ROL_Acc(); break;
		case 0x2B: PLD(); break;
		case 0x2C: AddrMode_Abs(); BIT(); break;
		case 0x2D: AddrMode_Abs(); AND(); break;
		case 0x2E: AddrMode_Abs(); ROL(); break;
		case 0x2F: AddrMode_AbsLng(); AND(); break;
		case 0x30: AddrMode_Rel(); BMI(); break;
		case 0x31: AddrMode_DirIndIdxY(false); AND(); break;
		case 0x32: AddrMode_DirInd(); AND(); break;
		case 0x33: AddrMode_StkRelIndIdxY(); AND(); break;
		case 0x34: AddrMode_DirIdxX(); BIT(); break;
		case 0x35: AddrMode_DirIdxX(); AND(); break;
		case 0x36: AddrMode_DirIdxX(); ROL(); break;
		case 0x37: AddrMode_DirIndLngIdxY(); AND(); break;
		case 0x38: AddrMode_Imp(); SEC(); break;
		case 0x39: AddrMode_AbsIdxY(false); AND(); break;
		case 0x3A: AddrMode_Acc(); DEC_Acc(); break;
		case 0x3B: AddrMode_Imp(); TSC(); break;
		case 0x3C: AddrMode_AbsIdxX(false); BIT(); break;
		case 0x3D: AddrMode_AbsIdxX(false); AND(); break;
		case 0x3E: AddrMode_AbsIdxX(true); ROL(); break;
		case 0x3F: AddrMode_AbsLngIdxX(); AND(); break;
		case 0x40: RTI(); break;
		case 0x41: AddrMode_DirIdxIndX(); EOR(); break;
		case 0x42: AddrMode_Imm8(); WDM(); break;
		case 0x43: AddrMode_StkRel(); EOR(); break;
		case 0x44: AddrMode_BlkMov(); MVP(); break;
		case 0x45: AddrMode_Dir(); EOR(); break;
		case 0x46: AddrMode_Dir(); LSR(); break;
		case 0x47: AddrMode_DirIndLng(); EOR(); break;
		case 0x48: PHA(); break;
		case 0x49: AddrMode_ImmM(); EOR(); break;
		case 0x4A: AddrMode_Acc(); LSR_Acc(); break;
		case 0x4B: PHK(); break;
		case 0x4C: AddrMode_AbsJmp(); JMP(); break;
		case 0x4D: AddrMode_Abs(); EOR(); break;
		case 0x4E: AddrMode_Abs(); LSR(); break;
		case 0x4F: AddrMode_AbsLng(); EOR(); break;
		case 0x50: AddrMode_Rel(); BVC(); break;
		case 0x51: AddrMode_DirIndIdxY(false); EOR(); break;
		case 0x52: AddrMode_DirInd(); EOR(); break;
		case 0x53: AddrMode_StkRelIndIdxY(); EOR(); break;
		case 0x54: AddrMode_BlkMov(); MVN(); break;
		case 0x55: AddrMode_DirIdxX(); EOR(); break;
		case 0x56: AddrMode_DirIdxX(); LSR(); break;
		case 0x57: AddrMode_DirIndLngIdxY(); EOR(); break;
		case 0x58: AddrMode_Imp(); CLI(); break;
		case 0x59: AddrMode_AbsIdxY(false); EOR(); break;
		case 0x5A: PHY(); break;
		case 0x5B: AddrMode_Imp(); TCD(); break;
		case 0x5C: AddrMode_AbsLngJmp(); JML(); break;
		case 0x5D: AddrMode_AbsIdxX(false); EOR(); break;
		case 0x5E: AddrMode_AbsIdxX(true); LSR(); break;
		case 0x5F: AddrMode_AbsLngIdxX(); EOR(); break;
		case 0x60: RTS(); break;
		case 0x61: AddrMode_DirIdxIndX(); ADC(); break;
		case 0x62: AddrMode_RelLng(); PER(); break;
		case 0x63: AddrMode_StkRel(); ADC(); break;
		case 0x64: AddrMode_Dir(); STZ(); break;
		case 0x65: AddrMode_Dir(); ADC(); break;
		case 0x66: AddrMode_Dir(); ROR(); break;
		case 0x67: AddrMode_DirIndLng(); ADC(); break;
		case 0x68: PLA(); break;
		case 0x69: AddrMode_ImmM(); ADC(); break;
		case 0x6A: AddrMode_Acc(); ROR_Acc(); break;
		case 0x6B: RTL(); break;
		case 0x6C: AddrMode_AbsInd(); JMP(); break;
		case 0x6D: AddrMode_Abs(); ADC(); break;
		case 0x6E: AddrMode_Abs(); ROR(); break;
		case 0x6F: AddrMode_AbsLng(); ADC(); break;
		case 0x70: AddrMode_Rel(); BVS(); break;
		case 0x71: AddrMode_DirIndIdxY(false); ADC(); break;
		case 0x72: AddrMode_DirInd(); ADC(); break;
		case 0x73: AddrMode_StkRelIndIdxY(); ADC(); break;
		case 0x74: AddrMode_DirIdxX(); STZ(); break;
		case 0x75: AddrMode_DirIdxX(); ADC(); break;
		case 0x76: AddrMode_DirIdxX(); ROR(); break;
		case 0x77: AddrMode_DirIndLngIdxY(); ADC(); break;
		case 0x78: AddrMode_Imp(); SEI(); break;
		case 0x79: AddrMode_AbsIdxY(false); ADC(); break;
		case 0x7A: PLY(); break;
		case 0x7B: AddrMode_Imp(); TDC(); break;
		case 0x7C: JMP_AbsIdxXInd(); break;
		case 0x7D: AddrMode_AbsIdxX(false); ADC(); break;
		case 0x7E: AddrMode_AbsIdxX(true); ROR(); break;
		case 0x7F: AddrMode_AbsLngIdxX(); ADC(); break;
		case 0x80: AddrMode_Rel(); BRA(); break;
		case 0x81: AddrMode_DirIdxIndX(); STA(); break;
		case 0x82: AddrMode_RelLng(); BRL(); break;
		case 0x83: AddrMode_StkRel(); STA(); break;
		case 0x84: AddrMode_Dir(); STY(); break;
		case 0x85: AddrMode_Dir(); STA(); break;
		case 0x86: AddrMode_Dir(); STX(); break;
		case 0x87: AddrMode_DirIndLng(); STA(); break;
		case 0x88: AddrMode_Imp(); DEY(); break;
		case 0x89: AddrMode_ImmM(); BIT(); break;
		case 0x8A: AddrMode_Imp(); TXA(); break;
		case 0x8B: PHB(); break;
		case 0x8C: AddrMode_Abs(); STY(); break;
		case 0x8D: AddrMode_Abs(); STA(); break;
		case 0x8E: AddrMode_Abs(); STX(); break;
		case 0x8F: AddrMode_AbsLng(); STA(); break;
		case 0x90: AddrMode_Rel(); BCC(); break;
		case 0x91: AddrMode_DirIndIdxY(true); STA(); break;
		case 0x92: AddrMode_DirInd(); STA(); break;
		case 0x93: AddrMode_StkRelIndIdxY(); STA(); break;
		case 0x94: AddrMode_DirIdxX(); STY(); break;
		case 0x95: AddrMode_DirIdxX(); STA(); break;
		case 0x96: AddrMode_DirIdxY(); STX(); break;
		case 0x97: AddrMode_DirIndLngIdxY(); STA(); break;
		case 0x98: AddrMode_Imp(); TYA(); break;
		case 0x99: AddrMode_AbsIdxY(true); STA(); break;
		case 0x9A: AddrMode_Imp(); TXS(); break;
		case 0x9B: AddrMode_Imp(); TXY(); break;
		case 0x9C: AddrMode_Abs(); STZ(); break;
		case 0x9D: AddrMode_AbsIdxX(true); STA(); break;
		case 0x9E: AddrMode_AbsIdxX(true); STZ(); break;
		case 0x9F: AddrMode_AbsLngIdxX(); STA(); break;
		case 0xA0: AddrMode_ImmX(); LDY(); break;
		case 0xA1: AddrMode_DirIdxIndX(); LDA(); break;
		case 0xA2: AddrMode_ImmX(); LDX(); break;
		case 0xA3: AddrMode_StkRel(); LDA(); break;
		case 0xA4: AddrMode_Dir(); LDY(); break;
		case 0xA5: AddrMode_Dir(); LDA(); break;
		case 0xA6: AddrMode_Dir(); LDX(); break;
		case 0xA7: AddrMode_DirIndLng(); LDA(); break;
		case 0xA8: AddrMode_Imp(); TAY(); break;
		case 0xA9: AddrMode_ImmM(); LDA(); break;
		case 0xAA: AddrMode_Imp(); TAX(); break;
		case 0xAB: PLB(); break;
		case 0xAC: AddrMode_Abs(); LDY(); break;
		case 0xAD: AddrMode_Abs(); LDA(); break;
		case 0xAE: AddrMode_Abs(); LDX(); break;
		case 0xAF: AddrMode_AbsLng(); LDA(); break;
		case 0xB0: AddrMode_Rel(); BCS(); break;
		case 0xB1: AddrMode_DirIndIdxY(false); LDA(); break;
		case 0xB2: AddrMode_DirInd(); LDA(); break;
		case 0xB3: AddrMode_StkRelIndIdxY(); LDA(); break;
		case 0xB4: AddrMode_DirIdxX(); LDY(); break;
		case 0xB5: AddrMode_DirIdxX(); LDA(); break;
		case 0xB6: AddrMode_DirIdxY(); LDX(); break;
		case 0xB7: AddrMode_DirIndLngIdxY(); LDA(); break;
		case 0xB8: AddrMode_Imp(); CLV(); break;
		case 0xB9: AddrMode_AbsIdxY(false); LDA(); break;
		case 0xBA: AddrMode_Imp(); TSX(); break;
		case 0xBB: AddrMode_Imp(); TYX(); break;
		case 0xBC: AddrMode_AbsIdxX(false); LDY(); break;
		case 0xBD: AddrMode_AbsIdxX(false); LDA(); break;
		case 0xBE: AddrMode_AbsIdxY(false); LDX(); break;
		case 0xBF: AddrMode_AbsLngIdxX(); LDA(); break;
		case 0xC0: AddrMode_ImmX(); CPY(); break;
		case 0xC1: AddrMode_DirIdxIndX(); CMP(); break;
		case 0xC2: AddrMode_Imm8(); REP(); break;
		case 0xC3: AddrMode_StkRel(); CMP(); break;
		case 0xC4: AddrMode_Dir(); CPY(); break;
		case 0xC5: AddrMode_Dir(); CMP(); break;
		case 0xC6: AddrMode_Dir(); DEC(); break;
		case 0xC7: AddrMode_DirIndLng(); CMP(); break;
		case 0xC8: AddrMode_Imp(); INY(); break;
		case 0xC9: AddrMode_ImmM(); CMP(); break;
		case 0xCA: AddrMode_Imp(); DEX(); break;
		case 0xCB: WAI(); break;
		case 0xCC: AddrMode_Abs(); CPY(); break;
		case 0xCD: AddrMode_Abs(); CMP(); break;
		case 0xCE: AddrMode_Abs(); DEC(); break;
		case 0xCF: AddrMode_AbsLng(); CMP(); break;
		case 0xD0: AddrMode_Rel(); BNE(); break;
		case 0xD1: AddrMode_DirIndIdxY(false); CMP(); break;
		case 0xD2: AddrMode_DirInd(); CMP(); break;
		case 0xD3: AddrMode_StkRelIndIdxY(); CMP(); break;
		case 0xD4: AddrMode_Dir(); PEI(); break;
		case 0xD5: AddrMode_DirIdxX(); CMP(); break;
		case 0xD6: AddrMode_DirIdxX(); DEC(); break;
		case 0xD7: AddrMode_DirIndLngIdxY(); CMP(); break;
		case 0xD8: AddrMode_Imp(); CLD(); break;
		case 0xD9: AddrMode_AbsIdxY(false); CMP(); break;
		case 0xDA: PHX(); break;
		case 0xDB: AddrMode_Imp(); STP(); break;
		case 0xDC: AddrMode_AbsIndLng(); JML(); break;
		case 0xDD: AddrMode_AbsIdxX(false); CMP(); break;
		case 0xDE: AddrMode_AbsIdxX(true); DEC(); break;
		case 0xDF: AddrMode_AbsLngIdxX(); CMP(); break;
		case 0xE0: AddrMode_ImmX(); CPX(); break;
		case 0xE1: AddrMode_DirIdxIndX(); SBC(); break;
		case 0xE2: AddrMode_Imm8(); SEP(); break;
		case 0xE3: AddrMode_StkRel(); SBC(); break;
		case 0xE4: AddrMode_Dir(); CPX(); break;
		case 0xE5: AddrMode_Dir(); SBC(); break;
		case 0xE6: AddrMode_Dir(); INC(); break;
		case 0xE7: AddrMode_DirIndLng(); SBC(); break;
		case 0xE8: AddrMode_Imp(); INX(); break;
		case 0xE9: AddrMode_ImmM(); SBC(); break;
		case 0xEA: AddrMode_Imp(); NOP(); break;
		case 0xEB: AddrMode_Imp(); XBA(); break;
		case 0xEC: AddrMode_Abs(); CPX(); break;
		case 0xED: AddrMode_Abs(); SBC(); break;
		case 0xEE: AddrMode_Abs(); INC(); break;
		case 0xEF: AddrMode_AbsLng(); SBC(); break;
		case 0xF0: AddrMode_Rel(); BEQ(); break;
		case 0xF1: AddrMode_DirIndIdxY(false); SBC(); break;
		case 0xF2: AddrMode_DirInd(); SBC(); break;
		case 0xF3: AddrMode_StkRelIndIdxY(); SBC(); break;
		case 0xF4: AddrMode_Imm16(); PEA(); break;
		case 0xF5: AddrMode_DirIdxX(); SBC(); break;
		case 0xF6: AddrMode_DirIdxX(); INC(); break;
		case 0xF7: AddrMode_DirIndLngIdxY(); SBC(); break;
		case 0xF8: AddrMode_Imp(); SED(); break;
		case 0xF9: AddrMode_AbsIdxY(false); SBC(); break;
		case 0xFA: PLX(); break;
		case 0xFB: AddrMode_Imp(); XCE(); break;
		case 0xFC: JSR_AbsIdxXInd(); break;
		case 0xFD: AddrMode_AbsIdxX(false); SBC(); break;
		case 0xFE: AddrMode_AbsIdxX(true); INC(); break;
		case 0xFF: AddrMode_AbsLngIdxX(); SBC(); break;
	}
}

SnesCpuState& SnesCpu::GetState()
{
	return _state;
}

uint64_t SnesCpu::GetCycleCount()
{
	return _state.CycleCount;
}

void SnesCpu::SetNmiFlag(uint8_t delay)
{
	_state.NmiFlagCounter = delay;
}

void SnesCpu::DetectNmiSignalEdge()
{
	//"This edge detector polls the status of the NMI line during φ2 of each CPU cycle (i.e., during the 
	//second half of each cycle) and raises an internal signal if the input goes from being high during 
	//one cycle to being low during the next"
	if(_state.NmiFlagCounter) {
		_state.NmiFlagCounter--;
		if(_state.NmiFlagCounter == 0) {
			if(!_state.IrqLock) {
				//Trigger an NMI on the next instruction
				_state.NmiFlagCounter = false;
				_state.NeedNmi = true;
				_waiOver = true;
			} else {
				//Delay NMI if a DMA/HDMA just finished running
				_state.NmiFlagCounter = 1;
				_state.NeedNmi = false;
			}
		}
	}
	
	if(!_state.IrqLock) {
		_state.PrevIrqSource = _state.IrqSource != 0 && !CheckFlag(ProcFlags::IrqDisable);
		if(_state.IrqSource != 0) {
			//WAI ends even if the I flag is set (but execution won't jump to the IRQ handler)
			_waiOver = true;
		}
	} else {
		//Delay IRQ if a DMA/HDMA just finished running
		_state.PrevIrqSource = 0;
	}

	_state.IrqLock = false;
}

void SnesCpu::SetIrqSource(SnesIrqSource source)
{
	_state.IrqSource |= (uint8_t)source;
}

bool SnesCpu::CheckIrqSource(SnesIrqSource source)
{
	if(_state.IrqSource & (uint8_t)source) {
		return true;
	} else {
		return false;
	}
}

void SnesCpu::ClearIrqSource(SnesIrqSource source)
{
	_state.IrqSource &= ~(uint8_t)source;
}

uint32_t SnesCpu::GetProgramAddress(uint16_t addr)
{
	return (_state.K << 16) | addr;
}

uint32_t SnesCpu::GetDataAddress(uint16_t addr)
{
	return (_state.DBR << 16) | addr;
}

uint8_t SnesCpu::GetOpCode()
{
	uint8_t opCode = ReadCode(_state.PC, MemoryOperationType::ExecOpCode);
	_state.PC++;
	return opCode;
}

void SnesCpu::IdleOrRead()
{
	if(!_state.IrqLock && ((_state.IrqSource != 0 || _state.PrevIrqSource != 0) && !CheckFlag(ProcFlags::IrqDisable)) || (_state.NmiFlagCounter == 1 || _state.NeedNmi)) {
		//If an IRQ or NMI will be triggered on the next instruction/cycle, the 6-clock idle cycle turns into a dummy read at the current PC
		ReadCode(_state.PC);
	} else {
		Idle();
	}
}

uint8_t SnesCpu::ReadOperandByte()
{
	uint8_t value = ReadCode(_state.PC, MemoryOperationType::ExecOperand);
	_state.PC++;
	return value;
}

uint16_t SnesCpu::ReadOperandWord()
{
	uint8_t lsb = ReadOperandByte();
	uint8_t msb = ReadOperandByte();
	return (msb << 8) | lsb;
}

uint32_t SnesCpu::ReadOperandLong()
{
	uint8_t b1 = ReadOperandByte();
	uint8_t b2 = ReadOperandByte();
	uint8_t b3 = ReadOperandByte();
	return (b3 << 16) | (b2 << 8) | b1;
}

uint8_t SnesCpu::ReadCode(uint16_t addr, MemoryOperationType type)
{
	return Read((_state.K << 16) | addr, type);
}

uint16_t SnesCpu::ReadCodeWord(uint16_t addr, MemoryOperationType type)
{
	uint8_t lsb = ReadCode(addr);
	uint8_t msb = ReadCode(addr + 1);
	return (msb << 8) | lsb;
}

uint8_t SnesCpu::ReadData(uint32_t addr, MemoryOperationType type)
{
	return Read(addr & _readWriteMask, type);
}

uint16_t SnesCpu::ReadDataWord(uint32_t addr, MemoryOperationType type)
{
	uint8_t lsb = ReadData(addr);
	uint8_t msb = ReadData(addr + 1);
	return (msb << 8) | lsb;
}

uint32_t SnesCpu::ReadDataLong(uint32_t addr, MemoryOperationType type)
{
	uint8_t b1 = ReadData(addr);
	uint8_t b2 = ReadData(addr + 1);
	uint8_t b3 = ReadData(addr + 2);
	return (b3 << 16) | (b2 << 8) | b1;
}

void SnesCpu::WriteWord(uint32_t addr, uint16_t value, MemoryOperationType type)
{
	Write(addr, (uint8_t)value);
	Write((addr + 1) & _readWriteMask, (uint8_t)(value >> 8));
}

void SnesCpu::WriteWordRmw(uint32_t addr, uint16_t value, MemoryOperationType type)
{
	//Read-modify-write instructions write the MSB first
	Write((addr + 1) & _readWriteMask, (uint8_t)(value >> 8));
	Write(addr, (uint8_t)value);
}

uint8_t SnesCpu::GetByteValue()
{
	if(_immediateMode) {
		return (uint8_t)_operand;
	} else {
		return ReadData(_operand);
	}
}

uint16_t SnesCpu::GetWordValue()
{
	if(_immediateMode) {
		return (uint16_t)_operand;
	} else {
		return ReadDataWord(_operand);
	}
}

void SnesCpu::PushByte(uint8_t value, bool allowEmulationMode)
{
	Write(_state.SP, value);
	SetSP(_state.SP - 1, allowEmulationMode);
}

uint8_t SnesCpu::PopByte(bool allowEmulationMode)
{
	SetSP(_state.SP + 1, allowEmulationMode);
	return ReadData(_state.SP);
}

void SnesCpu::PushWord(uint16_t value, bool allowEmulationMode)
{
	PushByte(value >> 8, allowEmulationMode);
	PushByte((uint8_t)value, allowEmulationMode);
}

uint16_t SnesCpu::PopWord(bool allowEmulationMode)
{
	uint8_t lo = PopByte(allowEmulationMode);
	uint8_t hi = PopByte(allowEmulationMode);
	return lo | hi << 8;
}

uint16_t SnesCpu::GetDirectAddress(uint16_t offset, bool allowEmulationMode)
{
	if(allowEmulationMode && _state.EmulationMode && (_state.D & 0xFF) == 0) {
		return (uint16_t)((_state.D & 0xFF00) | (offset & 0xFF));
	} else {
		return (uint16_t)(_state.D + offset);
	}
}

uint16_t SnesCpu::GetDirectAddressIndirectWordWithPageWrap(uint16_t offset)
{
	uint8_t lsb = ReadData(GetDirectAddress(offset + 0));
	if(!_state.EmulationMode || (_state.D & 0xFF) == 0) {
		return (ReadData(GetDirectAddress(offset + 1)) << 8) | lsb;
	} else {
		uint16_t addr = GetDirectAddress(offset + 1);
		uint8_t msb;
		if((addr & 0xFF) == 0) {
			//Crossed to next page, wrap around to start of previous page instead (cpu bug?)
			msb = ReadData((uint16_t)(addr - 0x100));
		} else {
			msb = ReadData(addr);
		}
		return (msb << 8) | lsb;
	}
}

uint16_t SnesCpu::GetDirectAddressIndirectWord(uint16_t offset)
{
	uint8_t lsb = ReadData(GetDirectAddress(offset + 0));
	uint8_t msb = ReadData(GetDirectAddress(offset + 1));
	return (msb << 8) | lsb;
}

uint32_t SnesCpu::GetDirectAddressIndirectLong(uint16_t offset)
{
	uint8_t b1 = ReadData(GetDirectAddress(offset + 0, false));
	uint8_t b2 = ReadData(GetDirectAddress(offset + 1, false));
	uint8_t b3 = ReadData(GetDirectAddress(offset + 2, false));
	return (b3 << 16) | (b2 << 8) | b1;
}

void SnesCpu::SetSP(uint16_t sp, bool allowEmulationMode)
{
	if(allowEmulationMode && _state.EmulationMode) {
		_state.SP = 0x100 | (sp & 0xFF);
	} else {
		_state.SP = sp;
	}
}

void SnesCpu::RestrictStackPointerValue()
{
	if(_state.EmulationMode) {
		_state.SP = 0x100 | (_state.SP & 0xFF);
	}
}

void SnesCpu::SetPS(uint8_t ps)
{
	_state.PS = ps;
	if(CheckFlag(ProcFlags::IndexMode8)) {
		//Truncate X/Y when 8-bit indexes are enabled
		_state.Y &= 0xFF;
		_state.X &= 0xFF;
	}
}

void SnesCpu::SetRegister(uint8_t& reg, uint8_t value)
{
	SetZeroNegativeFlags(value);
	reg = value;
}

void SnesCpu::SetRegister(uint16_t& reg, uint16_t value, bool eightBitMode)
{
	if(eightBitMode) {
		SetZeroNegativeFlags((uint8_t)value);
		reg = (reg & 0xFF00) | (uint8_t)value;
	} else {
		SetZeroNegativeFlags(value);
		reg = value;
	}
}

void SnesCpu::SetZeroNegativeFlags(uint16_t value)
{
	ClearFlags(ProcFlags::Zero | ProcFlags::Negative);
	if(value == 0) {
		SetFlags(ProcFlags::Zero);
	} else if(value & 0x8000) {
		SetFlags(ProcFlags::Negative);
	}
}

void SnesCpu::SetZeroNegativeFlags(uint8_t value)
{
	ClearFlags(ProcFlags::Zero | ProcFlags::Negative);
	if(value == 0) {
		SetFlags(ProcFlags::Zero);
	} else if(value & 0x80) {
		SetFlags(ProcFlags::Negative);
	}
}

void SnesCpu::ClearFlags(uint8_t flags)
{
	_state.PS &= ~flags;
}

void SnesCpu::SetFlags(uint8_t flags)
{
	_state.PS |= flags;
}

bool SnesCpu::CheckFlag(uint8_t flag)
{
	return (_state.PS & flag) == flag;
}

void SnesCpu::Serialize(Serializer &s)
{
	SV(_state.A); SV(_state.CycleCount); SV(_state.D); SV(_state.DBR); SV(_state.EmulationMode); SV(_state.IrqSource); SV(_state.K);
	SV(_state.NmiFlagCounter); SV(_state.PC); SV(_state.PrevIrqSource); SV(_state.PS); SV(_state.SP); SV(_state.StopState);
	SV(_state.X); SV(_state.Y); SV(_state.IrqLock); SV(_state.NeedNmi); SV(_waiOver);
}
