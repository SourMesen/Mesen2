#include "pch.h"
#include "WS/WsCpu.h"
#include "WS/WsMemoryManager.h"
#include "Shared/Emulator.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/Serializer.h"

WsCpuParityTable WsCpu::_parity = {};

WsCpu::WsCpu(Emulator* emu, WsMemoryManager* memoryManager)
#ifndef DUMMYCPU
	: _prefetch(this, memoryManager)
#endif
{
	_emu = emu;
	_memoryManager = memoryManager;

	_state.CS = 0xFFFF;
	_state.IP = 0x0000;

	_state.BX = 0x1C00;
	_state.CX = 0x0004;
	_state.SP = 0x2000;
	_state.DI = 0x6FFF;

	_state.Flags.Mode = true;

	ClearPrefetch();
}

uint32_t WsCpu::GetProgramCounter(bool adjustForRepLoop)
{
	uint16_t ip = _state.IP;
	if(adjustForRepLoop && _prefix.Preserve && _prefix.Rep != WsRepMode::None) {
		ip -= _prefix.PrefixCount;
	}
	return ((_state.CS << 4) + ip) & 0xFFFFF;
}

void WsCpu::Exec()
{
#ifndef DUMMYCPU
	bool irqPending = _memoryManager->HasPendingIrq();
	if(_state.Halted && !irqPending) {
		Idle();
		_emu->ProcessHaltedCpu<CpuType::Ws>();
		return;
	}

	if(irqPending) {
		_state.Halted = false;
		if(_state.Flags.Irq) {
			Interrupt(_memoryManager->GetIrqVector(), true);
		}
	}

	_emu->ProcessInstruction<CpuType::Ws>();
#endif

	ExecOpCode();
}

template<typename T>
T WsCpu::ReadMemory(uint16_t seg, uint16_t offset)
{
#ifndef DUMMYCPU
	return _memoryManager->Read<T>(seg, offset);
#else 
	T value = _memoryManager->DebugCpuRead<T>(seg, offset);
	LogMemoryOperation((seg << 4) + offset, value, MemoryOperationType::Read, MemoryType::WsMemory, std::is_same<T, uint16_t>::value);
	return value;
#endif
}

template<typename T>
void WsCpu::WriteMemory(uint16_t seg, uint16_t offset, T value)
{
#ifndef DUMMYCPU
	_memoryManager->Write<T>(seg, offset, value);
#else 
	LogMemoryOperation((seg << 4) + offset, value, MemoryOperationType::Write, MemoryType::WsMemory, std::is_same<T, uint16_t>::value);
#endif
}

template<typename T>
T WsCpu::ReadPort(uint16_t port)
{
#ifndef DUMMYCPU
	return _memoryManager->ReadPort<T>(port);
#else
	T value = _memoryManager->DebugReadPort<T>(port);
	LogMemoryOperation(port, value, MemoryOperationType::Read, MemoryType::WsPort, std::is_same<T, uint16_t>::value);
	return value;
#endif
}

template<typename T>
void WsCpu::WritePort(uint16_t port, T value)
{
#ifndef DUMMYCPU
	_memoryManager->WritePort<T>(port, value);
#else
	LogMemoryOperation(port, value, MemoryOperationType::Write, MemoryType::WsPort, std::is_same<T, uint16_t>::value);
#endif
}

void WsCpu::ProcessCpuCycle()
{
#ifndef DUMMYCPU
	_memoryManager->Exec();
#endif
}

uint8_t WsCpu::ReadCodeByte(bool forOpCode)
{
#ifndef DUMMYCPU
	uint8_t value = _prefetch.Read();
	_emu->ProcessMemoryRead<CpuType::Ws>((_state.CS << 4) + _state.IP, value, forOpCode ? MemoryOperationType::ExecOpCode : MemoryOperationType::ExecOperand);
	_state.IP++;
	return value;
#else
	uint32_t addr = (_state.CS << 4) + _state.IP;
	uint32_t value = _memoryManager->DebugRead(addr);
	_state.IP++;
	LogMemoryOperation(addr, value, MemoryOperationType::ExecOperand, MemoryType::WsMemory, false);
	return value;
#endif
}

uint16_t WsCpu::ReadCodeWord()
{
	uint8_t lo = ReadCodeByte();
	uint8_t hi = ReadCodeByte();
	return (hi << 8) | lo;
}

template<typename T>
T WsCpu::ReadImmediate()
{
	return std::is_same<T, uint16_t>::value ? ReadCodeWord() : ReadCodeByte();
}

template<uint8_t cycles>
void WsCpu::Idle()
{
#ifndef DUMMYCPU
	if constexpr(cycles >= 1) {
		_prefetch.Prefetch();
	}

	if constexpr(cycles > 1) {
		Idle<cycles - 1>();
	}
#endif
}

template<typename T>
constexpr uint32_t WsCpu::GetMaxValue()
{
	return (1 << GetBitCount<T>()) - 1;
}

template<typename T>
constexpr uint32_t WsCpu::GetBitCount()
{
	return sizeof(T) * 8;
}

template<typename T>
constexpr uint32_t WsCpu::GetSign()
{
	return 1 << (GetBitCount<T>() - 1);
}

void WsCpu::Move(uint16_t& dst, uint16_t src)
{
	Idle();
	dst = src;
}

void WsCpu::MoveLo(uint16_t& dst, uint8_t src)
{
	Idle();
	dst = (dst & 0xFF00) | src;
}

void WsCpu::MoveHi(uint16_t& dst, uint8_t src)
{
	Idle();
	dst = (dst & 0xFF) | (src << 8);
}

void WsCpu::Push(uint16_t value)
{
	_state.SP -= 2;
	WriteMemory<uint16_t>(_state.SS, _state.SP, value);
}

void WsCpu::PushSP()
{
	_state.SP -= 2;
	WriteMemory<uint16_t>(_state.SS, _state.SP, _state.SP);
}

void WsCpu::Pop(uint16_t& dst)
{
	uint16_t value = ReadMemory<uint16_t>(_state.SS, _state.SP);
	_state.SP += 2;
	dst = value;
}

void WsCpu::PushSegment(uint16_t value)
{
	Idle<1>();
	Push(value);
}

void WsCpu::PopSegment(uint16_t& dst)
{
	Idle<2>();
	Pop(dst);
}

void WsCpu::PushFlags()
{
	Idle();
	Push(_state.Flags.Get());
}

void WsCpu::PopFlags()
{
	Idle<2>();
	uint16_t flags;
	Pop(flags);
	_state.Flags.Set(flags);
}

void WsCpu::PopMemory()
{
	Idle();
	ReadModRmByte();
	uint16_t value;
	Pop(value);
	SetModRm<uint16_t>(value);
}

void WsCpu::PopAll()
{
	Idle();
	Pop(_state.DI);
	Pop(_state.SI);
	Pop(_state.BP);
	uint16_t sp;
	Pop(sp); //don't update SP
	Pop(_state.BX);
	Pop(_state.DX);
	Pop(_state.CX);
	Pop(_state.AX);
}

void WsCpu::PushAll()
{
	Idle();
	uint16_t sp = _state.SP;
	Push(_state.AX);
	Push(_state.CX);
	Push(_state.DX);
	Push(_state.BX);
	Push(sp);
	Push(_state.BP);
	Push(_state.SI);
	Push(_state.DI);
}

uint16_t WsCpu::GetSegment(WsSegment defaultSegment)
{
	switch(_prefix.Segment == WsSegment::Default ? defaultSegment : _prefix.Segment) {
		default:
		case WsSegment::ES: return _state.ES;
		case WsSegment::SS: return _state.SS;
		case WsSegment::CS: return _state.CS;
		case WsSegment::DS: return _state.DS;
	}
}

void WsCpu::ReadModRmByte()
{
	uint8_t modRm = ReadCodeByte();
	_modRm.Mode = (modRm >> 6) & 0x03;
	_modRm.Register = (modRm >> 3) & 0x07;
	_modRm.Rm = modRm & 0x07;

	uint16_t offset = 0;
	uint16_t seg = GetSegment(WsSegment::DS);

	if(_modRm.Mode == 0 && _modRm.Rm == 0x06) {
		offset = ReadCodeWord();
	} else if(_modRm.Mode != 3) {
		switch(_modRm.Rm) {
			case 0x00: Idle(); offset = _state.BX + _state.SI; break;
			case 0x01: Idle(); offset = _state.BX + _state.DI; break;
			case 0x02: Idle(); offset = _state.BP + _state.SI; seg = GetSegment(WsSegment::SS); break;
			case 0x03: Idle(); offset = _state.BP + _state.DI; seg = GetSegment(WsSegment::SS); break;
			case 0x04: offset = _state.SI; break;
			case 0x05: offset = _state.DI; break;
			case 0x06: offset = _state.BP; seg = GetSegment(WsSegment::SS); break;
			case 0x07: offset = _state.BX; break;
		}

		if(_modRm.Mode == 0x01) {
			offset += (int16_t)((int8_t)ReadCodeByte());
		}
		else if(_modRm.Mode == 0x02) {
			offset += (int16_t)ReadCodeWord();
		}
	}

	_modRm.Segment = seg;
	_modRm.Offset = offset;
}

