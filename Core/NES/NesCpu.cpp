#include "stdafx.h"
#include <random>
#include <assert.h>
#include "../../Utilities/Serializer.h"
#include "../MessageManager.h"
#include "../EmuSettings.h"
#include "NesCpu.h"
#include "NesPpu.h"
#include "NesApu.h"
#include "DeltaModulationChannel.h"
#include "Debugger.h"
#include "NesMemoryManager.h"
#include "NesConsole.h"
#include "MemoryOperationType.h"
#include "Emulator.h"

NesCpu::NesCpu(shared_ptr<NesConsole> console)
{
	_console = console;
	_memoryManager = _console->GetMemoryManager();

	Func opTable[] = { 
	//	0					1					2					3					4					5					6							7					8					9					A							B					C							D					E							F
		&NesCpu::BRK,	&NesCpu::ORA,	&NesCpu::HLT,	&NesCpu::SLO,	&NesCpu::NOP,	&NesCpu::ORA,	&NesCpu::ASL_Memory,	&NesCpu::SLO,	&NesCpu::PHP,	&NesCpu::ORA,	&NesCpu::ASL_Acc,		&NesCpu::AAC,	&NesCpu::NOP,			&NesCpu::ORA,	&NesCpu::ASL_Memory,	&NesCpu::SLO, //0
		&NesCpu::BPL,	&NesCpu::ORA,	&NesCpu::HLT,	&NesCpu::SLO,	&NesCpu::NOP,	&NesCpu::ORA,	&NesCpu::ASL_Memory,	&NesCpu::SLO,	&NesCpu::CLC,	&NesCpu::ORA,	&NesCpu::NOP,			&NesCpu::SLO,	&NesCpu::NOP,			&NesCpu::ORA,	&NesCpu::ASL_Memory,	&NesCpu::SLO, //1
		&NesCpu::JSR,	&NesCpu::AND,	&NesCpu::HLT,	&NesCpu::RLA,	&NesCpu::BIT,	&NesCpu::AND,	&NesCpu::ROL_Memory,	&NesCpu::RLA,	&NesCpu::PLP,	&NesCpu::AND,	&NesCpu::ROL_Acc,		&NesCpu::AAC,	&NesCpu::BIT,			&NesCpu::AND,	&NesCpu::ROL_Memory,	&NesCpu::RLA, //2
		&NesCpu::BMI,	&NesCpu::AND,	&NesCpu::HLT,	&NesCpu::RLA,	&NesCpu::NOP,	&NesCpu::AND,	&NesCpu::ROL_Memory,	&NesCpu::RLA,	&NesCpu::SEC,	&NesCpu::AND,	&NesCpu::NOP,			&NesCpu::RLA,	&NesCpu::NOP,			&NesCpu::AND,	&NesCpu::ROL_Memory,	&NesCpu::RLA, //3
		&NesCpu::RTI,	&NesCpu::EOR,	&NesCpu::HLT,	&NesCpu::SRE,	&NesCpu::NOP,	&NesCpu::EOR,	&NesCpu::LSR_Memory,	&NesCpu::SRE,	&NesCpu::PHA,	&NesCpu::EOR,	&NesCpu::LSR_Acc,		&NesCpu::ASR,	&NesCpu::JMP_Abs,		&NesCpu::EOR,	&NesCpu::LSR_Memory,	&NesCpu::SRE, //4
		&NesCpu::BVC,	&NesCpu::EOR,	&NesCpu::HLT,	&NesCpu::SRE,	&NesCpu::NOP,	&NesCpu::EOR,	&NesCpu::LSR_Memory,	&NesCpu::SRE,	&NesCpu::CLI,	&NesCpu::EOR,	&NesCpu::NOP,			&NesCpu::SRE,	&NesCpu::NOP,			&NesCpu::EOR,	&NesCpu::LSR_Memory,	&NesCpu::SRE, //5
		&NesCpu::RTS,	&NesCpu::ADC,	&NesCpu::HLT,	&NesCpu::RRA,	&NesCpu::NOP,	&NesCpu::ADC,	&NesCpu::ROR_Memory,	&NesCpu::RRA,	&NesCpu::PLA,	&NesCpu::ADC,	&NesCpu::ROR_Acc,		&NesCpu::ARR,	&NesCpu::JMP_Ind,		&NesCpu::ADC,	&NesCpu::ROR_Memory,	&NesCpu::RRA, //6
		&NesCpu::BVS,	&NesCpu::ADC,	&NesCpu::HLT,	&NesCpu::RRA,	&NesCpu::NOP,	&NesCpu::ADC,	&NesCpu::ROR_Memory,	&NesCpu::RRA,	&NesCpu::SEI,	&NesCpu::ADC,	&NesCpu::NOP,			&NesCpu::RRA,	&NesCpu::NOP,			&NesCpu::ADC,	&NesCpu::ROR_Memory,	&NesCpu::RRA, //7
		&NesCpu::NOP,	&NesCpu::STA,	&NesCpu::NOP,	&NesCpu::SAX,	&NesCpu::STY,	&NesCpu::STA,	&NesCpu::STX,			&NesCpu::SAX,	&NesCpu::DEY,	&NesCpu::NOP,	&NesCpu::TXA,			&NesCpu::UNK,	&NesCpu::STY,			&NesCpu::STA,	&NesCpu::STX,			&NesCpu::SAX, //8
		&NesCpu::BCC,	&NesCpu::STA,	&NesCpu::HLT,	&NesCpu::AXA,	&NesCpu::STY,	&NesCpu::STA,	&NesCpu::STX,			&NesCpu::SAX,	&NesCpu::TYA,	&NesCpu::STA,	&NesCpu::TXS,			&NesCpu::TAS,	&NesCpu::SYA,			&NesCpu::STA,	&NesCpu::SXA,			&NesCpu::AXA, //9
		&NesCpu::LDY,	&NesCpu::LDA,	&NesCpu::LDX,	&NesCpu::LAX,	&NesCpu::LDY,	&NesCpu::LDA,	&NesCpu::LDX,			&NesCpu::LAX,	&NesCpu::TAY,	&NesCpu::LDA,	&NesCpu::TAX,			&NesCpu::ATX,	&NesCpu::LDY,			&NesCpu::LDA,	&NesCpu::LDX,			&NesCpu::LAX, //A
		&NesCpu::BCS,	&NesCpu::LDA,	&NesCpu::HLT,	&NesCpu::LAX,	&NesCpu::LDY,	&NesCpu::LDA,	&NesCpu::LDX,			&NesCpu::LAX,	&NesCpu::CLV,	&NesCpu::LDA,	&NesCpu::TSX,			&NesCpu::LAS,	&NesCpu::LDY,			&NesCpu::LDA,	&NesCpu::LDX,			&NesCpu::LAX, //B
		&NesCpu::CPY,	&NesCpu::CPA,	&NesCpu::NOP,	&NesCpu::DCP,	&NesCpu::CPY,	&NesCpu::CPA,	&NesCpu::DEC,			&NesCpu::DCP,	&NesCpu::INY,	&NesCpu::CPA,	&NesCpu::DEX,			&NesCpu::AXS,	&NesCpu::CPY,			&NesCpu::CPA,	&NesCpu::DEC,			&NesCpu::DCP, //C
		&NesCpu::BNE,	&NesCpu::CPA,	&NesCpu::HLT,	&NesCpu::DCP,	&NesCpu::NOP,	&NesCpu::CPA,	&NesCpu::DEC,			&NesCpu::DCP,	&NesCpu::CLD,	&NesCpu::CPA,	&NesCpu::NOP,			&NesCpu::DCP,	&NesCpu::NOP,			&NesCpu::CPA,	&NesCpu::DEC,			&NesCpu::DCP, //D
		&NesCpu::CPX,	&NesCpu::SBC,	&NesCpu::NOP,	&NesCpu::ISB,	&NesCpu::CPX,	&NesCpu::SBC,	&NesCpu::INC,			&NesCpu::ISB,	&NesCpu::INX,	&NesCpu::SBC,	&NesCpu::NOP,			&NesCpu::SBC,	&NesCpu::CPX,			&NesCpu::SBC,	&NesCpu::INC,			&NesCpu::ISB, //E
		&NesCpu::BEQ,	&NesCpu::SBC,	&NesCpu::HLT,	&NesCpu::ISB,	&NesCpu::NOP,	&NesCpu::SBC,	&NesCpu::INC,			&NesCpu::ISB,	&NesCpu::SED,	&NesCpu::SBC,	&NesCpu::NOP,			&NesCpu::ISB,	&NesCpu::NOP,			&NesCpu::SBC,	&NesCpu::INC,			&NesCpu::ISB  //F
	};

	typedef AddrMode M;
	AddrMode addrMode[] = {
	//	0			1				2			3				4				5				6				7				8			9			A			B			C			D			E			F
		M::Imp,	M::IndX,		M::None,	M::IndX,		M::Zero,		M::Zero,		M::Zero,		M::Zero,		M::Imp,	M::Imm,	M::Acc,	M::Imm,	M::Abs,	M::Abs,	M::Abs,	M::Abs,	//0
		M::Rel,	M::IndY,		M::None,	M::IndYW,	M::ZeroX,	M::ZeroX,	M::ZeroX,	M::ZeroX,	M::Imp,	M::AbsY,	M::Imp,	M::AbsYW,M::AbsX,	M::AbsX,	M::AbsXW,M::AbsXW,//1
		M::Abs,	M::IndX,		M::None,	M::IndX,		M::Zero,		M::Zero,		M::Zero,		M::Zero,		M::Imp,	M::Imm,	M::Acc,	M::Imm,	M::Abs,	M::Abs,	M::Abs,	M::Abs,	//2
		M::Rel,	M::IndY,		M::None,	M::IndYW,	M::ZeroX,	M::ZeroX,	M::ZeroX,	M::ZeroX,	M::Imp,	M::AbsY,	M::Imp,	M::AbsYW,M::AbsX,	M::AbsX,	M::AbsXW,M::AbsXW,//3
		M::Imp,	M::IndX,		M::None,	M::IndX,		M::Zero,		M::Zero,		M::Zero,		M::Zero,		M::Imp,	M::Imm,	M::Acc,	M::Imm,	M::Abs,	M::Abs,	M::Abs,	M::Abs,	//4
		M::Rel,	M::IndY,		M::None,	M::IndYW,	M::ZeroX,	M::ZeroX,	M::ZeroX,	M::ZeroX,	M::Imp,	M::AbsY,	M::Imp,	M::AbsYW,M::AbsX,	M::AbsX,	M::AbsXW,M::AbsXW,//5
		M::Imp,	M::IndX,		M::None,	M::IndX,		M::Zero,		M::Zero,		M::Zero,		M::Zero,		M::Imp,	M::Imm,	M::Acc,	M::Imm,	M::Ind,	M::Abs,	M::Abs,	M::Abs,	//6
		M::Rel,	M::IndY,		M::None,	M::IndYW,	M::ZeroX,	M::ZeroX,	M::ZeroX,	M::ZeroX,	M::Imp,	M::AbsY,	M::Imp,	M::AbsYW,M::AbsX,	M::AbsX,	M::AbsXW,M::AbsXW,//7
		M::Imm,	M::IndX,		M::Imm,	M::IndX,		M::Zero,		M::Zero,		M::Zero,		M::Zero,		M::Imp,	M::Imm,	M::Imp,	M::Imm,	M::Abs,	M::Abs,	M::Abs,	M::Abs,	//8
		M::Rel,	M::IndYW,	M::None,	M::IndYW,	M::ZeroX,	M::ZeroX,	M::ZeroY,	M::ZeroY,	M::Imp,	M::AbsYW,M::Imp,	M::AbsYW,M::AbsXW,M::AbsXW,M::AbsYW,M::AbsYW,//9
		M::Imm,	M::IndX,		M::Imm,	M::IndX,		M::Zero,		M::Zero,		M::Zero,		M::Zero,		M::Imp,	M::Imm,	M::Imp,	M::Imm,	M::Abs,	M::Abs,	M::Abs,	M::Abs,	//A
		M::Rel,	M::IndY,		M::None,	M::IndY,		M::ZeroX,	M::ZeroX,	M::ZeroY,	M::ZeroY,	M::Imp,	M::AbsY,	M::Imp,	M::AbsY,	M::AbsX,	M::AbsX,	M::AbsY,	M::AbsY,	//B
		M::Imm,	M::IndX,		M::Imm,	M::IndX,		M::Zero,		M::Zero,		M::Zero,		M::Zero,		M::Imp,	M::Imm,	M::Imp,	M::Imm,	M::Abs,	M::Abs,	M::Abs,	M::Abs,	//C
		M::Rel,	M::IndY,		M::None,	M::IndYW,	M::ZeroX,	M::ZeroX,	M::ZeroX,	M::ZeroX,	M::Imp,	M::AbsY,	M::Imp,	M::AbsYW,M::AbsX,	M::AbsX,	M::AbsXW,M::AbsXW,//D
		M::Imm,	M::IndX,		M::Imm,	M::IndX,		M::Zero,		M::Zero,		M::Zero,		M::Zero,		M::Imp,	M::Imm,	M::Imp,	M::Imm,	M::Abs,	M::Abs,	M::Abs,	M::Abs,	//E
		M::Rel,	M::IndY,		M::None,	M::IndYW,	M::ZeroX,	M::ZeroX,	M::ZeroX,	M::ZeroX,	M::Imp,	M::AbsY,	M::Imp,	M::AbsYW,M::AbsX,	M::AbsX,	M::AbsXW,M::AbsXW,//F
	};
	
	memcpy(_opTable, opTable, sizeof(opTable));
	memcpy(_addrMode, addrMode, sizeof(addrMode));

	_instAddrMode = AddrMode::None;
	_state = {};
	_cycleCount = 0;
	_operand = 0;
	_spriteDmaTransfer = false;
	_spriteDmaOffset = 0;
	_needHalt = false;
	_ppuOffset = 0;
	_startClockCount = 6;
	_endClockCount = 6;
	_masterClock = 0;
	_dmcDmaRunning = false;
	_cpuWrite = false;
	_irqMask = 0;
	_state = {};
	_prevRunIrq = false;
	_runIrq = false;
}

