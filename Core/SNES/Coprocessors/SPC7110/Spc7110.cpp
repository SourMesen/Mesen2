#include "pch.h"
#include "SNES/Coprocessors/SPC7110/Spc7110.h"
#include "SNES/Coprocessors/SPC7110/Spc7110Decomp.h"
#include "SNES/MemoryMappings.h"
#include "SNES/SnesConsole.h"
#include "SNES/BaseCartridge.h"
#include "SNES/SnesMemoryManager.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/BatteryManager.h"
#include "Shared/MessageManager.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/Serializer.h"

Spc7110::Spc7110(SnesConsole* console, bool useRtc)
{
	_console = console;
	_emu = console->GetEmulator();
	_cart = console->GetCartridge();
	_useRtc = useRtc;

	MemoryMappings* mappings = console->GetMemoryManager()->GetMemoryMappings();
	vector<unique_ptr<IMemoryHandler>>& prgRomHandlers = _cart->GetPrgRomHandlers();
	vector<unique_ptr<IMemoryHandler>>& saveRamHandlers = _cart->GetSaveRamHandlers();

	//Regular A Bus register handler, keep a reference to it, it'll be overwritten below
	_cpuRegisterHandler = mappings->GetHandler(0x4000);

	//SPC7110 registers (0x4800-0x4842)
	mappings->RegisterHandler(0x00, 0x3F, 0x4000, 0x4FFF, this);
	mappings->RegisterHandler(0x80, 0xBF, 0x4000, 0x4FFF, this);
	mappings->RegisterHandler(0x50, 0x50, 0x0000, 0xFFFF, this);
	mappings->RegisterHandler(0x58, 0x58, 0x0000, 0xFFFF, this);

	//SRAM
	mappings->RegisterHandler(0x00, 0x3F, 0x6000, 0x7FFF, saveRamHandlers);
	mappings->RegisterHandler(0x80, 0xBF, 0x6000, 0x7FFF, saveRamHandlers);

	//PRG
	mappings->RegisterHandler(0x00, 0x3F, 0x8000, 0xFFFF, prgRomHandlers, 8);
	mappings->RegisterHandler(0x80, 0xBF, 0x8000, 0xFFFF, prgRomHandlers, 8);

	bool enableStrictBoardMappings = _emu->GetSettings()->GetSnesConfig().EnableStrictBoardMappings;
	uint32_t romSize = _cart->DebugGetPrgRomSize();
	if(!enableStrictBoardMappings && _cart->DebugGetPrgRomSize() >= 0x600000) {
		mappings->RegisterHandler(0x40, 0x4F, 0x0000, 0xFFFF, prgRomHandlers, 0, 0x600);
		_realDataRomSize = romSize > 0x200000 ? romSize - 0x200000 : 0;
	} else {
		_realDataRomSize = romSize > 0x100000 ? romSize - 0x100000 : 0;
	}
	mappings->RegisterHandler(0xC0, 0xCF, 0x0000, 0xFFFF, prgRomHandlers);

	Reset();
}

void Spc7110::Serialize(Serializer& s)
{
	SVArray(_decompBuffer, 32);
	SVArray(_dataRomBanks, 3);

	SV(_directoryBase); SV(_directoryIndex); SV(_targetOffset); SV(_dataLengthCounter); SV(_skipBytes); SV(_decompFlags); SV(_decompMode); SV(_srcAddress); SV(_decompOffset); SV(_decompStatus);
	SV(_dividend); SV(_multiplier); SV(_divisor); SV(_multDivResult); SV(_remainder); SV(_aluState); SV(_aluFlags); SV(_sramEnabled); SV(_dataRomSize); SV(_readBase);
	SV(_readOffset); SV(_readStep); SV(_readMode); SV(_readBuffer);

	SV(_decomp);
	if(_rtc) {
		SV(_rtc);
	}

	if(!s.IsSaving()) {
		UpdateMappings();
	}
}

