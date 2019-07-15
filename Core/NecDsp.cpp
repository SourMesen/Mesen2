#include "stdafx.h"
#include "NecDsp.h"
#include "MemoryManager.h"
#include "Console.h"
#include "BaseCartridge.h"
#include "CartTypes.h"
#include "MessageManager.h"
#include "../Utilities/FolderUtilities.h"

NecDsp::NecDsp(Console* console, vector<uint8_t> &biosRom, uint16_t registerMask)
{
	_console = console;
	_memoryManager = console->GetMemoryManager().get();
	_registerMask = registerMask;

	for(int i = 0; i < 2048; i++) {
		_progRom[i] = biosRom[i*3] | (biosRom[i*3 + 1] << 8) | (biosRom[i*3 + 2] << 16);
	}
	for(int i = 0; i < 1024; i++) {
		_dataRom[i] = biosRom[2048*3 + i*2] | (biosRom[2048*3 + i*2 + 1] << 8);
	}
}

bool NecDsp::LoadBios(string combinedFilename, string splitFilenameProgram, string splitFilenameData, vector<uint8_t> &biosData)
{
	VirtualFile combinedBios(FolderUtilities::CombinePath(FolderUtilities::GetBiosFolder(), combinedFilename));
	if(combinedBios.GetSize() == 0x2000) {
		combinedBios.ReadFile(biosData);
		return true;
	} else {
		VirtualFile splitBiosA(FolderUtilities::CombinePath(FolderUtilities::GetBiosFolder(), splitFilenameProgram));
		VirtualFile splitBiosB(FolderUtilities::CombinePath(FolderUtilities::GetBiosFolder(), splitFilenameData));

		if(splitBiosA.GetSize() == 0x1800 && splitBiosB.GetSize() == 0x800) {
			splitBiosA.ReadFile(biosData);
			
			vector<uint8_t> splitData;
			splitBiosB.ReadFile(splitData);
			
			biosData.insert(biosData.end(), splitData.begin(), splitData.end());
			return true;
		}
	}

	MessageManager::DisplayMessage("Error", "Could not find BIOS file for DSP: " + combinedFilename);
	return false;
}

NecDsp* NecDsp::InitCoprocessor(CoprocessorType type, Console *console)
{
	bool biosLoaded = false;
	vector<uint8_t> biosData;
	switch(type) {
		case CoprocessorType::DSP1: biosLoaded = LoadBios("dsp1.rom", "dsp1.program.rom", "dsp1.data.rom", biosData); break;
		case CoprocessorType::DSP1B: biosLoaded = LoadBios("dsp1b.rom", "dsp1b.program.rom", "dsp1b.data.rom", biosData); break;
		case CoprocessorType::DSP2: biosLoaded = LoadBios("dsp2.rom", "dsp2.program.rom", "dsp2.data.rom", biosData); break;
		case CoprocessorType::DSP3: biosLoaded = LoadBios("dsp3.rom", "dsp3.program.rom", "dsp3.data.rom", biosData); break;
		case CoprocessorType::DSP4: biosLoaded = LoadBios("dsp4.rom", "dsp4.program.rom", "dsp4.data.rom", biosData); break;
	}

	if(!biosLoaded) {
		return nullptr;
	}

	NecDsp* dsp = nullptr;
	MemoryManager* mm = console->GetMemoryManager().get();
	if(console->GetCartridge()->GetCartFlags() & CartFlags::LoRom) {
		dsp = new NecDsp(console, biosData, 0x4000);
		mm->RegisterHandler(0x30, 0x3F, 0x8000, 0xFFFF, dsp);
		mm->RegisterHandler(0xB0, 0xBF, 0x8000, 0xFFFF, dsp);
	} else if(console->GetCartridge()->GetCartFlags() & CartFlags::HiRom) {
		dsp = new NecDsp(console, biosData, 0x1000);
		mm->RegisterHandler(0x00, 0x1F, 0x6000, 0x7FFF, dsp);
		mm->RegisterHandler(0x80, 0x9F, 0x6000, 0x7FFF, dsp);
	}
	return dsp;
}

