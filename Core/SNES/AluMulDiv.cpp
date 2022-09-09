#include "pch.h"
#include "SNES/AluMulDiv.h"
#include "SNES/SnesCpu.h"
#include "SNES/InternalRegisterTypes.h"
#include "Utilities/Serializer.h"

void AluMulDiv::Initialize(SnesCpu* cpu)
{
	_cpu = cpu;

	_state = {};
	_state.MultOperand1 = 0xFF;
	_state.Dividend = 0xFFFF;
}

void AluMulDiv::Run(bool isRead)
{
	uint64_t cpuCycle = _cpu->GetCycleCount();

	if(isRead) {
		//Run 1 cycle less for read operations, since they occur earlier within the CPU cycle, compared to a write
		cpuCycle--;
	}

	if(_multCounter != 0 || _divCounter != 0) {
		uint64_t cyclesToRun = cpuCycle - _prevCpuCycle;
		while(cyclesToRun--) {
			if(!_multCounter && !_divCounter) {
				break;
			}

			if(_multCounter > 0) {
				_multCounter--;

				if(_state.DivResult & 0x01) {
					_state.MultOrRemainderResult += _shift;
				}

				_shift <<= 1;
				_state.DivResult >>= 1;
			}

			if(_divCounter > 0) {
				_divCounter--;
				_shift >>= 1;
				_state.DivResult <<= 1;

				if(_state.MultOrRemainderResult >= _shift) {
					_state.MultOrRemainderResult -= _shift;
					_state.DivResult |= 1;
				}
			}
		}
	}
	
	_prevCpuCycle = cpuCycle;
}

uint8_t AluMulDiv::Read(uint16_t addr)
{
	Run(true);
	return Peek(addr);
}

uint8_t AluMulDiv::Peek(uint16_t addr)
{
	switch(addr) {
		case 0x4214: return (uint8_t)_state.DivResult;
		case 0x4215: return (uint8_t)(_state.DivResult >> 8);

		case 0x4216: return (uint8_t)_state.MultOrRemainderResult;
		case 0x4217: return (uint8_t)(_state.MultOrRemainderResult >> 8);
	}

	throw std::runtime_error("ALU: invalid address");
}

void AluMulDiv::Write(uint16_t addr, uint8_t value)
{
	Run(false);

	switch(addr) {
		case 0x4202: _state.MultOperand1 = value; break;
		case 0x4203:
			_state.MultOrRemainderResult = 0;
			if(!_divCounter && !_multCounter) {
				_multCounter = 8;

				_state.MultOperand2 = value;
				_state.DivResult = (value << 8) | _state.MultOperand1;
				_shift = value;
			}
			break;

		case 0x4204: _state.Dividend = (_state.Dividend & 0xFF00) | value; break;
		case 0x4205: _state.Dividend = (_state.Dividend & 0xFF) | (value << 8); break;
		
		case 0x4206:
			_state.MultOrRemainderResult = _state.Dividend;

			if(!_divCounter && !_multCounter) {
				_divCounter = 16;
				_state.Divisor = value;
				_shift = (value << 16);
			}
			break;

		default: throw std::runtime_error("ALU: invalid address");
	}
}

AluState AluMulDiv::GetState()
{
	return _state;
}

void AluMulDiv::Serialize(Serializer &s)
{
	SV(_state.MultOperand1); SV(_state.MultOperand2); SV(_state.MultOrRemainderResult); SV(_state.Dividend); SV(_state.Divisor); SV(_state.DivResult);
	SV(_divCounter); SV(_multCounter); SV(_shift); SV(_prevCpuCycle);
}
