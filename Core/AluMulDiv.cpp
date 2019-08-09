#include "stdafx.h"
#include "AluMulDiv.h"
#include "Cpu.h"
#include "../Utilities/Serializer.h"

void AluMulDiv::Initialize(Cpu* cpu)
{
	_cpu = cpu;
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

				if(_divResult & 0x01) {
					_multOrRemainderResult += _shift;
				}

				_shift <<= 1;
				_divResult >>= 1;
			}

			if(_divCounter > 0) {
				_divCounter--;
				_shift >>= 1;
				_divResult <<= 1;

				if(_multOrRemainderResult >= _shift) {
					_multOrRemainderResult -= _shift;
					_divResult |= 1;
				}
			}
		}
	}
	
	_prevCpuCycle = cpuCycle;
}

uint8_t AluMulDiv::Read(uint16_t addr)
{
	Run(true);

	switch(addr) {
		case 0x4214: return (uint8_t)_divResult;
		case 0x4215: return (uint8_t)(_divResult >> 8);

		case 0x4216: return (uint8_t)_multOrRemainderResult;
		case 0x4217: return (uint8_t)(_multOrRemainderResult >> 8);
	}

	throw std::runtime_error("ALU: invalid address");
}

void AluMulDiv::Write(uint16_t addr, uint8_t value)
{
	Run(false);

	switch(addr) {
		case 0x4202: _multOperand1 = value; break;
		case 0x4203:
			_multOrRemainderResult = 0;
			if(!_divCounter && !_multCounter) {
				_multCounter = 8;

				_multOperand2 = value;
				_divResult = (value << 8) | _multOperand1;
				_shift = value;
			}
			break;

		case 0x4204: _dividend = (_dividend & 0xFF00) | value; break;
		case 0x4205: _dividend = (_dividend & 0xFF) | (value << 8); break;
		
		case 0x4206:
			_multOrRemainderResult = _dividend;

			if(!_divCounter && !_multCounter) {
				_divCounter = 16;
				_divisor = value;
				_shift = (value << 16);
			}
			break;

		default: throw std::runtime_error("ALU: invalid address");
	}
}

void AluMulDiv::Serialize(Serializer &s)
{
	s.Stream(
		_multOperand1, _multOperand2, _multOrRemainderResult, _dividend, _divisor, _divResult,
		_divCounter, _multCounter, _shift, _prevCpuCycle
	);
}