void NecDsp::Reset()
{
	_cycleCount = 0;
	_state = {};
}

void NecDsp::Run()
{
	uint64_t targetCycle = (uint64_t)(_memoryManager->GetMasterClock() * ((double)NecDsp::DspClockRate / _console->GetMasterClockRate()));

	if(_inRqmLoop) {
		_cycleCount = targetCycle;
		_inRqmLoop = false;
		return;
	}

	while(_cycleCount < targetCycle) {
		_opCode = _progRom[_state.PC & 0x7FF];
		_console->ProcessNecDspExec(_state.PC, _opCode);
		_state.PC++;

		switch(_opCode & 0xC00000) {
			case 0x000000: ExecOp(); break;
			case 0x400000: ExecAndReturn(); break;
			case 0x800000: Jump(); break;
			case 0xC00000: Load(_opCode & 0x0F, (uint16_t)(_opCode >> 6)); break;
		}

		//Store the multiplication's result
		int32_t multResult = (int16_t)_state.K * (int16_t)_state.L;
		_state.M = multResult >> 15;
		_state.N = multResult << 1;

		_cycleCount++;
	}
}

uint8_t NecDsp::Read(uint32_t addr)
{
	Run();
	_inRqmLoop = false;
	if(addr & _registerMask) {
		return (_state.SR >> 8);
	} else {
		//DR
		if(_state.SR & NecDspStatusFlags::DataRegControl) {
			//8 bits
			_state.SR &= ~NecDspStatusFlags::RequestForMaster;
			return (uint8_t)_state.DR;
		} else {
			//16 bits
			if(_state.SR & NecDspStatusFlags::DataRegStatus) {
				_state.SR &= ~NecDspStatusFlags::RequestForMaster;
				_state.SR &= ~NecDspStatusFlags::DataRegStatus;
				return _state.DR >> 8;
			} else {
				_state.SR |= NecDspStatusFlags::DataRegStatus;
				return (uint8_t)_state.DR;
			}
		}
	}
}

void NecDsp::Write(uint32_t addr, uint8_t value)
{
	Run();
	_inRqmLoop = false;
	if(addr & _registerMask) {
		//SR
	} else {
		//DR
		if(_state.SR & NecDspStatusFlags::DataRegControl) {
			//8 bits
			_state.SR &= ~NecDspStatusFlags::RequestForMaster;
			_state.DR = (_state.DR & 0xFF00) | value;
		} else {
			//16 bits
			if(_state.SR & NecDspStatusFlags::DataRegStatus) {
				_state.SR &= ~NecDspStatusFlags::RequestForMaster;
				_state.SR &= ~NecDspStatusFlags::DataRegStatus;
				_state.DR = (_state.DR & 0xFF) | (value << 8);
			} else {
				_state.SR |= NecDspStatusFlags::DataRegStatus;
				_state.DR = (_state.DR & 0xFF00) | value;
			}
		}
	}
}

uint8_t NecDsp::Peek(uint32_t addr)
{
	return 0;
}

void NecDsp::PeekBlock(uint8_t *output)
{
	memset(output, 0, 0x1000);
}

AddressInfo NecDsp::GetAbsoluteAddress(uint32_t address)
{
	return { -1, SnesMemoryType::CpuMemory };
}

