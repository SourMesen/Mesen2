#include "stdafx.h"
#include "Debugger/Debugger.h"
#include "Shared/Emulator.h"
#include "SNES/SnesMemoryManager.h"
#include "SNES/SnesPpu.h"
#include "SNES/Spc.h"
#include "SNES/Coprocessors/DSP/NecDsp.h"
#include "SNES/Coprocessors/SA1/Sa1.h"
#include "SNES/Coprocessors/CX4/Cx4.h"
#include "SNES/Coprocessors/GSU/Gsu.h"
#include "Gameboy/Gameboy.h"
#include "Gameboy/GbMemoryManager.h"
#include "SNES/Coprocessors/BSX/BsxCart.h"
#include "SNES/Coprocessors/BSX/BsxMemoryPack.h"
#include "SNES/SnesConsole.h"
#include "Debugger/MemoryDumper.h"
#include "SNES/BaseCartridge.h"
#include "NES/NesConsole.h"
#include "Shared/Video/VideoDecoder.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/DebugBreakHelper.h"
#include "Debugger/Disassembler.h"

MemoryDumper::MemoryDumper(Debugger* debugger)
{
	_debugger = debugger;
	_emu = debugger->GetEmulator();
	_disassembler = debugger->GetDisassembler();

	IConsole* console = _debugger->GetConsole();
	//TODO
	if(SnesConsole* snes = dynamic_cast<SnesConsole*>(console)) {
		_ppu = snes->GetPpu();
		_spc = snes->GetSpc();
		_memoryManager = snes->GetMemoryManager();
		_cartridge = snes->GetCartridge();
		_gameboy = snes->GetCartridge()->GetGameboy();
	} else if(NesConsole* nes = dynamic_cast<NesConsole*>(console)) {
		_nesConsole = nes;
	} else if(Gameboy* gb = dynamic_cast<Gameboy*>(console)) {
		_gameboy = gb;
	}

	for(int i = 0; i <= (int)MemoryType::Register; i++) {
		_memorySize[i] = InternalGetMemorySize((MemoryType)i);
	}
}

void MemoryDumper::SetMemoryState(MemoryType type, uint8_t *buffer, uint32_t length)
{
	if(length > GetMemorySize(type)) {
		return;
	}

	uint8_t* dst = GetMemoryBuffer(type);
	if(dst) {
		memcpy(dst, buffer, length);
	}
}

uint8_t* MemoryDumper::GetMemoryBuffer(MemoryType type)
{
	return (uint8_t*)_emu->GetMemory(type).Memory;
}

uint32_t MemoryDumper::GetMemorySize(MemoryType type)
{
	if(type <= DebugUtilities::GetLastCpuMemoryType()) {
		return _memorySize[(int)type];
	} else {
		return _emu->GetMemory(type).Size;
	}
}

uint32_t MemoryDumper::InternalGetMemorySize(MemoryType type)
{
	if(!_debugger->HasCpuType(DebugUtilities::ToCpuType(type))) {
		return 0;
	}

	switch(type) {
		case MemoryType::SnesMemory: return 0x1000000;
		case MemoryType::SpcMemory: return 0x10000;
		case MemoryType::NecDspMemory: return _emu->GetMemory(MemoryType::DspProgramRom).Size;
		case MemoryType::Sa1Memory: return 0x1000000;
		case MemoryType::GsuMemory: return 0x1000000;
		case MemoryType::Cx4Memory: return 0x1000000;
		case MemoryType::GameboyMemory: return 0x10000;
		case MemoryType::NesMemory: return 0x10000;
		case MemoryType::NesPpuMemory: return 0x4000;
		case MemoryType::Register: return 0x10000;
		default: return _emu->GetMemory(type).Size;
	}
}

void MemoryDumper::GetMemoryState(MemoryType type, uint8_t *buffer)
{
	if(GetMemorySize(type) == 0) {
		return;
	}

	switch(type) {
		case MemoryType::SnesMemory:
			for(int i = 0; i <= 0xFFFFFF; i+=0x1000) {
				_memoryManager->PeekBlock(i, buffer+i);
			}
			break;

		case MemoryType::SpcMemory:
			for(int i = 0; i <= 0xFFFF; i++) {
				buffer[i] = _spc->DebugRead(i);
			}
			break;
		
		case MemoryType::Sa1Memory:
			if(_cartridge->GetSa1()) {
				for(int i = 0; i <= 0xFFFFFF; i += 0x1000) {
					_cartridge->GetSa1()->GetMemoryMappings()->PeekBlock(i, buffer + i);
				}
			}
			break;

		case MemoryType::GsuMemory:
			if(_cartridge->GetGsu()) {
				for(int i = 0; i <= 0xFFFFFF; i += 0x1000) {
					_cartridge->GetGsu()->GetMemoryMappings()->PeekBlock(i, buffer + i);
				}
			}
			break;

		case MemoryType::Cx4Memory:
			if(_cartridge->GetCx4()) {
				for(int i = 0; i <= 0xFFFFFF; i += 0x1000) {
					_cartridge->GetCx4()->GetMemoryMappings()->PeekBlock(i, buffer + i);
				}
			}
			break;

		case MemoryType::GameboyMemory: {
			if(_gameboy) {
				GbMemoryManager* memManager = _gameboy->GetMemoryManager();
				for(int i = 0; i <= 0xFFFF; i++) {
					buffer[i] = memManager->DebugRead(i);
				}
			}
			break;
		}

		case MemoryType::NesMemory: {
			if(_nesConsole) {
				for(int i = 0; i <= 0xFFFF; i++) {
					buffer[i] = _nesConsole->DebugRead(i);
				}
			}
			break;
		}

		case MemoryType::NesPpuMemory: {
			if(_nesConsole) {
				for(int i = 0; i < 0x4000; i++) {
					buffer[i] = _nesConsole->DebugReadVram(i);
				}
			}
			break;
		}

		case MemoryType::NecDspMemory:
			GetMemoryState(MemoryType::DspProgramRom, buffer);
			break;

		default: 
			uint8_t* src = GetMemoryBuffer(type);
			if(src) {
				memcpy(buffer, src, GetMemorySize(type));
			}
			break;
	}
}