uint16_t WsCpu::GetModSegRegister(uint8_t reg)
{
	return *_modSegLut16[reg & 0x03];
}

void WsCpu::SetModSegRegister(uint8_t reg, uint16_t value)
{
	*_modSegLut16[reg & 0x03] = value;
}

void WsCpu::Exchange(uint16_t& x, uint16_t& y)
{
	Idle<3>();
	std::swap(x, y);
}

void WsCpu::ProcessInvalidDiv()
{
	Interrupt(0);
}

void WsCpu::JumpFar()
{
	Idle<6>();
	uint16_t offset = ReadCodeWord();
	uint16_t segment = ReadCodeWord();
	_state.CS = segment;
	_state.IP = offset;
	ClearPrefetch();
}

void WsCpu::Jump(bool condition)
{
#ifdef DUMMYCPU
	condition = true;
#endif

	Idle();
	int8_t offset = ReadCodeByte();
	if(condition) {
		Idle<2>();
		_state.IP += offset;
		ClearPrefetch();
	}
}

void WsCpu::JumpNearWord()
{
	Idle<3>();
	int16_t disp = (int16_t)ReadCodeWord();
	_state.IP += disp;
	ClearPrefetch();
}

void WsCpu::Call(uint16_t offset)
{
	Push(_state.IP);
	_state.IP = offset;
	ClearPrefetch();
}

void WsCpu::CallNearWord()
{
	Idle<2>();
	int16_t disp = (int16_t)ReadCodeWord();
	Push(_state.IP);
	_state.IP += disp;
	ClearPrefetch();
}

void WsCpu::CallFar()
{
	uint16_t offset = ReadCodeWord();
	uint16_t segment = ReadCodeWord();
	CallFar(segment, offset);
}

void WsCpu::CallFar(uint16_t segment, uint16_t offset)
{
	Idle<6>();
	Push(_state.CS);
	Push(_state.IP);
	_state.CS = segment;
	_state.IP = offset;
	ClearPrefetch();
}

void WsCpu::Loop()
{
	Idle();
	int8_t offset = (int8_t)ReadCodeByte();
	_state.CX--;
	if(_state.CX) {
		Idle<3>();
		_state.IP += offset;
		ClearPrefetch();
	}
}

void WsCpu::LoopIf(bool condition)
{
#ifdef DUMMYCPU
	condition = true;
#endif

	Idle<2>();
	int8_t offset = (int8_t)ReadCodeByte();
	_state.CX--;
	if(condition && _state.CX) {
		Idle<3>();
		_state.IP += offset;
		ClearPrefetch();
	}
}

void WsCpu::Ret()
{
	Idle<4>();
	Pop(_state.IP);
	ClearPrefetch();
}

void WsCpu::RetImm()
{
	Idle<4>();
	uint16_t spOffset = ReadCodeWord();
	Pop(_state.IP);
	_state.SP += spOffset;
	ClearPrefetch();
}

void WsCpu::RetFar()
{
	Idle<5>();
	Pop(_state.IP);
	Pop(_state.CS);
	ClearPrefetch();
}

void WsCpu::RetFarImm()
{
	Idle<6>();
	uint16_t spOffset = ReadCodeWord();
	Pop(_state.IP);
	Pop(_state.CS);
	_state.SP += spOffset;
	ClearPrefetch();
}

void WsCpu::RetInterrupt()
{
	Idle<6>();
	Pop(_state.IP);
	Pop(_state.CS);
	uint16_t flags;
	Pop(flags);
	_state.Flags.Set(flags);
	ClearPrefetch();
}

void WsCpu::Interrupt(uint8_t vector, bool pushFirstPrefix)
{
	Idle<30>();

	uint16_t flags = _state.Flags.Get();
	Push(flags);
	Push(_state.CS);

	if(pushFirstPrefix && _prefix.PrefixCount) {
		_state.IP -= _prefix.PrefixCount;
	}
	_prefix = {};

	Push(_state.IP);

#ifndef DUMMYCPU
	uint32_t originalPc = GetProgramCounter();
#endif

	uint16_t offset = ReadMemory<uint16_t>(0, vector << 2);
	uint16_t segment = ReadMemory<uint16_t>(0, (vector << 2) + 2);

	_state.Flags.Irq = false;
	_state.Flags.Trap = false;
	_state.Flags.Mode = true;
	_state.CS = segment;
	_state.IP = offset;
	ClearPrefetch();

#ifndef DUMMYCPU
	_emu->ProcessInterrupt<CpuType::Ws>(originalPc, GetProgramCounter(), false);
#endif
}

void WsCpu::InterruptOverflow()
{
	Idle<6>();
	if(!_state.Flags.Overflow) {
		return;
	}
	Idle<3>();
	Interrupt(4);
}

void WsCpu::Enter()
{
	Idle<7>();

	uint16_t spOffset = ReadImmediate<uint16_t>();
	uint8_t pushCount = ReadImmediate<uint8_t>() & 0x1F;
	Push(_state.BP);
	uint16_t sp = _state.SP;
	if(pushCount > 0) {
		for(int i = 0; i < pushCount - 1; i++) {
			Idle<2>();
			_state.BP -= 2;
			Push(ReadMemory<uint16_t>(GetSegment(WsSegment::SS), _state.BP));
		}
		Push(sp);
	}
	_state.BP = sp;
	_state.SP -= spOffset;
}

void WsCpu::Leave()
{
	Idle<2>();
	_state.SP = _state.BP;
	Pop(_state.BP);
}

void WsCpu::NOP()
{
	Idle();
}

void WsCpu::FP01()
{
	//2-byte nop (floating point unit on original V series)
	Idle();
	ReadCodeByte();
}

void WsCpu::BOUND()
{
	Idle<12>();

	ReadModRmByte();
	int16_t min = (int16_t)GetModRm<uint16_t>();
	_modRm.Offset += 2;
	int16_t max = (int16_t)GetModRm<uint16_t>();
	int16_t index = (int16_t)GetModRegister<uint16_t>(_modRm.Register);
	if(index < min || index > max) {
		Idle<3>();
		Interrupt(5);
	}
}

void WsCpu::SALC()
{
	Idle<8>();
	_state.AX = (_state.AX & 0xFF00) | (_state.Flags.Carry ? 0xFF : 0);
}

void WsCpu::CBW()
{
	Idle();
	if(_state.AX & 0x80) {
		_state.AX |= 0xFF00;
	} else {
		_state.AX &= 0xFF;
	}
}

void WsCpu::CWD()
{
	Idle();
	if(_state.AX & 0x8000) {
		_state.DX = 0xFFFF;
	} else {
		_state.DX = 0;
	}
}

void WsCpu::LAHF()
{
	Idle<2>();
	_state.AX = (_state.AX & 0xFF) | ((_state.Flags.Get() & 0xFF) << 8);
}

void WsCpu::SAHF()
{
	Idle<4>();
	_state.Flags.Set((_state.Flags.Get() & 0xFF00) | (_state.AX >> 8));
}

void WsCpu::LdsLesLeaModRm()
{
	ReadModRmByte();
	if(_modRm.Mode == 3) {
		//These instructions change the behavior of mode 3 (reg)
		_modRm.Mode = 0;
		uint16_t seg = GetSegment(WsSegment::DS);
		uint16_t offset;
		switch(_modRm.Rm) {
			default:
			case 0x00: offset = _state.BX + _state.AX; break;
			case 0x01: offset = _state.BX + _state.CX; break;
			case 0x02: offset = _state.BP + _state.DX; seg = GetSegment(WsSegment::SS); break;
			case 0x03: offset = _state.BP + _state.BX; seg = GetSegment(WsSegment::SS); break;
			case 0x04: offset = _state.SP + _state.SI; break;
			case 0x05: offset = _state.BP + _state.DI; break;
			case 0x06: offset = _state.BP + _state.SI; seg = GetSegment(WsSegment::SS); break;
			case 0x07: offset = _state.BX + _state.DI; break;
		}
		_modRm.Segment = seg;
		_modRm.Offset = offset;
	}
}

uint16_t WsCpu::LoadSegment()
{
	Idle<4>();
	LdsLesLeaModRm();

	uint16_t value = GetModRm<uint16_t>();
	SetModRegister(_modRm.Register, value);

	_modRm.Offset += 2;
	return GetModRm<uint16_t>();
}

void WsCpu::LDS()
{
	_state.DS = LoadSegment();
}