void NecDsp::RunApuOp(uint8_t aluOperation, uint16_t source)
{
	uint16_t result = 0;

	//Select the accumulator/flags for the operation
	uint8_t accSelect = (_opCode >> 15) & 0x01;
	NecDspAccFlags flags = accSelect ? _state.FlagsB : _state.FlagsA;
	uint16_t acc = accSelect ? _state.B : _state.A;
	uint8_t otherCarry = accSelect ? _state.FlagsA.Carry : _state.FlagsB.Carry;

	//Select the 2nd operand for the operation
	uint8_t pSelect = (_opCode >> 20) & 0x03;
	uint16_t p;
	switch(pSelect) {
		case 0: p = _ram[_state.DP & 0xFF]; break;
		case 1: p = source; break;
		case 2: p = _state.M; break;
		case 3: p = _state.N; break;
	}

	//Perform the ALU operation, and set flags
	switch(aluOperation) {
		case 0x00: break;
		case 0x01: result = acc | p; break;
		case 0x02: result = acc & p; break;
		case 0x03: result = acc ^ p; break;
		case 0x04: result = acc - p; break;
		case 0x05: result = acc + p; break;
		case 0x06: result = acc - p - otherCarry; break;
		case 0x07: result = acc + p + otherCarry; break;
		case 0x08: result = acc - 1; p = 1; break;
		case 0x09: result = acc + 1; p = 1; break;

		case 0x0A: result = ~acc; break;
		case 0x0B: result = (acc >> 1) | (acc & 0x8000); break;
		case 0x0C: result = (acc << 1) | (uint8_t)otherCarry; break;
		case 0x0D: result = (acc << 2) | 0x03; break;
		case 0x0E: result = (acc << 4) | 0x0F; break;
		case 0x0F: result = (acc << 8) | (acc >> 8); break;
	}

	flags.Zero = result == 0;
	flags.Sign0 = (result & 0x8000) >> 15;
	if(!flags.Overflow1) {
		flags.Sign1 = flags.Sign0;
	}

	switch(aluOperation) {
		case 0x00: case 0x01: case 0x02: case 0x03:
		case 0x0A: case 0x0D: case 0x0E: case 0x0F:
			flags.Carry = false;
			flags.Overflow0 = false;
			flags.Overflow1 = false;
			break;

		case 0x04: case 0x05: case 0x06: case 0x07: case 0x08: case 0x09: {
			uint16_t overflow = (acc ^ result) & (p ^ ((aluOperation & 0x01) ? result : acc));
			flags.Overflow0 = (bool)((overflow & 0x8000) >> 15);
			if(flags.Overflow0 && flags.Overflow1) {
				flags.Overflow1 = flags.Sign0 == flags.Sign1;
			} else {
				flags.Overflow1 |= flags.Overflow0;
			}
			flags.Carry = (bool)(((acc ^ p ^ result ^ overflow) & 0x8000) >> 15);
			break;
		}

		case 0x0B:
			flags.Carry = (bool)(acc & 0x01);
			flags.Overflow0 = false;
			flags.Overflow1 = false;
			break;

		case 0x0C:
			flags.Carry = (bool)((acc >> 15) & 0x01);
			flags.Overflow0 = false;
			flags.Overflow1 = false;
			break;
	}

	//Update selected accumulator/flags with the operation's results
	if(accSelect) {
		_state.B = result;
		_state.FlagsB = flags;
	} else {
		_state.A = result;
		_state.FlagsA = flags;
	}
}

void NecDsp::UpdateDataPointer()
{
	uint16_t dp = _state.DP;
	switch((_opCode >> 13) & 0x03) {
		case 0: break; //NOP
		case 1: dp = (dp & 0xF0) | ((dp + 1) & 0x0F); break; //Increment lower nibble, with no carry
		case 2: dp = (dp & 0xF0) | ((dp - 1) & 0x0F); break; //Decrement lower  nibble, with no carry
		case 3: dp &= 0xF0; break; //Clear lower nibble
	}

	uint8_t dpHighModify = (_opCode >> 9) & 0x0F;
	_state.DP = dp ^ (dpHighModify << 4);
}

void NecDsp::ExecOp()
{
	uint8_t aluOperation = (_opCode >> 16) & 0x0F;
	uint16_t source = GetSourceValue((_opCode >> 4) & 0x0F);

	//First, process the ALU operation, if needed
	if(aluOperation) {
		RunApuOp(aluOperation, source);
	}

	//Then transfer data from source to destination
	uint8_t dest = _opCode & 0x0F;
	Load(dest, source);

	if(dest != 0x04) {
		//Destination was not the data pointer (DP), update it
		UpdateDataPointer();
	}

	uint8_t rpDecrement = (_opCode >> 8) & 0x01;
	if(rpDecrement && dest != 0x05) {
		//Destination was not the rom pointer (RP), decrement it
		_state.RP--;
	}
}