void MemoryDumper::SetMemoryValues(MemoryType memoryType, uint32_t address, uint8_t* data, uint32_t length)
{
	DebugBreakHelper helper(_debugger);
	for(uint32_t i = 0; i < length; i++) {
		SetMemoryValue(memoryType, address+i, data[i], true);
	}
}

void MemoryDumper::SetMemoryValue(MemoryType memoryType, uint32_t address, uint8_t value, bool disableSideEffects)
{
	if(address >= GetMemorySize(memoryType)) {
		return;
	}

	if(disableSideEffects && memoryType <= DebugUtilities::GetLastCpuMemoryType()) {
		AddressInfo addr = { (int32_t)address, memoryType };
		addr = _debugger->GetAbsoluteAddress(addr);
		if(addr.Address >= 0) {
			SetMemoryValue(addr.Type, addr.Address, value, true);
		}
		return;
	}

	auto invalidateCache = [=]() {
		AddressInfo addr = { (int32_t)address, memoryType };
		_debugger->GetDisassembler()->InvalidateCache(addr, DebugUtilities::ToCpuType(memoryType));
	};

	switch(memoryType) {
		case MemoryType::SnesMemory: _memoryManager->GetMemoryMappings()->DebugWrite(address, value); break;
		case MemoryType::SpcMemory: _spc->DebugWrite(address, value); break;
		case MemoryType::Sa1Memory: _cartridge->GetSa1()->GetMemoryMappings()->DebugWrite(address, value); break;
		case MemoryType::GsuMemory: _cartridge->GetGsu()->GetMemoryMappings()->DebugWrite(address, value); break;
		case MemoryType::Cx4Memory: _cartridge->GetCx4()->GetMemoryMappings()->DebugWrite(address, value); break;
		case MemoryType::GameboyMemory: _gameboy->GetMemoryManager()->DebugWrite(address, value); break;
		case MemoryType::NesMemory: _nesConsole->DebugWrite(address, value); break;
		case MemoryType::NesPpuMemory: _nesConsole->DebugWriteVram(address, value); break;
		case MemoryType::NecDspMemory: 
			SetMemoryValue(MemoryType::DspProgramRom, address, value, disableSideEffects);
			return;

		default:
			uint8_t* src = GetMemoryBuffer(memoryType);
			if(src) {
				src[address] = value;
				invalidateCache();
			}
			break;
	}
}

void MemoryDumper::GetMemoryValues(MemoryType memoryType, uint32_t start, uint32_t end, uint8_t* output)
{
	int x = 0;
	uint32_t size = GetMemorySize(memoryType);
	for(uint32_t i = start; i <= end && i < size; i++) {
		output[x++] = InternalGetMemoryValue(memoryType, i);
	}
}

uint8_t MemoryDumper::GetMemoryValue(MemoryType memoryType, uint32_t address, bool disableSideEffects)
{
	if(address >= GetMemorySize(memoryType)) {
		return 0;
	}

	return InternalGetMemoryValue(memoryType, address, disableSideEffects);
}

uint8_t MemoryDumper::InternalGetMemoryValue(MemoryType memoryType, uint32_t address, bool disableSideEffects)
{
	switch(memoryType) {
		//TODO
		case MemoryType::SnesMemory: return _memoryManager->Peek(address);
		case MemoryType::SpcMemory: return _spc->DebugRead(address);
		case MemoryType::Sa1Memory: return _cartridge->GetSa1()->GetMemoryMappings()->Peek(address);
		case MemoryType::GsuMemory: return _cartridge->GetGsu()->GetMemoryMappings()->Peek(address);
		case MemoryType::Cx4Memory: return _cartridge->GetCx4()->GetMemoryMappings()->Peek(address);
		case MemoryType::GameboyMemory: return _gameboy->GetMemoryManager()->DebugRead(address);
		case MemoryType::NesMemory: return _nesConsole->DebugRead(address);
		case MemoryType::NesPpuMemory: return _nesConsole->DebugReadVram(address);
		case MemoryType::NecDspMemory: return GetMemoryValue(MemoryType::DspProgramRom, address);

		default:
			uint8_t* src = GetMemoryBuffer(memoryType);
			return src ? src[address] : 0;
	}
}

uint16_t MemoryDumper::GetMemoryValueWord(MemoryType memoryType, uint32_t address, bool disableSideEffects)
{
	uint32_t memorySize = GetMemorySize(memoryType);
	uint8_t lsb = GetMemoryValue(memoryType, address);
	uint8_t msb = GetMemoryValue(memoryType, address + 1 >= memorySize ? 0 : address + 1);
	return (msb << 8) | lsb;
}

void MemoryDumper::SetMemoryValueWord(MemoryType memoryType, uint32_t address, uint16_t value, bool disableSideEffects)
{
	DebugBreakHelper helper(_debugger);
	SetMemoryValue(memoryType, address, (uint8_t)value, disableSideEffects);
	SetMemoryValue(memoryType, address + 1, (uint8_t)(value >> 8), disableSideEffects);
}