void WsCpu::LES()
{
	_state.ES = LoadSegment();
}

void WsCpu::LEA()
{
	Idle();
	LdsLesLeaModRm();
	SetModRegister(_modRm.Register, _modRm.Offset);
}

void WsCpu::XLAT()
{
	Idle<4>();
	_state.AX = (_state.AX & 0xFF00) | ReadMemory<uint8_t>(GetSegment(WsSegment::DS), _state.BX + (_state.AX & 0xFF));
}

void WsCpu::AdjustAscii(bool forSub)
{
	Idle<9>();
	uint8_t al = _state.AX;
	uint8_t ah = _state.AX >> 8;

	_state.Flags.Overflow = false;
	_state.Flags.Parity = true;

	if((al & 0x0F) > 9 || _state.Flags.AuxCarry) {
		al += forSub ? -6 : 6;
		ah += forSub ? -1 : 1;
		_state.Flags.AuxCarry = true;
	} else {
		_state.Flags.AuxCarry = false;
	}

	_state.Flags.Zero = _state.Flags.AuxCarry;
	_state.Flags.Carry = _state.Flags.AuxCarry;
	_state.Flags.Sign = !_state.Flags.AuxCarry;

	al &= 0x0F;

	_state.AX = al | (ah << 8);
}

void WsCpu::AAA()
{
	AdjustAscii(false);
}

void WsCpu::AAS()
{
	AdjustAscii(true);
}

void WsCpu::AAD()
{
	Idle<6>();
	uint8_t imm = ReadCodeByte();

	uint8_t ah = _state.AX >> 8;
	uint8_t al = (uint8_t)_state.AX;
	_state.AX = Add<uint8_t>((ah * imm), al, 0);
}

void WsCpu::AAM()
{
	Idle<12>();
	uint8_t imm = ReadCodeByte();
	if(imm == 0) {
		_state.Flags.AuxCarry = false;
		_state.Flags.Parity = false;
		_state.Flags.Sign = false;
		_state.Flags.Zero = (_state.AX & 0xFF) <= 0x3F;
		_state.Flags.Carry = _mulOverflow;
		_state.Flags.Overflow = _mulOverflow;
		ProcessInvalidDiv();
		return;
	}
	Idle<4>();

	_state.Flags.AuxCarry = false;
	_state.Flags.Carry = false;
	_state.Flags.Overflow = false;

	uint8_t al = (uint8_t)_state.AX;
	uint8_t ah = al / imm;
	al = al % imm;

	_state.AX = (ah << 8) | al;
	_state.Flags.Parity = _parity.CheckParity(al);
	_state.Flags.Zero = al == 0;
	_state.Flags.Sign = al & 0x80;
}

void WsCpu::AdjustDecimal(bool forSub)
{
	uint8_t al = _state.AX;
	uint8_t ah = _state.AX >> 8;

	_state.Flags.Overflow = false;

	if((al & 0x0F) > 9 || _state.Flags.AuxCarry) {
		uint8_t prevAl = al;
		al += forSub ? -6 : 6;
		_state.Flags.AuxCarry = true;
		_state.Flags.Overflow = (al ^ prevAl) & ((forSub ? prevAl : al) ^ 0x06) & 0x80;
	} else {
		_state.Flags.AuxCarry = false;
	}

	if((_state.AX & 0xFF) > 0x99 || _state.Flags.Carry) {
		uint8_t prevAl = al;
		al += forSub ? -0x60 : 0x60;
		_state.Flags.Carry = true;
		_state.Flags.Overflow |= (bool)((al ^ prevAl) & ((forSub ? prevAl : al) ^ 0x60) & 0x80);
	} else {
		_state.Flags.Carry = false;
	}

	_state.AX = al | (ah << 8);
	_state.Flags.Sign = al & 0x80;
	_state.Flags.Zero = al == 0;
	_state.Flags.Parity = _parity.CheckParity(al);
}

void WsCpu::DAA()
{
	Idle<10>();
	AdjustDecimal(false);
}

void WsCpu::DAS()
{
	Idle<11>();
	AdjustDecimal(true);
}

void WsCpu::Undefined()
{
	Idle();
}

void WsCpu::Wait()
{
	Idle<10>();
}

void WsCpu::Halt()
{
	Idle<12>();
	_state.Halted = true;
}

void WsCpu::SetFlagValue(bool& flag, bool value)
{
	Idle<4>();
	flag = value;
}

void WsCpu::ClearPrefetch()
{
#ifndef DUMMYCPU
	_prefetch.Clear(_state.CS, _state.IP);
#endif
}

template<>
uint8_t WsCpu::GetModRegister(uint8_t reg)
{
	uint16_t* regValue = _modRegLut8[reg & 0x03];
	if(reg & 0x04) {
		return (*regValue) >> 8;
	}
	return *regValue;
}

template<>
uint16_t WsCpu::GetModRegister<uint16_t>(uint8_t reg)
{
	return *_modRegLut16[reg];
}

template<>
void WsCpu::SetModRegister(uint8_t reg, uint8_t value)
{
	uint16_t& regValue = *_modRegLut8[reg & 0x03];
	if(reg & 0x04) {
		regValue = (regValue & 0xFF) | (value << 8);
	} else {
		regValue = (regValue & 0xFF00) | value;
	}
}

template<>
void WsCpu::SetModRegister(uint8_t reg, uint16_t value)
{
	*_modRegLut16[reg] = value;
}

template<typename T>
T WsCpu::GetModRm()
{
	if(_modRm.Mode == 0x03) {
		return GetModRegister<T>(_modRm.Rm);
	}

	return ReadMemory<T>(_modRm.Segment, _modRm.Offset);
}

template<typename T>
void WsCpu::SetModRm(T value)
{
	if(_modRm.Mode == 0x03) {
		SetModRegister(_modRm.Rm, value);
	} else {
		WriteMemory<T>(_modRm.Segment, _modRm.Offset, value);
	}
}

template<bool sign, typename T>
void WsCpu::Grp1ModRm()
{
	Idle();

	ReadModRmByte();

	T param1 = GetModRm<T>();
	uint16_t param2 = std::is_same<T, uint16_t>::value && !sign ? ReadCodeWord() : ReadCodeByte();
	if constexpr(std::is_same<T, uint16_t>::value && sign) {
		param2 = (int16_t)(int8_t)param2;
	}

	T result = GetAluResult<T>((AluOp)_modRm.Register, param1, param2);
	if((AluOp)_modRm.Register != AluOp::Cmp) {
		SetModRm(result);
	}
}

template<typename T, WsCpu::Grp2Mode mode>
void WsCpu::Grp2ModRm()
{
	ReadModRmByte();

	T param = GetModRm<T>();
	uint8_t shift;
	switch(mode) {
		case Grp2Mode::One: shift = 1; Idle(); break;
		case Grp2Mode::CL: shift = _state.CX & 0xFF; Idle<3>(); break;
		case Grp2Mode::Immediate: shift = ReadCodeByte(); Idle<3>(); break;
	}

	shift &= 0x1F;

	T result;
	switch(_modRm.Register) {
		default:
		case 0x00: result = ROL(param, shift); break;
		case 0x01: result = ROR(param, shift); break;
		case 0x02: result = RCL(param, shift); break;
		case 0x03: result = RCR(param, shift); break;
		case 0x04: result = SHL(param, shift); break;
		case 0x05: result = SHR(param, shift); break;
		case 0x06: result = 0; break;
		case 0x07: result = SAR(param, shift); break;
	}
	SetModRm(result);
}

template<typename T>
void WsCpu::Grp3ModRm()
{
	ReadModRmByte();

	T param = GetModRm<T>();

	Idle();

	switch(_modRm.Register) {
		case 0x00: And<T>(param, ReadImmediate<T>()); break; //TEST
		case 0x01: break; //NOP
		case 0x02: param = ~param; SetModRm(param); break;
		case 0x03: param = Sub<T>(0, param, 0); SetModRm(param); break; //NEG
		case 0x04: Idle<2>(); MulUnsigned<T>((T)_state.AX, param); break;
		case 0x05: Idle<2>(); MulSigned<T>((T)_state.AX, param); break;
		case 0x06: DivUnsigned<T>(param); break;
		case 0x07: DivSigned<T>(param); break;
	}
}