void NesCpu::Reset(bool softReset, NesModel model)
{
	_state.NMIFlag = false;
	_state.IRQFlag = 0;

	_spriteDmaTransfer = false;
	_spriteDmaOffset = 0;
	_needHalt = false;
	_dmcDmaRunning = false;
	_lastCrashWarning = 0;

	//Used by NSF code to disable Frame Counter & DMC interrupts
	_irqMask = 0xFF;

	//Use _memoryManager->Read() directly to prevent clocking the PPU/APU when setting PC at reset
	_state.PC = _memoryManager->Read(NesCpu::ResetVector) | _memoryManager->Read(NesCpu::ResetVector+1) << 8;
	_state.DebugPC = _state.PC;
	_state.PreviousDebugPC = _state.PC;

	if(softReset) {
		SetFlags(PSFlags::Interrupt);
		_state.SP -= 0x03;
	} else {
		_state.A = 0;
		_state.SP = 0xFD;
		_state.X = 0;
		_state.Y = 0;
		_state.PS = PSFlags::Interrupt;

		_runIrq = false;
	}

	uint8_t ppuDivider;
	uint8_t cpuDivider;
	switch(model) {
		default:
		case NesModel::NTSC:
			ppuDivider = 4;
			cpuDivider = 12;
			break;

		case NesModel::PAL:
			ppuDivider = 5;
			cpuDivider = 16;
			break;

		case NesModel::Dendy:
			ppuDivider = 5;
			cpuDivider = 15;
			break;
	}

	_cycleCount = -1;
	_masterClock = 0;

	uint8_t cpuOffset = 0;
	if(_console->GetNesConfig().RandomizeCpuPpuAlignment) {
		std::random_device rd;
		std::mt19937 mt(rd());
		std::uniform_int_distribution<> distPpu(0, ppuDivider - 1);
		std::uniform_int_distribution<> distCpu(0, cpuDivider - 1);
		_ppuOffset = distPpu(mt);
		cpuOffset += distCpu(mt);

		string ppuAlignment = " PPU: " + std::to_string(_ppuOffset) + "/" + std::to_string(ppuDivider - 1);
		string cpuAlignment = " CPU: " + std::to_string(cpuOffset) + "/" + std::to_string(cpuDivider - 1);
		MessageManager::Log("CPU/PPU alignment -" + ppuAlignment + cpuAlignment);
	} else {
		_ppuOffset = 1;
		cpuOffset = 0;
	}

	_masterClock += cpuDivider + cpuOffset;

	//The CPU takes 8 cycles before it starts executing the ROM's code after a reset/power up
	for(int i = 0; i < 8; i++) {
		StartCpuCycle(true);
		EndCpuCycle(true);
	}
}

