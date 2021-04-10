#include "stdafx.h"
#include "Debugger/Debugger.h"
#include "Shared/Emulator.h"
#include "SNES/MemoryManager.h"
#include "SNES/Ppu.h"
#include "SNES/Spc.h"
#include "SNES/Coprocessors/DSP/NecDsp.h"
#include "SNES/Coprocessors/SA1/Sa1.h"
#include "SNES/Coprocessors/CX4/Cx4.h"
#include "SNES/Coprocessors/GSU/Gsu.h"
#include "Gameboy/Gameboy.h"
#include "Gameboy/GbMemoryManager.h"
#include "SNES/Coprocessors/BSX/BsxCart.h"
#include "SNES/Coprocessors/BSX/BsxMemoryPack.h"
#include "SNES/Console.h"
#include "Debugger/MemoryDumper.h"
#include "SNES/BaseCartridge.h"
#include "NES/NesConsole.h"
#include "NES/NesMemoryManager.h"
#include "Shared/Video/VideoDecoder.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/DebugBreakHelper.h"
#include "Debugger/Disassembler.h"

MemoryDumper::MemoryDumper(Debugger* debugger)
{
	_debugger = debugger;
	_emu = debugger->GetEmulator();
	_disassembler = debugger->GetDisassembler().get();

	IConsole* console = _debugger->GetConsole();
	//TODO
	if(Console* c = dynamic_cast<Console*>(console)) {
		_ppu = c->GetPpu().get();
		_spc = c->GetSpc().get();
		_memoryManager = c->GetMemoryManager().get();
		_cartridge = c->GetCartridge().get();
	} else if(NesConsole* c = dynamic_cast<NesConsole*>(console)) {
		_nesMemoryManager = c->GetMemoryManager();
	}
}

void MemoryDumper::SetMemoryState(SnesMemoryType type, uint8_t *buffer, uint32_t length)
{
	if(length > GetMemorySize(type)) {
		return;
	}

	uint8_t* dst = GetMemoryBuffer(type);
	if(dst) {
		memcpy(dst, buffer, length);
	}
}

uint8_t* MemoryDumper::GetMemoryBuffer(SnesMemoryType type)
{
	return (uint8_t*)_emu->GetMemory(type).Memory;
}

uint32_t MemoryDumper::GetMemorySize(SnesMemoryType type)
{
	switch(type) {
		case SnesMemoryType::CpuMemory: return 0x1000000;
		case SnesMemoryType::SpcMemory: return 0x10000;
		case SnesMemoryType::Sa1Memory: return 0x1000000;
		case SnesMemoryType::GsuMemory: return 0x1000000;
		case SnesMemoryType::Cx4Memory: return 0x1000000;
		case SnesMemoryType::GameboyMemory: return 0x10000;
		case SnesMemoryType::NesMemory: return 0x10000;
		default: return _emu->GetMemory(type).Size;
	}
}

void MemoryDumper::GetMemoryState(SnesMemoryType type, uint8_t *buffer)
{
	switch(type) {
		case SnesMemoryType::CpuMemory:
			for(int i = 0; i <= 0xFFFFFF; i+=0x1000) {
				_memoryManager->PeekBlock(i, buffer+i);
			}
			break;

		case SnesMemoryType::SpcMemory:
			for(int i = 0; i <= 0xFFFF; i++) {
				buffer[i] = _spc->DebugRead(i);
			}
			break;
		
		case SnesMemoryType::Sa1Memory:
			if(_cartridge->GetSa1()) {
				for(int i = 0; i <= 0xFFFFFF; i += 0x1000) {
					_cartridge->GetSa1()->GetMemoryMappings()->PeekBlock(i, buffer + i);
				}
			}
			break;

		case SnesMemoryType::GsuMemory:
			if(_cartridge->GetGsu()) {
				for(int i = 0; i <= 0xFFFFFF; i += 0x1000) {
					_cartridge->GetGsu()->GetMemoryMappings()->PeekBlock(i, buffer + i);
				}
			}
			break;

		case SnesMemoryType::Cx4Memory:
			if(_cartridge->GetCx4()) {
				for(int i = 0; i <= 0xFFFFFF; i += 0x1000) {
					_cartridge->GetCx4()->GetMemoryMappings()->PeekBlock(i, buffer + i);
				}
			}
			break;

		case SnesMemoryType::GameboyMemory: {
			if(_cartridge->GetGameboy()) {
				GbMemoryManager* memManager = _cartridge->GetGameboy()->GetMemoryManager();
				for(int i = 0; i <= 0xFFFF; i++) {
					buffer[i] = memManager->DebugRead(i);
				}
			}
			break;
		}

		case SnesMemoryType::NesMemory: {
			if(_nesMemoryManager) {
				for(int i = 0; i <= 0xFFFF; i++) {
					buffer[i] = _nesMemoryManager->DebugRead(i);
				}
			}
			break;
		}

		default: 
			uint8_t* src = GetMemoryBuffer(type);
			if(src) {
				memcpy(buffer, src, GetMemorySize(type));
			}
			break;
	}
}