template<typename T>
void WsCpu::Grp45ModRm()
{
	ReadModRmByte();

	switch(_modRm.Register) {
		case 0x00: {
			T result = GetModRm<T>();
			Inc<T>(result);
			SetModRm(result);
			break;
		}

		case 0x01: {
			T result = GetModRm<T>();
			Dec<T>(result);
			SetModRm(result);
			break;
		}

		case 0x02:
			Idle<2>();
			Call(GetModRm<uint16_t>());
			break;

		case 0x03: {
			uint16_t offset = GetModRm<uint16_t>();
			_modRm.Offset += 2;
			uint16_t segment = GetModRm<uint16_t>();
			CallFar(segment, offset);
			break;
		}

		case 0x04: {
			Idle<4>();
			uint16_t offset = GetModRm<uint16_t>();
			_state.IP = offset;
			ClearPrefetch();
			break;
		}

		case 0x05: {
			Idle<7>();
			uint16_t offset = GetModRm<uint16_t>();
			_modRm.Offset += 2;
			uint16_t segment = GetModRm<uint16_t>();
			_state.CS = segment;
			_state.IP = offset;
			ClearPrefetch();
			break;
		}

		case 0x06:
			Push(GetModRm<uint16_t>());
			break;

		case 0x07:
			Undefined();
			break;
	}
}

template<typename T>
void WsCpu::TestModRm()
{
	Idle();

	ReadModRmByte();

	T param1 = GetModRm<T>();
	T param2 = GetModRegister<T>(_modRm.Register);

	And<T>(param1, param2);
}

template<typename T>
void WsCpu::TestImmediate()
{
	Idle();
	T imm = ReadImmediate<T>();
	And<T>(_state.AX, imm);
}

template<typename T>
void WsCpu::ExchangeModRm()
{
	Idle<3>();
	ReadModRmByte();

	T param1 = GetModRm<T>();
	T param2 = GetModRegister<T>(_modRm.Register);

	SetModRegister(_modRm.Register, param1);
	SetModRm(param2);
}

template<bool direction, typename T>
void WsCpu::MoveModRm()
{
	ReadModRmByte();

	if(_modRm.Mode == 0x03) {
		Idle();
	}

	T param = direction ? GetModRm<T>() : GetModRegister<T>(_modRm.Register);

	if constexpr(direction) {
		SetModRegister(_modRm.Register, param);
	} else {
		SetModRm(param);
	}
}

template<bool direction>
void WsCpu::MoveSegment()
{
	Idle<direction ? 2 : 1>();

	ReadModRmByte();

	uint16_t param = direction ? GetModRm<uint16_t>() : GetModSegRegister(_modRm.Register);

	if constexpr(direction) {
		SetModSegRegister(_modRm.Register, param);
	} else {
		SetModRm(param);
	}
}

template<bool direction, typename T>
void WsCpu::MoveAccumulator()
{
	uint16_t offset = ReadCodeWord();
	T param;
	if(direction) {
		param = (T)_state.AX;
	} else {
		param = ReadMemory<T>(GetSegment(WsSegment::DS), offset);
	}

	if constexpr(direction) {
		WriteMemory<T>(GetSegment(WsSegment::DS), offset, param);
	} else {
		//Write to AL/AX
		SetModRegister(0, param);
	}
}

template<typename T>
void WsCpu::MoveImmediate()
{
	ReadModRmByte();
	T imm = ReadImmediate<T>();
	SetModRm(imm);
}

template<WsCpu::AluOp op, bool direction, typename T>
void WsCpu::ProcessAluModRm()
{
	Idle();

	ReadModRmByte();
	T param1 = direction ? GetModRegister<T>(_modRm.Register) : GetModRm<T>();
	T param2 = direction ? GetModRm<T>() : GetModRegister<T>(_modRm.Register);

	T result = GetAluResult<T>(op, param1, param2);
	if constexpr(op != AluOp::Cmp) {
		if constexpr(direction) {
			SetModRegister(_modRm.Register, result);
		} else {
			SetModRm(result);
		}
	}
}

template<WsCpu::AluOp op, typename T>
void WsCpu::ProcessAluImm()
{
	Idle();

	T param1 = (T)_state.AX;
	T param2 = ReadImmediate<T>();

	T result = GetAluResult<T>(op, param1, param2);

	if constexpr(op != AluOp::Cmp) {
		SetModRegister(0, result);
	}
}

template<typename T>
T WsCpu::GetAluResult(AluOp op, T param1, T param2)
{
	switch(op) {
		default:
		case AluOp::Add: return Add<T>(param1, param2, 0);
		case AluOp::Or: return Or<T>(param1, param2);
		case AluOp::Adc: return Add<T>(param1, param2, _state.Flags.Carry);
		case AluOp::Sbb: return Sub<T>(param1, param2, _state.Flags.Carry);
		case AluOp::And: return And<T>(param1, param2);
		case AluOp::Sub: return Sub<T>(param1, param2, 0);
		case AluOp::Xor: return Xor<T>(param1, param2);
		case AluOp::Cmp: return Sub<T>(param1, param2, 0);
	}
}

template<typename T>
void WsCpu::Inc(T& dst)
{
	Idle();

	T result = dst + 1;
	_state.Flags.Zero = result == 0;
	_state.Flags.AuxCarry = (dst & 0x0F) == 0x0F;
	_state.Flags.Overflow = result == GetSign<T>();
	_state.Flags.Sign = result & GetSign<T>();
	_state.Flags.Parity = _parity.CheckParity(result);
	dst = result;
}

template<typename T>
void WsCpu::Dec(T& dst)
{
	Idle();

	T result = dst - 1;
	_state.Flags.Zero = result == 0;
	_state.Flags.AuxCarry = (dst & 0x0F) == 0;
	_state.Flags.Overflow = result == GetSign<T>() - 1;
	_state.Flags.Sign = result & GetSign<T>();
	_state.Flags.Parity = _parity.CheckParity(result);
	dst = result;
}

template<typename T>
T WsCpu::Add(T x, T y, uint8_t carry)
{
	uint32_t result = x + y + carry;

	_state.Flags.Carry = result > GetMaxValue<T>();
	result &= GetMaxValue<T>();

	_state.Flags.Zero = result == 0;
	_state.Flags.AuxCarry = (x & 0x0F) + (y & 0x0F) + carry >= 0x10;
	_state.Flags.Overflow = (result ^ x) & (result ^ y) & GetSign<T>();
	_state.Flags.Sign = result & GetSign<T>();
	_state.Flags.Parity = _parity.CheckParity(result);

	return result;
}

template<typename T>
T WsCpu::Sub(T x, T y, uint8_t borrow)
{
	int32_t result = (int32_t)x - (int32_t)y - (int32_t)borrow;

	_state.Flags.Carry = result < 0;
	result &= GetMaxValue<T>();

	_state.Flags.Zero = result == 0;
	_state.Flags.AuxCarry = (int8_t)(x & 0x0F) - (int8_t)(y & 0x0F) - (int8_t)borrow < 0;
	_state.Flags.Overflow = (x ^ y) & (x ^ result) & GetSign<T>();
	_state.Flags.Sign = result & GetSign<T>();
	_state.Flags.Parity = _parity.CheckParity(result);

	return (T)result;
}

template<typename T>
void WsCpu::MultSignedModRm()
{
	Idle<3>();
	ReadModRmByte();

	int16_t param1;
	if constexpr(std::is_same<T, uint16_t>::value) {
		param1 = (int16_t)ReadImmediate<T>();
	} else {
		param1 = (int8_t)ReadImmediate<T>();
	}
	uint16_t param2 = GetModRm<uint16_t>();

	int32_t result = GetMultiplyResult<uint16_t>(param1, param2);
	SetModRegister<uint16_t>(_modRm.Register, result);
}

template<typename T>
void WsCpu::MulSigned(T x, T y)
{
	int32_t result = GetMultiplyResult(x, y);
	if constexpr(std::is_same<T, uint16_t>::value) {
		_state.AX = (uint16_t)result;
		_state.DX = (uint16_t)(result >> 16);
	} else {
		_state.AX = (uint16_t)result;
	}
}

template<typename T>
int32_t WsCpu::GetMultiplyResult(T x, T y)
{
	int32_t result;
	bool overflow;
	if constexpr(std::is_same<T, uint16_t>::value) {
		result = (int16_t)x * (int16_t)y;
		overflow = result != (int16_t)result;
	} else {
		result = (int8_t)x * (int8_t)y;
		overflow = result != (int8_t)result;
	}

	_state.Flags.Carry = overflow;
	_state.Flags.Overflow = overflow;
	_state.Flags.AuxCarry = false;
	_state.Flags.Parity = false;
	_state.Flags.Sign = false;
	_state.Flags.Zero = true;
	_mulOverflow = overflow;

	return result;
}

template<typename T>
void WsCpu::MulUnsigned(T x, T y)
{
	uint32_t result = x * y;
	bool overflow = result > GetMaxValue<T>();
	_state.Flags.Carry = overflow;
	_state.Flags.Overflow = overflow;
	_state.Flags.AuxCarry = false;
	_state.Flags.Parity = false;
	_state.Flags.Sign = false;
	_state.Flags.Zero = true;

	_mulOverflow = overflow;

	if constexpr(std::is_same<T, uint16_t>::value) {
		_state.AX = (uint16_t)result;
		_state.DX = (uint16_t)(result >> 16);
	} else {
		_state.AX = (uint16_t)result;
	}
}