uint8_t Spc7110::Read(uint32_t addr)
{
	if((addr & 0xFF0000) == 0x500000) {
		addr = 0x4800;
	} else if((addr & 0xFF0000) == 0x580000) {
		addr = 0x4808;
	}

	switch(addr & 0xFFFF) {
		//Decompression
		case 0x4800:
			_dataLengthCounter--;
			return ReadDecompressedByte();

		case 0x4801: return (_directoryBase) & 0xFF;
		case 0x4802: return (_directoryBase >> 8) & 0xFF;
		case 0x4803: return (_directoryBase >> 16) & 0xFF;
		case 0x4804: return _directoryIndex;
		case 0x4805: return (_targetOffset) & 0xFF;
		case 0x4806: return (_targetOffset >> 8) & 0xFF;
		case 0x4807: return _skipBytes;
		case 0x4808: return 0;
		case 0x4809: return (_dataLengthCounter & 0xFF);
		case 0x480A: return (_dataLengthCounter >> 8) & 0xFF;
		case 0x480B: return _decompFlags;
		case 0x480C: return _decompStatus;

		//Data read
		case 0x4810: {
			uint8_t value = _readBuffer;
			IncrementPosition4810();
			return value;
		}

		case 0x4811: return _readBase & 0xFF;
		case 0x4812: return (_readBase >> 8) & 0xFF;
		case 0x4813: return (_readBase >> 16) & 0xFF;
		case 0x4814: return _readOffset & 0xFF;
		case 0x4815: return (_readOffset >> 8) & 0xFF;
		case 0x4816: return _readStep & 0xFF;
		case 0x4817: return (_readStep >> 8) & 0xFF;
		case 0x4818: return _readMode;

		case 0x481A:
			if((_readMode & 0x60) == 0x60) {
				IncrementPosition();
			}
			return 0;

		//ALU
		case 0x4820: return _dividend & 0xFF;
		case 0x4821: return (_dividend >> 8) & 0xFF;
		case 0x4822: return (_dividend >> 16) & 0xFF;
		case 0x4823: return (_dividend >> 24) & 0xFF;
		case 0x4824: return _multiplier & 0xFF;
		case 0x4825: return (_multiplier >> 8) & 0xFF;
		case 0x4826: return _divisor & 0xFF;
		case 0x4827: return (_divisor >> 8) & 0xFF;
		case 0x4828: return _multDivResult & 0xFF;
		case 0x4829: return (_multDivResult >> 8) & 0xFF;
		case 0x482A: return (_multDivResult >> 16) & 0xFF;
		case 0x482B: return (_multDivResult >> 24) & 0xFF;
		case 0x482C: return _remainder & 0xFF;
		case 0x482D: return (_remainder >> 8) & 0xFF;
		case 0x482E: return _aluFlags;
		case 0x482F: return _aluState;

		case 0x4830: return _sramEnabled;
		case 0x4831: return _dataRomBanks[0];
		case 0x4832: return _dataRomBanks[1];
		case 0x4833: return _dataRomBanks[2];
		case 0x4834: return _dataRomSize;

		case 0x4840:
		case 0x4841:
		case 0x4842:
			return _rtc ? _rtc->Read(addr) : 0;

		default:
			if((addr & 0xFFFF) >= 0x4800) {
				LogDebug("[Debug] Missing read handler: $" + HexUtilities::ToHex(addr & 0xFFFF));
			}
			return _cpuRegisterHandler->Read(addr);
	}
}