void MemoryDumper::SetMemoryValues(SnesMemoryType memoryType, uint32_t address, uint8_t* data, uint32_t length)
{
	DebugBreakHelper helper(_debugger);
	for(uint32_t i = 0; i < length; i++) {
		SetMemoryValue(memoryType, address+i, data[i], true);
	}
}

void MemoryDumper::SetMemoryValue(SnesMemoryType memoryType, uint32_t address, uint8_t value, bool disableSideEffects)
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
		case SnesMemoryType::CpuMemory: _memoryManager->GetMemoryMappings()->DebugWrite(address, value); break;
		case SnesMemoryType::SpcMemory: _spc->DebugWrite(address, value); break;
		case SnesMemoryType::Sa1Memory: _cartridge->GetSa1()->GetMemoryMappings()->DebugWrite(address, value); break;
		case SnesMemoryType::GsuMemory: _cartridge->GetGsu()->GetMemoryMappings()->DebugWrite(address, value); break;
		case SnesMemoryType::Cx4Memory: _cartridge->GetCx4()->GetMemoryMappings()->DebugWrite(address, value); break;
		case SnesMemoryType::GameboyMemory: _cartridge->GetGameboy()->GetMemoryManager()->DebugWrite(address, value); break;
		case SnesMemoryType::NesMemory: _nesMemoryManager->DebugWrite(address, value); break;

		default:
			uint8_t* src = GetMemoryBuffer(memoryType);
			if(src) {
				src[address] = value;
				invalidateCache();
			}
			break;
	}
}

void MemoryDumper::GetMemoryValues(SnesMemoryType memoryType, uint32_t start, uint32_t end, uint8_t* output)
{
	int x = 0;
	for(uint32_t i = start; i < end; i++) {
		output[x++] = GetMemoryValue(memoryType, i);
	}
}

uint8_t MemoryDumper::GetMemoryValue(SnesMemoryType memoryType, uint32_t address, bool disableSideEffects)
{
	if(address >= GetMemorySize(memoryType)) {
		return 0;
	}

	switch(memoryType) {
		//TODO
		case SnesMemoryType::CpuMemory: return _memoryManager->Peek(address);
		case SnesMemoryType::SpcMemory: return _spc->DebugRead(address);
		case SnesMemoryType::Sa1Memory: return _cartridge->GetSa1()->GetMemoryMappings()->Peek(address);
		case SnesMemoryType::GsuMemory: return _cartridge->GetGsu()->GetMemoryMappings()->Peek(address);
		case SnesMemoryType::Cx4Memory: return _cartridge->GetCx4()->GetMemoryMappings()->Peek(address);
		case SnesMemoryType::GameboyMemory: return _cartridge->GetGameboy()->GetMemoryManager()->DebugRead(address);
		case SnesMemoryType::NesMemory: return _nesMemoryManager->DebugRead(address);
		
		default:
			uint8_t* src = GetMemoryBuffer(memoryType);
			return src ? src[address] : 0;
	}
}

uint16_t MemoryDumper::GetMemoryValueWord(SnesMemoryType memoryType, uint32_t address, bool disableSideEffects)
{
	uint32_t memorySize = GetMemorySize(memoryType);
	uint8_t lsb = GetMemoryValue(memoryType, address);
	uint8_t msb = GetMemoryValue(memoryType, (address + 1) & (memorySize - 1));
	return (msb << 8) | lsb;
}

void MemoryDumper::SetMemoryValueWord(SnesMemoryType memoryType, uint32_t address, uint16_t value, bool disableSideEffects)
{
	DebugBreakHelper helper(_debugger);
	SetMemoryValue(memoryType, address, (uint8_t)value, disableSideEffects);
	SetMemoryValue(memoryType, address + 1, (uint8_t)(value >> 8), disableSideEffects);
}