template<typename T>
void WsCpu::DivSigned(T y)
{
	Idle<14>();

	if constexpr(std::is_same<T, uint16_t>::value) {
		_state.Flags.Carry = false;
		_state.Flags.Overflow = false;
		_state.Flags.AuxCarry = false;
		_state.Flags.Parity = false;
		_state.Flags.Sign = false;

		if(_state.DX == 0x8000 && _state.AX == 0 && y == 0) {
			//CPU bug? 0x80000000 / 0x0000 does not trigger a divide by 0 error
			_state.AX = 0x8001;
			_state.DX = 0;
			_state.Flags.Zero = true;
			return;
		}
	} else {
		_state.Flags.AuxCarry = false;
		_state.Flags.Carry = _mulOverflow;
		_state.Flags.Overflow = _mulOverflow;

		if(_state.AX == 0x8000 && y == 0) {
			//CPU bug? 0x8000 / 0x00 does not trigger a divide by 0 error
			_state.AX = 0x81;
			_state.Flags.Zero = false;
			_state.Flags.Parity = true;
			_state.Flags.Sign = true;
			_state.Flags.Carry = false;
			_state.Flags.Overflow = false;
			return;
		}
	}

	if(y == 0) {
		_state.Flags.Parity = false;
		_state.Flags.Sign = false;
		ProcessInvalidDiv();
		return;
	}

	Idle<std::is_same<T, uint16_t>::value ? 9 : 2>();

	int32_t result;
	int32_t mod;
	bool overflow;
	if constexpr(std::is_same<T, uint16_t>::value) {
		int32_t x = (_state.DX << 16) | _state.AX;
		result = x / (int16_t)y;
		mod = x % (int16_t)y;
		overflow = result > INT16_MAX || result < -INT16_MAX;
	} else {
		result = (int16_t)_state.AX / (int8_t)y;
		mod = (int16_t)_state.AX % (int8_t)y;
		overflow = result > INT8_MAX || result < -INT8_MAX;
	}

	if(overflow) {
		//Result too large
		_state.Flags.Parity = false;
		_state.Flags.Sign = false;
		ProcessInvalidDiv();
		return;
	}

	if constexpr(std::is_same<T, uint16_t>::value) {
		_state.AX = (int16_t)result;
		_state.DX = (int16_t)mod;
		_state.Flags.Zero = mod == 0 && (result & 0x01);
	} else {
		_state.AX = ((uint8_t)mod << 8) | (uint8_t)result;

		uint8_t al = _state.AX & 0xFF;
		_state.Flags.Parity = _parity.CheckParity(al);
		_state.Flags.Zero = al == 0;
		_state.Flags.Sign = al & 0x80;
		_state.Flags.Carry = false;
		_state.Flags.Overflow = false;
	}
}

template<typename T>
void WsCpu::DivUnsigned(T y)
{
	Idle<11>();
	_state.Flags.AuxCarry = false;
	_state.Flags.Parity = false;
	_state.Flags.Sign = false;

	if constexpr(std::is_same<T, uint16_t>::value) {
		_state.Flags.Carry = false;
		_state.Flags.Overflow = false;
	} else {
		_state.Flags.Carry = _mulOverflow;
		_state.Flags.Overflow = _mulOverflow;
	}

	if(y == 0) {
		ProcessInvalidDiv();
		return;
	}

	Idle<std::is_same<T, uint16_t>::value ? 11 : 3>();

	uint32_t x;
	if constexpr(std::is_same<T, uint16_t>::value) {
		x = (_state.DX << 16) | _state.AX;
	} else {
		x = _state.AX;
	}

	uint32_t result = x / y;
	uint32_t mod = x % y;

	if(result > GetMaxValue<T>()) {
		//Result too large
		ProcessInvalidDiv();
		return;
	}

	if constexpr(std::is_same<T, uint16_t>::value) {
		_state.AX = (uint16_t)result;
		_state.DX = (uint16_t)mod;
	} else {
		_state.AX = ((uint8_t)mod << 8) | (uint8_t)result;
	}

	_state.Flags.Zero = mod == 0 && (result & 0x01);
}

template<typename T>
void WsCpu::UpdateFlags(T result)
{
	_state.Flags.Carry = false;
	_state.Flags.Overflow = false;
	_state.Flags.AuxCarry = false;
	_state.Flags.Zero = result == 0;
	_state.Flags.Sign = result & GetSign<T>();
	_state.Flags.Parity = _parity.CheckParity(result);
}

template<typename T>
T WsCpu::And(T x, T y)
{
	T result = x & y;
	UpdateFlags<T>(result);
	return result;
}

template<typename T>
T WsCpu::Or(T x, T y)
{
	T result = x | y;
	UpdateFlags<T>(result);
	return result;
}

template<typename T>
T WsCpu::Xor(T x, T y)
{
	T result = x ^ y;
	UpdateFlags<T>(result);
	return result;
}

template<typename T>
T WsCpu::ROL(T x, uint8_t shift)
{
	uint8_t s = shift % GetBitCount<T>();
	T result = s == 0 ? x : ((x << s) | (x >> (GetBitCount<T>() - s)));
	if(shift != 0) {
		_state.Flags.Carry = result & 0x01;
	}
	_state.Flags.Overflow = (result ^ (_state.Flags.Carry << (GetBitCount<T>() - 1))) & GetSign<T>();
	return result;
}

template<typename T>
T WsCpu::ROR(T x, uint8_t shift)
{
	uint8_t s = shift % GetBitCount<T>();
	T result = s == 0 ? x : ((x >> s) | (x << (GetBitCount<T>() - s)));
	if(shift != 0) {
		_state.Flags.Carry = result & GetSign<T>();
	}
	_state.Flags.Overflow = (result ^ (result << 1)) & GetSign<T>();
	return result;
}

template<typename T>
T WsCpu::RCL(T x, uint8_t shift)
{
	uint8_t s = shift % (GetBitCount<T>() + 1);
	uint64_t result = shift == 0 ? x : ((x << s) | (x >> (GetBitCount<T>() - s + 1)) | ((uint8_t)_state.Flags.Carry << (s > 0 ? (s - 1) : GetBitCount<T>())));
	if(shift != 0) {
		_state.Flags.Carry = (result >> GetBitCount<T>()) & 0x01;
	}
	_state.Flags.Overflow = (result ^ (_state.Flags.Carry << (GetBitCount<T>() - 1))) & GetSign<T>();
	return result;
}

template<typename T>
T WsCpu::RCR(T x, uint8_t shift)
{
	uint8_t s = shift % (GetBitCount<T>() + 1);
	uint64_t result = shift == 0 ? x : ((x >> s) | (x << (GetBitCount<T>() - s + 1)) | ((uint8_t)_state.Flags.Carry << (GetBitCount<T>() - s)));
	if(shift != 0) {
		_state.Flags.Carry = (result >> GetBitCount<T>()) & 0x01;
	}
	_state.Flags.Overflow = (result ^ (result << 1)) & GetSign<T>();
	return result;
}

template<typename T>
T WsCpu::SHL(T x, uint8_t shift)
{
	uint32_t result = x << shift;
	bool carry = _state.Flags.Carry;
	UpdateFlags<T>(result);

	if(shift == 0) {
		_state.Flags.Carry = carry;
	} else {
		_state.Flags.Carry = result & (1 << GetBitCount<T>());
	}

	_state.Flags.Overflow = (result ^ (_state.Flags.Carry << (GetBitCount<T>() - 1))) & GetSign<T>();

	return (T)result;
}

template<typename T>
T WsCpu::SHR(T x, uint8_t shift)
{
	uint64_t result = x >> shift;
	bool carry = _state.Flags.Carry;
	UpdateFlags<T>(result);

	if(shift == 0) {
		_state.Flags.Carry = carry;
	} else {
		_state.Flags.Carry = (x >> (shift - 1)) & 0x01;
	}

	_state.Flags.Overflow = (result ^ (result << 1)) & GetSign<T>();

	return (T)result;
}

template<typename T>
T WsCpu::SAR(T x, uint8_t shift)
{
	T result;
	bool carry;
	if constexpr(std::is_same<T, uint16_t>::value) {
		result = (int16_t)x >> shift;
		carry = ((int16_t)x >> (shift - 1)) & 0x01;
	} else {
		result = (int8_t)x >> shift;
		carry = ((int8_t)x >> (shift - 1)) & 0x01;
	}

	bool orgCarry = _state.Flags.Carry;
	UpdateFlags<T>(result);
	_state.Flags.Carry = shift != 0 ? carry : orgCarry;
	_state.Flags.Overflow = (result ^ (result << 1)) & GetSign<T>();

	return (T)result;
}