void NesCpu::Exec()
{
	uint8_t opCode = GetOPCode();
	_instAddrMode = _addrMode[opCode];
	_operand = FetchOperand();
	(this->*_opTable[opCode])();
	
	if(_prevRunIrq || _prevNeedNmi) {
		IRQ();
	}
}

void NesCpu::IRQ() 
{
#ifndef DUMMYCPU
	uint16_t originalPc = PC();
#endif

	DummyRead();  //fetch opcode (and discard it - $00 (BRK) is forced into the opcode register instead)
	DummyRead();  //read next instruction byte (actually the same as above, since PC increment is suppressed. Also discarded.)
	Push((uint16_t)(PC()));

	if(_needNmi) {
		_needNmi = false;
		Push((uint8_t)(PS() | PSFlags::Reserved));
		SetFlags(PSFlags::Interrupt);

		SetPC(MemoryReadWord(NesCpu::NMIVector));

		#ifndef DUMMYCPU
		_console->DebugAddTrace("NMI");
		_console->DebugProcessInterrupt(originalPc, _state.PC, true);
		#endif
	} else {
		Push((uint8_t)(PS() | PSFlags::Reserved));
		SetFlags(PSFlags::Interrupt);

		SetPC(MemoryReadWord(NesCpu::IRQVector));

		#ifndef DUMMYCPU
		_console->DebugAddTrace("IRQ");
		_console->DebugProcessInterrupt(originalPc, _state.PC, false);
		#endif
	}
}