void Spc7110::Write(uint32_t addr, uint8_t value)
{
	if((addr & 0xFF0000) == 0x500000) {
		addr = 0x4800;
	} else if((addr & 0xFF0000) == 0x580000) {
		addr = 0x4808;
	}

	switch(addr & 0xFFFF) {
		//Data ROM Decompression (4800-480C)
		case 0x4801: _directoryBase = (_directoryBase & 0xFFFF00) | value; break;
		case 0x4802: _directoryBase = (_directoryBase & 0xFF00FF) | (value << 8); break;
		case 0x4803: _directoryBase = (_directoryBase & 0x00FFFF) | (value << 16); break;
		case 0x4804:
			_directoryIndex = value; 
			LoadEntryHeader();
			break;

		case 0x4805: _targetOffset = (_targetOffset & 0xFF00) | value; break;
		case 0x4806: 
			_targetOffset = (_targetOffset & 0x00FF) | (value << 8);
			BeginDecompression();
			break;

		case 0x4807: _skipBytes = value; break;
		case 0x4808: break;

		case 0x4809: _dataLengthCounter = (_dataLengthCounter & 0xFF00) | value; break;
		case 0x480A: _dataLengthCounter = (_dataLengthCounter & 0x00FF) | (value << 8); break;
		case 0x480B: _decompFlags = value & 0x03; break;

		//Direct Data ROM Access (4810-481A)
		case 0x4811: _readBase = (_readBase & 0xFFFF00) | value; break;
		case 0x4812: _readBase = (_readBase & 0xFF00FF) | (value << 8); break;
		case 0x4813: 
			_readBase = (_readBase & 0x00FFFF) | (value << 16);
			FillReadBuffer();
			break;

		case 0x4814:
			_readOffset = (_readOffset & 0xFF00) | value;
			if((_readMode & 0x60) == 0x20) {
				IncrementPosition();
			}
			break;

		case 0x4815:
			_readOffset = (_readOffset & 0x00FF) | (value << 8);
			if(_readMode & 0x02) {
				FillReadBuffer();
			}
			if((_readMode & 0x60) == 0x40) {
				IncrementPosition();
			}
			break;

		case 0x4816: _readStep = (_readStep & 0xFF00) | value; break;
		case 0x4817: _readStep = (_readStep & 0x00FF) | (value << 8); break;

		case 0x4818: 
			_readMode = value & 0x7F;
			FillReadBuffer();
			break;

		//ALU (4820-482F)
		case 0x4820: _dividend = (_dividend & 0xFFFFFF00) | value; break;
		case 0x4821: _dividend = (_dividend & 0xFFFF00FF) | (value << 8); break;
		case 0x4822: _dividend = (_dividend & 0xFF00FFFF) | (value << 16); break;
		case 0x4823: _dividend = (_dividend & 0x00FFFFFF) | (value << 24); break;
		case 0x4824: _multiplier = (_multiplier & 0xFF00) | value; break;
		case 0x4825: 
			_multiplier = (_multiplier & 0x00FF) | (value << 8);
			ProcessMultiplication(); 
			break;

		case 0x4826: _divisor = (_divisor & 0xFF00) | value; break;
		case 0x4827:
			_divisor = (_divisor & 0x00FF) | (value << 8);
			ProcessDivision();
			break;

		case 0x482E:  _aluFlags = value & 0x01; break; 

		//Memory mapping (4830-4834)
		case 0x4830: _sramEnabled = value & 0x87; break;
		case 0x4831: _dataRomBanks[0] = value & 0x07; UpdateMappings(); break;
		case 0x4832: _dataRomBanks[1] = value & 0x07; UpdateMappings(); break;
		case 0x4833: _dataRomBanks[2] = value & 0x07; UpdateMappings(); break;
		case 0x4834: _dataRomSize = value & 0x07; break;

		//RTC (4840-4842)
		case 0x4840:
		case 0x4841:
		case 0x4842:
			if(_rtc) {
				_rtc->Write(addr, value);
			}
			break;

		default: 
			if((addr & 0xFFFF) >= 0x4800) {
				LogDebug("[Debug] Missing write handler: $" + HexUtilities::ToHex(addr & 0xFFFF));
			}
			_cpuRegisterHandler->Write(addr, value);
			break;
	}
}

void Spc7110::UpdateMappings()
{
	MemoryMappings* mappings = _console->GetMemoryManager()->GetMemoryMappings();
	vector<unique_ptr<IMemoryHandler>>& prgRomHandlers = _cart->GetPrgRomHandlers();

	uint32_t dataRomSize = _realDataRomSize >> 12;
	if(dataRomSize > 0) {
		for(int i = 0; i < 3; i++) {
			mappings->RegisterHandler(0xD0 + (i * 0x10), 0xDF + (i * 0x10), 0x0000, 0xFFFF, prgRomHandlers, 0, 0x100 + ((_dataRomBanks[i] * 0x100) % dataRomSize));
		}
	}
}

void Spc7110::ProcessMultiplication()
{
	if(_aluFlags & 0x01) {
		//Signed multiplication (16x16=32)
		_multDivResult = (int32_t)((int16_t)_dividend * (int16_t)_multiplier);
	} else {
		//Unsigned multiplication (16x16=32)
		_multDivResult = (uint32_t)(_dividend * _multiplier);
	}

	_aluState |= 0x01;
	_aluState &= 0x7F;
}

void Spc7110::ProcessDivision()
{
	if(_aluFlags & 0x01) {
		//Signed division (32 / 16 = 32 + 16)
		int32_t dividend = (int32_t)_dividend;
		int16_t divisor = (int16_t)_divisor;

		if(divisor != 0) {
			_multDivResult = (int32_t)(dividend / divisor);
			_remainder = ((int32_t)(dividend % divisor)) & 0xFFFF;
		} else {
			_multDivResult = 0;
			_remainder = dividend & 0xFFFF;
		}
	} else {
		//Unsigned division (32 / 16 = 32 + 16)
		if(_divisor != 0) {
			_multDivResult = _dividend / _divisor;
			_remainder = (uint16_t)(_dividend % _divisor);
		} else {
			_multDivResult = 0;
			_remainder = _dividend & 0xffff;
		}
	}

	_aluState &= 0x7F;
}