template<typename T, uint8_t delay>
void WsCpu::Out(uint16_t port, T data)
{
	Idle<delay>();
	WritePort<T>(port, data);
}

template<typename T, bool isDxPort>
void WsCpu::InStoreAx(uint16_t port)
{
	Idle<isDxPort ? 4 : 5>();
	if constexpr(std::is_same<T, uint16_t>::value) {
		_state.AX = ReadPort<uint16_t>(port);
	} else {
		_state.AX = (_state.AX & 0xFF00) | ReadPort<uint8_t>(port);
	}
	
	//TODOWS
	//The gdma_timing test seems to imply that IN instruciton might potentially
	//be reading the port a cycle early and then spends an extra cycle
	//on something else (e.g potentially updating AX?), but this needs more
	//research/testing. This allows the gdma_timing test to pass.
	Idle();
}

template<typename T>
void WsCpu::ProcessStringOperation(bool incSi, bool incDi)
{
	if(incSi) {
		_state.SI += _state.Flags.Direction ? -1 * sizeof(T) : 1 * sizeof(T);
	}

	if(incDi) {
		_state.DI += _state.Flags.Direction ? -1 * sizeof(T) : 1 * sizeof(T);
	}

	if(_prefix.Rep != WsRepMode::None) {
		_state.CX--;
		if(_state.CX != 0) {
			//Re-execute the same instruction again
#ifndef DUMMYCPU
			_prefetch.ProcessRep(_opCode);
			_state.IP--;
			_prefix.Preserve = true;
#endif
		}
	}
}

template<typename T>
void WsCpu::ProcessStringCmpOperation(bool incSi)
{
	if(incSi) {
		_state.SI += _state.Flags.Direction ? -1 * sizeof(T) : 1 * sizeof(T);
	}

	_state.DI += _state.Flags.Direction ? -1 * sizeof(T) : 1 * sizeof(T);
	if(_prefix.Rep != WsRepMode::None) {
		_state.CX--;
		if(
			_state.CX != 0 &&
			(_prefix.Rep != WsRepMode::NotZero || !_state.Flags.Zero) &&
			(_prefix.Rep != WsRepMode::Zero || _state.Flags.Zero)
			) {
			//Re-execute the same instruction again
#ifndef DUMMYCPU
			_prefetch.ProcessRep(_opCode);
			_state.IP--;
			_prefix.Preserve = true;
#endif
		}
	}
}

template<typename T>
void WsCpu::INS()
{
	if(!_prefix.Preserve && _prefix.Rep != WsRepMode::None) {
		//First iteration
		Idle<5>();
	}

	_prefix.Preserve = false;
	if(_prefix.Rep == WsRepMode::None || _state.CX > 0) {
		if(_prefix.Rep == WsRepMode::None) {
			Idle<3>();
		} else {
			Idle<4>();
		}

		T value = ReadPort<T>(_state.DX);
		WriteMemory<T>(_state.ES, _state.DI, value);
		ProcessStringOperation<T>(false, true);
	}
}

template<typename T>
void WsCpu::OUTS()
{
	if(!_prefix.Preserve && _prefix.Rep != WsRepMode::None) {
		//First iteration
		Idle<5>();
	}

	_prefix.Preserve = false;
	if(_prefix.Rep == WsRepMode::None || _state.CX > 0) {
		if(_prefix.Rep == WsRepMode::None) {
			Idle<3>();
		} else {
			Idle<4>();
		}

		T value = ReadMemory<T>(GetSegment(WsSegment::DS), _state.SI);
		Out<T, 0>(_state.DX, value);
		ProcessStringOperation<T>(true, false);
	}
}

template<typename T>
void WsCpu::MOVS()
{
	if(!_prefix.Preserve && _prefix.Rep != WsRepMode::None) {
		//First iteration
		Idle<5>();
	}

	_prefix.Preserve = false;
	if(_prefix.Rep == WsRepMode::None || _state.CX > 0) {
		if(_prefix.Rep == WsRepMode::None) {
			Idle<3>();
		} else {
			Idle<5>();
		}

		T value = ReadMemory<T>(GetSegment(WsSegment::DS), _state.SI);
		WriteMemory<T>(_state.ES, _state.DI, value);
		ProcessStringOperation<T>(true, true);
	}
}

template<typename T>
void WsCpu::STOS()
{
	if(!_prefix.Preserve && _prefix.Rep != WsRepMode::None) {
		//First iteration
		Idle<5>();
	}

	_prefix.Preserve = false;
	if(_prefix.Rep == WsRepMode::None || _state.CX > 0) {
		if(_prefix.Rep == WsRepMode::None) {
			Idle<2>();
		} else {
			Idle<5>();
		}

		WriteMemory<T>(_state.ES, _state.DI, (T)_state.AX);
		ProcessStringOperation<T>(false, true);
	}
}

template<typename T>
void WsCpu::LODS()
{
	if(!_prefix.Preserve && _prefix.Rep != WsRepMode::None) {
		//First iteration
		Idle<5>();
	}

	_prefix.Preserve = false;
	if(_prefix.Rep == WsRepMode::None || _state.CX > 0) {
		if(_prefix.Rep == WsRepMode::None) {
			Idle<2>();
		} else {
			Idle<5>();
		}

		T value = ReadMemory<T>(GetSegment(WsSegment::DS), _state.SI);
		if constexpr(std::is_same<T, uint16_t>::value) {
			_state.AX = value;
		} else {
			_state.AX = (_state.AX & 0xFF00) | value;
		}
		ProcessStringOperation<T>(true, false);
	}
}

template<typename T>
void WsCpu::CMPS()
{
	if(!_prefix.Preserve && _prefix.Rep != WsRepMode::None) {
		//First iteration
		Idle<5>();
	}

	_prefix.Preserve = false;
	if(_prefix.Rep == WsRepMode::None || _state.CX > 0) {
		if(_prefix.Rep == WsRepMode::None) {
			Idle<4>();
		} else {
			Idle<8>();
		}

		T param1 = ReadMemory<T>(GetSegment(WsSegment::DS), _state.SI);
		T param2 = ReadMemory<T>(_state.ES, _state.DI);
		Sub<T>(param1, param2, 0);
		ProcessStringCmpOperation<T>(true);
	}
}

template<typename T>
void WsCpu::SCAS()
{
	if(!_prefix.Preserve && _prefix.Rep != WsRepMode::None) {
		//First iteration
		Idle<5>();
	}

	_prefix.Preserve = false;
	if(_prefix.Rep == WsRepMode::None || _state.CX > 0) {
		if(_prefix.Rep == WsRepMode::None) {
			Idle<3>();
		} else {
			Idle<8>();
		}

		T param = ReadMemory<T>(_state.ES, _state.DI);
		Sub<T>((T)_state.AX, param, 0);
		ProcessStringCmpOperation<T>(false);
	}
}