void NesCpu::BRK() {
	Push((uint16_t)(PC() + 1));

	uint8_t flags = PS() | PSFlags::Break | PSFlags::Reserved;
	if(_needNmi) {
		_needNmi = false;
		Push((uint8_t)flags);
		SetFlags(PSFlags::Interrupt);

		SetPC(MemoryReadWord(NesCpu::NMIVector));

		#ifndef DUMMYCPU
		_console->DebugAddTrace("NMI");
		#endif
	} else {
		Push((uint8_t)flags);
		SetFlags(PSFlags::Interrupt);

		SetPC(MemoryReadWord(NesCpu::IRQVector));

		#ifndef DUMMYCPU
		_console->DebugAddTrace("IRQ");
		#endif
	}

	//Ensure we don't start an NMI right after running a BRK instruction (first instruction in IRQ handler must run first - needed for nmi_and_brk test)
	_prevNeedNmi = false;
}

void NesCpu::MemoryWrite(uint16_t addr, uint8_t value, MemoryOperationType operationType)
{
#ifdef DUMMYCPU
	if(operationType == MemoryOperationType::Write || operationType == MemoryOperationType::DummyWrite) {
		_writeAddresses[_writeCounter] = addr;
		_isDummyWrite[_writeCounter] = operationType == MemoryOperationType::DummyWrite;
		_writeValue[_writeCounter] = value;
		_writeCounter++;
	}
#else
	_cpuWrite = true;
	StartCpuCycle(false);
	_memoryManager->Write(addr, value, operationType);
	EndCpuCycle(false);
	_cpuWrite = false;
#endif
}