void NecDsp::ExecAndReturn()
{
	ExecOp();
	_state.SP = (_state.SP - 1) & 0x03;
	_state.PC = _stack[_state.SP];	
}

void NecDsp::Jump()
{
	uint8_t bank = _opCode & 0x03;
	uint16_t address = (_opCode >> 2) & 0x7FF;
	uint16_t target = (_state.PC & 0x2000) | (bank << 11) | address;
	uint32_t jmpCond = 0;

	uint16_t jmpType = (_opCode >> 13) & 0x1FF;
	switch(jmpType) {
		case 0x00: _state.PC = _state.SerialOut; break;

		case 0x80: jmpCond = !_state.FlagsA.Carry; break;
		case 0x82: jmpCond = _state.FlagsA.Carry; break;
		case 0x84: jmpCond = !_state.FlagsB.Carry; break;
		case 0x86: jmpCond = _state.FlagsB.Carry; break;

		case 0x88: jmpCond = !_state.FlagsA.Zero; break;
		case 0x8A: jmpCond = _state.FlagsA.Zero; break;
		case 0x8C: jmpCond = !_state.FlagsB.Zero; break;
		case 0x8E: jmpCond = _state.FlagsB.Zero; break;

		case 0x90: jmpCond = !_state.FlagsA.Overflow0; break;
		case 0x92: jmpCond = _state.FlagsA.Overflow0; break;
		case 0x94: jmpCond = !_state.FlagsB.Overflow0; break;
		case 0x96: jmpCond = _state.FlagsB.Overflow0; break;
		case 0x98: jmpCond = !_state.FlagsA.Overflow1; break;
		case 0x9A: jmpCond = _state.FlagsA.Overflow1; break;
		case 0x9C: jmpCond = !_state.FlagsB.Overflow1; break;
		case 0x9E: jmpCond = _state.FlagsB.Overflow1; break;

		case 0xA0: jmpCond = !_state.FlagsA.Sign0; break;
		case 0xA2: jmpCond = _state.FlagsA.Sign0; break;
		case 0xA4: jmpCond = !_state.FlagsB.Sign0; break;
		case 0xA6: jmpCond = _state.FlagsB.Sign0; break;
		case 0xA8: jmpCond = !_state.FlagsA.Sign1; break;
		case 0xAA: jmpCond = _state.FlagsA.Sign1; break;
		case 0xAC: jmpCond = !_state.FlagsB.Sign1; break;
		case 0xAE: jmpCond = _state.FlagsB.Sign1; break;

		case 0xB0: jmpCond = !(_state.DP & 0x0F); break;
		case 0xB1: jmpCond = _state.DP & 0x0F; break;
		case 0xB2: jmpCond = (_state.DP & 0x0F) == 0x0F; break;
		case 0xB3: jmpCond = (_state.DP & 0x0F) != 0x0F; break;

		case 0xB4: jmpCond = !(_state.SR & NecDspStatusFlags::SerialInControl); break;
		case 0xB6: jmpCond = _state.SR & NecDspStatusFlags::SerialInControl; break;
		case 0xB8: jmpCond = !(_state.SR & NecDspStatusFlags::SerialOutControl); break;
		case 0xBA: jmpCond = _state.SR & NecDspStatusFlags::SerialOutControl; break;
		case 0xBC: jmpCond = !(_state.SR & NecDspStatusFlags::RequestForMaster); break;
		case 0xBE: jmpCond = _state.SR & NecDspStatusFlags::RequestForMaster; break;

		case 0x100: _state.PC = target & ~0x2000; break;
		case 0x101: _state.PC = target | 0x2000; break;

		case 0x140:
			_stack[_state.SP] = _state.PC;
			_state.SP = (_state.SP + 1) & 0x03;
			_state.PC = target & ~0x2000;
			break;

		case 0x141:
			_stack[_state.SP] = _state.PC;
			_state.SP = (_state.SP + 1) & 0x03;
			_state.PC = target | 0x2000;
			break;
	}

	if(jmpCond) {
		if((_state.PC - 1 == target) && (jmpType == 0xBC || jmpType == 0xBE)) {
			//CPU is in a wait loop for RQM, skip emulation until the CPU reads/writes from the IO registers
			_inRqmLoop = true;
		}
		_state.PC = target;
	}
}