uint8_t Spc7110::ReadDataRom(uint32_t addr)
{
	uint32_t configSize = 0x100000 * (1 << (_dataRomSize & 0x03));
	uint32_t dataRomSize = std::min(configSize, _realDataRomSize);
	if(addr >= dataRomSize) {
		return 0x00;
	}

	return _cart->DebugGetPrgRom()[0x100000 + addr];
}

void Spc7110::FillReadBuffer()
{
	int32_t offset = _readMode & 0x02 ? _readOffset : 0;
	if(_readMode & 0x08) {
		offset = (int16_t)offset;
	}
	_readBuffer = ReadDataRom(_readBase + offset);
}

void Spc7110::IncrementPosition()
{
	_readBase += (_readMode & 0x08) ? (int16_t)_readOffset : _readOffset;
	FillReadBuffer();
}

void Spc7110::IncrementPosition4810()
{
	int32_t step = _readMode & 0x01 ? _readStep : 1;
	if(_readMode & 0x04) {
		step = (int16_t)step;
	}

	if(_readMode & 0x10) {
		_readOffset += step;
	} else {
		_readBase = (_readBase + step) & 0xFFFFFF;
	}

	FillReadBuffer();
}

void Spc7110::LoadEntryHeader()
{
	uint32_t address = _directoryBase + _directoryIndex * 4;
	_decompMode = ReadDataRom(address) & 0x03;
	_srcAddress = (ReadDataRom(address + 1) << 16) | (ReadDataRom(address + 2) << 8) | ReadDataRom(address + 3);
}

void Spc7110::BeginDecompression()
{
	if(_decompMode == 3) {
		return;
	}

	_decomp->Initialize(_decompMode, _srcAddress);
	_decomp->Decode();

	uint32_t seek = _decompFlags & 0x02 ? _targetOffset : 0;
	while(seek--) {
		_decomp->Decode();
	}

	_decompStatus |= 0x80;
	_decompOffset = 0;
}

uint8_t Spc7110::ReadDecompressedByte()
{
	if((_decompStatus & 0x80) == 0) {
		return 0x00;
	}

	uint8_t bpp = _decomp->GetBpp();
	if(_decompOffset == 0) {
		for(int i = 0; i < 8; i++) {
			uint32_t result = _decomp->GetResult();
			switch(bpp) {
				case 1:
					_decompBuffer[i] = result;
					break;
				case 2:
					_decompBuffer[i * 2 + 0] = result >> 0;
					_decompBuffer[i * 2 + 1] = result >> 8;
					break;
				case 4:
					_decompBuffer[i * 2 + 0] = result >> 0;
					_decompBuffer[i * 2 + 1] = result >> 8;
					_decompBuffer[i * 2 + 16] = result >> 16;
					_decompBuffer[i * 2 + 17] = result >> 24;
					break;
			}

			uint32_t seek = (_decompFlags & 0x01) ? _skipBytes : 1;
			while(seek--) {
				_decomp->Decode();
			}
		}
	}

	uint8_t data = _decompBuffer[_decompOffset++];
	_decompOffset &= (bpp * 8) - 1;
	return data;
}

uint8_t Spc7110::Peek(uint32_t addr)
{
	return 0;
}

void Spc7110::PeekBlock(uint32_t addr, uint8_t* output)
{
	memset(output, 0, 0x1000);
}

AddressInfo Spc7110::GetAbsoluteAddress(uint32_t address)
{
	return { -1, MemoryType::None };
}

void Spc7110::Reset()
{
	_directoryBase = 0;
	_directoryIndex = 0;
	_targetOffset = 0;
	_dataLengthCounter = 0;
	_skipBytes = 0;
	_decompFlags = 0;
	_decompMode = 0;
	_srcAddress = 0;
	_decompOffset = 0;
	_decompStatus = 0;
	memset(_decompBuffer, 0, sizeof(_decompBuffer));

	_dividend = 0;
	_multiplier = 0;
	_divisor = 0;
	_multDivResult = 0;
	_remainder = 0;
	_aluState = 0;
	_aluFlags = 0;

	_sramEnabled = 0;
	_dataRomBanks[0] = 0;
	_dataRomBanks[1] = 1;
	_dataRomBanks[2] = 2;
	_dataRomSize = 0;

	_readBase = 0;
	_readOffset = 0;
	_readStep = 0;
	_readMode = 0;
	_readBuffer = 0;

	UpdateMappings();

	_decomp.reset(new Spc7110Decomp(this));
	if(_useRtc) {
		_rtc.reset(new Rtc4513(_emu));
	}
}

void Spc7110::LoadBattery()
{
	if(_rtc) {
		_rtc->LoadBattery();
	}
}

void Spc7110::SaveBattery()
{
	if(_rtc) {
		_rtc->SaveBattery();
	}
}