uint8_t NesCpu::MemoryRead(uint16_t addr, MemoryOperationType operationType) {
#ifdef DUMMYCPU
	uint8_t value = _memoryManager->DebugRead(addr);
	if(operationType == MemoryOperationType::Read || operationType == MemoryOperationType::DummyRead) {
		_readAddresses[_readCounter] = addr;
		_readValue[_readCounter] = value;
		_isDummyRead[_readCounter] = operationType == MemoryOperationType::DummyRead;
		_readCounter++;
	}
	return value;
#else 
	ProcessPendingDma(addr);

	StartCpuCycle(true);
	uint8_t value = _memoryManager->Read(addr, operationType);
	EndCpuCycle(true);
	return value;
#endif
}

uint16_t NesCpu::FetchOperand()
{
	switch(_instAddrMode) {
		case AddrMode::Acc:
		case AddrMode::Imp: DummyRead(); return 0;
		case AddrMode::Imm:
		case AddrMode::Rel: return GetImmediate();
		case AddrMode::Zero: return GetZeroAddr();
		case AddrMode::ZeroX: return GetZeroXAddr();
		case AddrMode::ZeroY: return GetZeroYAddr();
		case AddrMode::Ind: return GetIndAddr();
		case AddrMode::IndX: return GetIndXAddr();
		case AddrMode::IndY: return GetIndYAddr(false);
		case AddrMode::IndYW: return GetIndYAddr(true);
		case AddrMode::Abs: return GetAbsAddr();
		case AddrMode::AbsX: return GetAbsXAddr(false);
		case AddrMode::AbsXW: return GetAbsXAddr(true);
		case AddrMode::AbsY: return GetAbsYAddr(false);
		case AddrMode::AbsYW: return GetAbsYAddr(true);
		default: break;
	}
	
#if !defined(LIBRETRO) && !defined(DUMMYCPU)
	if(_lastCrashWarning == 0 || _cycleCount - _lastCrashWarning > 5000000) {
		MessageManager::DisplayMessage("Error", "GameCrash", "Invalid OP code - CPU crashed.");
		_lastCrashWarning = _cycleCount;
	}

	if(_console->GetNesConfig().BreakOnCrash) {
		//When "Break on Crash" is enabled, open the debugger and break immediately if a crash occurs
		_console->GetEmulator()->GetDebugger(true)->BreakImmediately(BreakSource::BreakOnCpuCrash);
	}
	
	if(_console->IsNsf()) {
		//Don't stop emulation on CPU crash when playing NSFs, reset cpu instead
		_console->GetEmulator()->Reset();
		return 0;
	} else {
		return 0;
	}
#else 
	return 0;
#endif
}