void NecDsp::Load(uint8_t dest, uint16_t value)
{
	switch(dest) {
		case 0x00: break;
		case 0x01: _state.A = value; break;
		case 0x02: _state.B = value; break;
		case 0x03: _state.TR = value; break;
		case 0x04: _state.DP = value; break;
		case 0x05: _state.RP = value; break;

		case 0x06:
			_state.DR = value;
			_state.SR |= NecDspStatusFlags::RequestForMaster;
			break;

		case 0x07:
			_state.SR = (_state.SR & 0x907C) | (value & ~0x907C);
			break;

		case 0x08: _state.SerialOut = value; break;
		case 0x09: _state.SerialOut = value; break;
		case 0x0A: _state.K = value; break;

		case 0x0B:
			_state.K = value;
			_state.L = _dataRom[_state.RP & 0x3FF];
			break;

		case 0x0C:
			_state.L = value;
			_state.K = _ram[(_state.DP | 0x40) & 0xFF];
			break;

		case 0x0D: _state.L = value; break;
		case 0x0E: _state.TRB = value; break;
		case 0x0F: _ram[_state.DP & 0xFF] = value; break;

		default:
			throw std::runtime_error("DSP-1: invalid destination");
	}
}

uint16_t NecDsp::GetSourceValue(uint8_t source)
{
	switch(source) {
		case 0x00: return _state.TRB;
		case 0x01: return _state.A;
		case 0x02: return _state.B;
		case 0x03: return _state.TR;
		case 0x04: return _state.DP;
		case 0x05: return _state.RP;
		case 0x06: return _dataRom[_state.RP & 0x3FF];
		case 0x07: return 0x8000 - _state.FlagsA.Sign1;

		case 0x08:
			_state.SR |= NecDspStatusFlags::RequestForMaster;
			return _state.DR;

		case 0x09: return _state.DR;
		case 0x0A: return _state.SR;
		case 0x0B: return _state.SerialIn;
		case 0x0C: return _state.SerialIn;
		case 0x0D: return _state.K;
		case 0x0E: return _state.L;
		case 0x0F: return _ram[_state.DP & 0xFF];
	}
	throw std::runtime_error("DSP-1: invalid source");
}

uint8_t* NecDsp::DebugGetProgramRom()
{
	return (uint8_t*)_progRom;
}

uint8_t* NecDsp::DebugGetDataRom()
{
	return (uint8_t*)_dataRom;
}

uint8_t* NecDsp::DebugGetDataRam()
{
	return (uint8_t*)_ram;
}

uint32_t NecDsp::DebugGetProgramRomSize()
{
	return 2048 * 4;
}

uint32_t NecDsp::DebugGetDataRomSize()
{
	return 1024 * 2;
}

uint32_t NecDsp::DebugGetDataRamSize()
{
	return 256 * 2;
}

NecDspState NecDsp::GetState()
{
	return _state;
}

void NecDsp::Serialize(Serializer &s)
{
	s.Stream(
		_state.A, _state.B, _state.DP, _state.DR, _state.K, _state.L, _state.M, _state.N, _state.PC,
		_state.RP, _state.SerialIn, _state.SerialOut, _state.SP, _state.SR, _state.TR, _state.TRB, 
		_state.FlagsA.Carry, _state.FlagsA.Overflow0, _state.FlagsA.Overflow1, _state.FlagsA.Sign0, _state.FlagsA.Sign1, _state.FlagsA.Zero,
		_state.FlagsB.Carry, _state.FlagsB.Overflow0, _state.FlagsB.Overflow1, _state.FlagsB.Sign0, _state.FlagsB.Sign1, _state.FlagsB.Zero
	);

	s.Stream(_opCode, _cycleCount, _inRqmLoop);
	s.StreamArray<uint16_t>(_ram, 256);
	s.StreamArray<uint16_t>(_stack, 4);
}