void WsCpu::ExecOpCode()
{
	uint8_t opCode = ReadCodeByte(true);
	goto start;
afterPrefix:
	opCode = ReadCodeByte();
start:
	_opCode = opCode;
	switch(opCode) {
		case 0x00: ProcessAluModRm<AluOp::Add, false, uint8_t>(); break;
		case 0x01: ProcessAluModRm<AluOp::Add, false, uint16_t>(); break;
		case 0x02: ProcessAluModRm<AluOp::Add, true, uint8_t>(); break;
		case 0x03: ProcessAluModRm<AluOp::Add, true, uint16_t>(); break;
		case 0x04: ProcessAluImm<AluOp::Add, uint8_t>(); break;
		case 0x05: ProcessAluImm<AluOp::Add, uint16_t>(); break;
		case 0x06: PushSegment(_state.ES); break;
		case 0x07: PopSegment(_state.ES); break;
		case 0x08: ProcessAluModRm<AluOp::Or, false, uint8_t>(); break;
		case 0x09: ProcessAluModRm<AluOp::Or, false, uint16_t>(); break;
		case 0x0A: ProcessAluModRm<AluOp::Or, true, uint8_t>(); break;
		case 0x0B: ProcessAluModRm<AluOp::Or, true, uint16_t>(); break;
		case 0x0C: ProcessAluImm<AluOp::Or, uint8_t>(); break;
		case 0x0D: ProcessAluImm<AluOp::Or, uint16_t>(); break;
		case 0x0E: PushSegment(_state.CS); break;
		case 0x0F: Undefined(); break;

		case 0x10: ProcessAluModRm<AluOp::Adc, false, uint8_t>(); break;
		case 0x11: ProcessAluModRm<AluOp::Adc, false, uint16_t>(); break;
		case 0x12: ProcessAluModRm<AluOp::Adc, true, uint8_t>(); break;
		case 0x13: ProcessAluModRm<AluOp::Adc, true, uint16_t>(); break;
		case 0x14: ProcessAluImm<AluOp::Adc, uint8_t>(); break;
		case 0x15: ProcessAluImm<AluOp::Adc, uint16_t>(); break;
		case 0x16: PushSegment(_state.SS); break;
		case 0x17: PopSegment(_state.SS); break;
		case 0x18: ProcessAluModRm<AluOp::Sbb, false, uint8_t>(); break;
		case 0x19: ProcessAluModRm<AluOp::Sbb, false, uint16_t>(); break;
		case 0x1A: ProcessAluModRm<AluOp::Sbb, true, uint8_t>(); break;
		case 0x1B: ProcessAluModRm<AluOp::Sbb, true, uint16_t>(); break;
		case 0x1C: ProcessAluImm<AluOp::Sbb, uint8_t>(); break;
		case 0x1D: ProcessAluImm<AluOp::Sbb, uint16_t>(); break;
		case 0x1E: PushSegment(_state.DS); break;
		case 0x1F: PopSegment(_state.DS); break;

		case 0x20: ProcessAluModRm<AluOp::And, false, uint8_t>(); break;
		case 0x21: ProcessAluModRm<AluOp::And, false, uint16_t>(); break;
		case 0x22: ProcessAluModRm<AluOp::And, true, uint8_t>(); break;
		case 0x23: ProcessAluModRm<AluOp::And, true, uint16_t>(); break;
		case 0x24: ProcessAluImm<AluOp::And, uint8_t>(); break;
		case 0x25: ProcessAluImm<AluOp::And, uint16_t>(); break;
		case 0x26: _prefix.Segment = WsSegment::ES; _prefix.PrefixCount++; goto afterPrefix; break;
		case 0x27: DAA(); break;
		case 0x28: ProcessAluModRm<AluOp::Sub, false, uint8_t>(); break;
		case 0x29: ProcessAluModRm<AluOp::Sub, false, uint16_t>(); break;
		case 0x2A: ProcessAluModRm<AluOp::Sub, true, uint8_t>(); break;
		case 0x2B: ProcessAluModRm<AluOp::Sub, true, uint16_t>(); break;
		case 0x2C: ProcessAluImm<AluOp::Sub, uint8_t>(); break;
		case 0x2D: ProcessAluImm<AluOp::Sub, uint16_t>(); break;
		case 0x2E: _prefix.Segment = WsSegment::CS; _prefix.PrefixCount++; goto afterPrefix; break;
		case 0x2F: DAS(); break;

		case 0x30: ProcessAluModRm<AluOp::Xor, false, uint8_t>(); break;
		case 0x31: ProcessAluModRm<AluOp::Xor, false, uint16_t>(); break;
		case 0x32: ProcessAluModRm<AluOp::Xor, true, uint8_t>(); break;
		case 0x33: ProcessAluModRm<AluOp::Xor, true, uint16_t>(); break;
		case 0x34: ProcessAluImm<AluOp::Xor, uint8_t>(); break;
		case 0x35: ProcessAluImm<AluOp::Xor, uint16_t>(); break;
		case 0x36: _prefix.Segment = WsSegment::SS; _prefix.PrefixCount++; goto afterPrefix; break;
		case 0x37: AAA(); break;
		case 0x38: ProcessAluModRm<AluOp::Cmp, false, uint8_t>(); break;
		case 0x39: ProcessAluModRm<AluOp::Cmp, false, uint16_t>(); break;
		case 0x3A: ProcessAluModRm<AluOp::Cmp, true, uint8_t>(); break;
		case 0x3B: ProcessAluModRm<AluOp::Cmp, true, uint16_t>(); break;
		case 0x3C: ProcessAluImm<AluOp::Cmp, uint8_t>(); break;
		case 0x3D: ProcessAluImm<AluOp::Cmp, uint16_t>(); break;
		case 0x3E: _prefix.Segment = WsSegment::DS; _prefix.PrefixCount++; goto afterPrefix; break;
		case 0x3F: AAS(); break;

		case 0x40: Inc(_state.AX); break;
		case 0x41: Inc(_state.CX); break;
		case 0x42: Inc(_state.DX); break;
		case 0x43: Inc(_state.BX); break;
		case 0x44: Inc(_state.SP); break;
		case 0x45: Inc(_state.BP); break;
		case 0x46: Inc(_state.SI); break;
		case 0x47: Inc(_state.DI); break;
		case 0x48: Dec(_state.AX); break;
		case 0x49: Dec(_state.CX); break;
		case 0x4A: Dec(_state.DX); break;
		case 0x4B: Dec(_state.BX); break;
		case 0x4C: Dec(_state.SP); break;
		case 0x4D: Dec(_state.BP); break;
		case 0x4E: Dec(_state.SI); break;
		case 0x4F: Dec(_state.DI); break;

		case 0x50: Push(_state.AX); break;
		case 0x51: Push(_state.CX); break;
		case 0x52: Push(_state.DX); break;
		case 0x53: Push(_state.BX); break;
		case 0x54: PushSP(); break;
		case 0x55: Push(_state.BP); break;
		case 0x56: Push(_state.SI); break;
		case 0x57: Push(_state.DI); break;
		case 0x58: Pop(_state.AX); break;
		case 0x59: Pop(_state.CX); break;
		case 0x5A: Pop(_state.DX); break;
		case 0x5B: Pop(_state.BX); break;
		case 0x5C: Pop(_state.SP); break;
		case 0x5D: Pop(_state.BP); break;
		case 0x5E: Pop(_state.SI); break;
		case 0x5F: Pop(_state.DI); break;

		case 0x60: PushAll(); break;
		case 0x61: PopAll(); break;
		case 0x62: BOUND(); break;
		case 0x63: Undefined(); break;
		case 0x64: Undefined(); break;
		case 0x65: Undefined(); break;
		case 0x66: Undefined(); break;
		case 0x67: Undefined(); break;
		case 0x68: Push(ReadCodeWord()); break;
		case 0x69: MultSignedModRm<uint16_t>(); break;
		case 0x6A: Push((int16_t)(int8_t)ReadCodeByte()); break;
		case 0x6B: MultSignedModRm<uint8_t>(); break;
		case 0x6C: INS<uint8_t>(); break;
		case 0x6D: INS<uint16_t>(); break;
		case 0x6E: OUTS<uint8_t>(); break;
		case 0x6F: OUTS<uint16_t>(); break;

		case 0x70: Jump(_state.Flags.Overflow); break;
		case 0x71: Jump(!_state.Flags.Overflow); break;
		case 0x72: Jump(_state.Flags.Carry); break;
		case 0x73: Jump(!_state.Flags.Carry); break;
		case 0x74: Jump(_state.Flags.Zero); break;
		case 0x75: Jump(!_state.Flags.Zero); break;
		case 0x76: Jump(_state.Flags.Carry || _state.Flags.Zero); break;
		case 0x77: Jump(!_state.Flags.Carry && !_state.Flags.Zero); break;
		case 0x78: Jump(_state.Flags.Sign); break;
		case 0x79: Jump(!_state.Flags.Sign); break;
		case 0x7A: Jump(_state.Flags.Parity); break;
		case 0x7B: Jump(!_state.Flags.Parity); break;
		case 0x7C: Jump(_state.Flags.Sign != _state.Flags.Overflow); break;
		case 0x7D: Jump(_state.Flags.Sign == _state.Flags.Overflow); break;
		case 0x7E: Jump(_state.Flags.Sign != _state.Flags.Overflow || _state.Flags.Zero); break;
		case 0x7F: Jump(_state.Flags.Sign == _state.Flags.Overflow && !_state.Flags.Zero); break;

		case 0x80: Grp1ModRm<false, uint8_t>(); break;
		case 0x81: Grp1ModRm<false, uint16_t>(); break;
		case 0x82: Grp1ModRm<true, uint8_t>(); break;
		case 0x83: Grp1ModRm<true, uint16_t>(); break;
		case 0x84: TestModRm<uint8_t>(); break;
		case 0x85: TestModRm<uint16_t>(); break;
		case 0x86: ExchangeModRm<uint8_t>(); break;
		case 0x87: ExchangeModRm<uint16_t>(); break;
		case 0x88: MoveModRm<false, uint8_t>(); break;
		case 0x89: MoveModRm<false, uint16_t>(); break;
		case 0x8A: MoveModRm<true, uint8_t>(); break;
		case 0x8B: MoveModRm<true, uint16_t>(); break;
		case 0x8C: MoveSegment<false>(); break;
		case 0x8D: LEA(); break;
		case 0x8E: MoveSegment<true>(); break;
		case 0x8F: PopMemory(); break;

		case 0x90: NOP(); break;
		case 0x91: Exchange(_state.CX, _state.AX); break;
		case 0x92: Exchange(_state.DX, _state.AX); break;
		case 0x93: Exchange(_state.BX, _state.AX); break;
		case 0x94: Exchange(_state.SP, _state.AX); break;
		case 0x95: Exchange(_state.BP, _state.AX); break;
		case 0x96: Exchange(_state.SI, _state.AX); break;
		case 0x97: Exchange(_state.DI, _state.AX); break;
		case 0x98: CBW(); break;
		case 0x99: CWD(); break;
		case 0x9A: CallFar(); break;
		case 0x9B: Wait(); break;
		case 0x9C: PushFlags(); break;
		case 0x9D: PopFlags(); break;
		case 0x9E: SAHF(); break;
		case 0x9F: LAHF(); break;

		case 0xA0: MoveAccumulator<false, uint8_t>(); break;
		case 0xA1: MoveAccumulator<false, uint16_t>(); break;
		case 0xA2: MoveAccumulator<true, uint8_t>(); break;
		case 0xA3: MoveAccumulator<true, uint16_t>(); break;
		case 0xA4: MOVS<uint8_t>(); break;
		case 0xA5: MOVS<uint16_t>(); break;
		case 0xA6: CMPS<uint8_t>(); break;
		case 0xA7: CMPS<uint16_t>(); break;
		case 0xA8: TestImmediate<uint8_t>(); break;
		case 0xA9: TestImmediate<uint16_t>(); break;
		case 0xAA: STOS<uint8_t>(); break;
		case 0xAB: STOS<uint16_t>(); break;
		case 0xAC: LODS<uint8_t>(); break;
		case 0xAD: LODS<uint16_t>(); break;
		case 0xAE: SCAS<uint8_t>(); break;
		case 0xAF: SCAS<uint16_t>(); break;

		case 0xB0: MoveLo(_state.AX, ReadCodeByte()); break;
		case 0xB1: MoveLo(_state.CX, ReadCodeByte()); break;
		case 0xB2: MoveLo(_state.DX, ReadCodeByte()); break;
		case 0xB3: MoveLo(_state.BX, ReadCodeByte()); break;
		case 0xB4: MoveHi(_state.AX, ReadCodeByte()); break;
		case 0xB5: MoveHi(_state.CX, ReadCodeByte()); break;
		case 0xB6: MoveHi(_state.DX, ReadCodeByte()); break;
		case 0xB7: MoveHi(_state.BX, ReadCodeByte()); break;
		case 0xB8: Move(_state.AX, ReadCodeWord()); break;
		case 0xB9: Move(_state.CX, ReadCodeWord()); break;
		case 0xBA: Move(_state.DX, ReadCodeWord()); break;
		case 0xBB: Move(_state.BX, ReadCodeWord()); break;
		case 0xBC: Move(_state.SP, ReadCodeWord()); break;
		case 0xBD: Move(_state.BP, ReadCodeWord()); break;
		case 0xBE: Move(_state.SI, ReadCodeWord()); break;
		case 0xBF: Move(_state.DI, ReadCodeWord()); break;

		case 0xC0: Grp2ModRm<uint8_t, Grp2Mode::Immediate>(); break;
		case 0xC1: Grp2ModRm<uint16_t, Grp2Mode::Immediate>(); break;
		case 0xC2: RetImm(); break;
		case 0xC3: Ret(); break;
		case 0xC4: LES(); break;
		case 0xC5: LDS(); break;
		case 0xC6: MoveImmediate<uint8_t>(); break;
		case 0xC7: MoveImmediate<uint16_t>(); break;
		case 0xC8: Enter(); break;
		case 0xC9: Leave(); break;
		case 0xCA: RetFarImm(); break;
		case 0xCB: RetFar(); break;
		case 0xCC: Idle<5>(); Interrupt(3); break;
		case 0xCD: Idle<6>(); Interrupt(ReadCodeByte()); break;
		case 0xCE: InterruptOverflow(); break;
		case 0xCF: RetInterrupt(); break;

		case 0xD0: Grp2ModRm<uint8_t, Grp2Mode::One>(); break;
		case 0xD1: Grp2ModRm<uint16_t, Grp2Mode::One>(); break;
		case 0xD2: Grp2ModRm<uint8_t, Grp2Mode::CL>(); break;
		case 0xD3: Grp2ModRm<uint16_t, Grp2Mode::CL>(); break;
		case 0xD4: AAM(); break;
		case 0xD5: AAD(); break;
		case 0xD6: SALC(); break;
		case 0xD7: XLAT(); break;
		case 0xD8: FP01(); break;
		case 0xD9: FP01(); break;
		case 0xDA: FP01(); break;
		case 0xDB: FP01(); break;
		case 0xDC: FP01(); break;
		case 0xDD: FP01(); break;
		case 0xDE: FP01(); break;
		case 0xDF: FP01(); break;

		case 0xE0: LoopIf(!_state.Flags.Zero); break;
		case 0xE1: LoopIf(_state.Flags.Zero); break;
		case 0xE2: Loop(); break;
		case 0xE3: Jump(_state.CX == 0); break;
		case 0xE4: InStoreAx<uint8_t, false>(ReadCodeByte()); break;
		case 0xE5: InStoreAx<uint16_t, false>(ReadCodeByte()); break;
		case 0xE6: Out<uint8_t, 6>(ReadCodeByte(), _state.AX); break;
		case 0xE7: Out<uint16_t, 6>(ReadCodeByte(), _state.AX); break;
		case 0xE8: CallNearWord(); break;
		case 0xE9: JumpNearWord(); break;
		case 0xEA: JumpFar(); break;
		case 0xEB: Jump(true); break;
		case 0xEC: InStoreAx<uint8_t, true>(_state.DX); break;
		case 0xED: InStoreAx<uint16_t, true>(_state.DX); break;
		case 0xEE: Out<uint8_t, 4>(_state.DX, _state.AX); break;
		case 0xEF: Out<uint16_t, 4>(_state.DX, _state.AX); break;

		case 0xF0: _prefix.Lock = true;  _prefix.PrefixCount++; goto afterPrefix; break;
		case 0xF1: Undefined(); break;
		case 0xF2: _prefix.Rep = WsRepMode::NotZero; _prefix.PrefixCount++; goto afterPrefix; break;
		case 0xF3: _prefix.Rep = WsRepMode::Zero; _prefix.PrefixCount++; goto afterPrefix; break;
		case 0xF4: Halt(); break;
		case 0xF5: SetFlagValue(_state.Flags.Carry, !_state.Flags.Carry); break; //CMC
		case 0xF6: Grp3ModRm<uint8_t>(); break;
		case 0xF7: Grp3ModRm<uint16_t>(); break;
		case 0xF8: SetFlagValue(_state.Flags.Carry, false); break; //CLC
		case 0xF9: SetFlagValue(_state.Flags.Carry, true); break; //STC
		case 0xFA: SetFlagValue(_state.Flags.Irq, false); break; //CLI
		case 0xFB: SetFlagValue(_state.Flags.Irq, true); break; //STI
		case 0xFC: SetFlagValue(_state.Flags.Direction, false); break; //CLD
		case 0xFD: SetFlagValue(_state.Flags.Direction, true); break; //STD
		case 0xFE: Grp45ModRm<uint8_t>(); break;
		case 0xFF: Grp45ModRm<uint16_t>(); break;
	}

	if(!_prefix.Preserve) {
		_prefix = {};
	}

	if(_state.Flags.Trap) {
		Interrupt(1);
	}
}