void NesCpu::EndCpuCycle(bool forRead)
{
	_masterClock += forRead ? (_endClockCount + 1) : (_endClockCount - 1);
	_console->GetPpu()->Run(_masterClock - _ppuOffset);

	//"The internal signal goes high during φ1 of the cycle that follows the one where the edge is detected,
	//and stays high until the NMI has been handled. "
	_prevNeedNmi = _needNmi;

	//"This edge detector polls the status of the NMI line during φ2 of each CPU cycle (i.e., during the 
	//second half of each cycle) and raises an internal signal if the input goes from being high during 
	//one cycle to being low during the next"
	if(!_prevNmiFlag && _state.NMIFlag) {
		_needNmi = true;
	}
	_prevNmiFlag = _state.NMIFlag;

	//"it's really the status of the interrupt lines at the end of the second-to-last cycle that matters."
	//Keep the irq lines values from the previous cycle.  The before-to-last cycle's values will be used
	_prevRunIrq = _runIrq;
	_runIrq = ((_state.IRQFlag & _irqMask) > 0 && !CheckFlag(PSFlags::Interrupt));
}

void NesCpu::StartCpuCycle(bool forRead)
{
	_masterClock += forRead ? (_startClockCount - 1) : (_startClockCount + 1);
	_cycleCount++;
	_console->GetPpu()->Run(_masterClock - _ppuOffset);
	_console->ProcessCpuClock();
}

void NesCpu::ProcessPendingDma(uint16_t readAddress)
{
	if(!_needHalt) {
		return;
	}

	//"If this cycle is a read, hijack the read, discard the value, and prevent all other actions that occur on this cycle (PC not incremented, etc)"
	StartCpuCycle(true);
	_memoryManager->Read(readAddress, MemoryOperationType::DummyRead);
	EndCpuCycle(true);
	_needHalt = false;

	uint16_t spriteDmaCounter = 0;
	uint8_t spriteReadAddr = 0;
	uint8_t readValue = 0;
	bool skipDummyReads = (readAddress == 0x4016 || readAddress == 0x4017);

	auto processCycle = [this] {
		//Sprite DMA cycles count as halt/dummy cycles for the DMC DMA when both run at the same time
		if(_needHalt) {
			_needHalt = false;
		} else if(_needDummyRead) {
			_needDummyRead = false;
		}
		StartCpuCycle(true);
	};

	while(_dmcDmaRunning || _spriteDmaTransfer) {
		bool getCycle = (_cycleCount & 0x01) == 0;
		if(getCycle) {
			if(_dmcDmaRunning && !_needHalt && !_needDummyRead) {
				//DMC DMA is ready to read a byte (both halt and dummy read cycles were performed before this)
				processCycle();
				readValue = _memoryManager->Read(_console->GetApu()->GetDmcReadAddress(), MemoryOperationType::DmaRead);
				EndCpuCycle(true); 
				_console->GetApu()->SetDmcReadBuffer(readValue);
				_dmcDmaRunning = false;
			} else if(_spriteDmaTransfer) {
				//DMC DMA is not running, or not ready, run sprite DMA
				processCycle();
				readValue = _memoryManager->Read(_spriteDmaOffset * 0x100 + spriteReadAddr);
				EndCpuCycle(true);
				spriteReadAddr++;
				spriteDmaCounter++;
			} else {
				//DMC DMA is running, but not ready (need halt/dummy read) and sprite DMA isn't runnnig, perform a dummy read
				assert(_needHalt || _needDummyRead);
				processCycle();
				if(!skipDummyReads) {
					_memoryManager->Read(readAddress, MemoryOperationType::DummyRead);
				}
				EndCpuCycle(true);
			}
		} else {
			if(_spriteDmaTransfer && (spriteDmaCounter & 0x01)) {
				//Sprite DMA write cycle (only do this if a sprite dma read was performed last cycle)
				processCycle();
				_memoryManager->Write(0x2004, readValue, MemoryOperationType::Write);
				EndCpuCycle(true);
				spriteDmaCounter++;
				if(spriteDmaCounter == 0x200) {
					_spriteDmaTransfer = false;
				}
			} else {
				//Align to read cycle before starting sprite DMA (or align to perform DMC read)
				processCycle();
				if(!skipDummyReads) {
					_memoryManager->Read(readAddress, MemoryOperationType::DummyRead);
				}
				EndCpuCycle(true);
			}
		}
	}
}

void NesCpu::RunDMATransfer(uint8_t offsetValue)
{
	_spriteDmaTransfer = true;
	_spriteDmaOffset = offsetValue;
	_needHalt = true;
}

void NesCpu::StartDmcTransfer()
{
	_dmcDmaRunning = true;
	_needDummyRead = true;
	_needHalt = true;
}

uint32_t NesCpu::GetClockRate(NesModel model)
{
	switch(model) {
		default:
		case NesModel::NTSC: return NesCpu::ClockRateNtsc; break;
		case NesModel::PAL: return NesCpu::ClockRatePal; break;
		case NesModel::Dendy: return NesCpu::ClockRateDendy; break;
	}
}

void NesCpu::SetMasterClockDivider(NesModel region)
{
	switch(region) {
		default:
		case NesModel::NTSC:
			_startClockCount = 6;
			_endClockCount = 6;
			break;

		case NesModel::PAL:
			_startClockCount = 8;
			_endClockCount = 8;
			break;

		case NesModel::Dendy:
			_startClockCount = 7;
			_endClockCount = 8;
			break;
	}
}

void NesCpu::Serialize(Serializer &s)
{
	EmuSettings* settings = _console->GetSettings();
	EmulationConfig emuConfig = settings->GetEmulationConfig();
	uint32_t extraScanlinesBeforeNmi = emuConfig.PpuExtraScanlinesBeforeNmi;
	uint32_t extraScanlinesAfterNmi = emuConfig.PpuExtraScanlinesAfterNmi;
	uint32_t dipSwitches = _console->GetNesConfig().DipSwitches;

	s.Stream(_state.PC, _state.SP, _state.PS, _state.A, _state.X, _state.Y, _cycleCount, _state.NMIFlag, 
			_state.IRQFlag, _dmcDmaRunning, _spriteDmaTransfer,
			extraScanlinesBeforeNmi, extraScanlinesBeforeNmi, dipSwitches,
			_needDummyRead, _needHalt, _startClockCount, _endClockCount, _ppuOffset, _masterClock,
			_prevNeedNmi, _prevNmiFlag, _needNmi);

	if(!s.IsSaving()) {
		//TODO
		//settings->SetPpuNmiConfig(extraScanlinesBeforeNmi, extraScanlinesAfterNmi);
		//settings->SetDipSwitches(dipSwitches);
	}
}