void WsCpu::Serialize(Serializer& s)
{
	SV(_state.CycleCount);
	SV(_state.CS);
	SV(_state.IP);
	SV(_state.SS);
	SV(_state.SP);
	SV(_state.BP);
	SV(_state.DS);
	SV(_state.ES);
	SV(_state.SI);
	SV(_state.DI);
	SV(_state.AX);
	SV(_state.BX);
	SV(_state.CX);
	SV(_state.DX);
	SV(_state.Halted);
	SV(_state.Flags.AuxCarry);
	SV(_state.Flags.Carry);
	SV(_state.Flags.Direction);
	SV(_state.Flags.Irq);
	SV(_state.Flags.Mode);
	SV(_state.Flags.Overflow);
	SV(_state.Flags.Parity);
	SV(_state.Flags.Sign);
	SV(_state.Flags.Trap);
	SV(_state.Flags.Zero);

	SV(_opCode);
	SV(_mulOverflow);
#ifndef DUMMYCPU
	SV(_prefetch);
#endif
	SV(_prefix.Lock);
	SV(_prefix.PrefixCount);
	SV(_prefix.Preserve);
	SV(_prefix.Rep);
	SV(_prefix.Segment);
	
	SV(_modRm.Segment);
	SV(_modRm.Offset);
	SV(_modRm.Mode);
	SV(_modRm.Register);
	SV(_modRm.Rm);
}
