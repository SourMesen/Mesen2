#include "stdafx.h"
#include "Debugger.h"
#include "MemoryManager.h"
#include "Ppu.h"
#include "Spc.h"
#include "NecDsp.h"
#include "Sa1.h"
#include "Cx4.h"
#include "Gsu.h"
#include "Gameboy.h"
#include "GbMemoryManager.h"
#include "BsxCart.h"
#include "BsxMemoryPack.h"
#include "Console.h"
#include "MemoryDumper.h"
#include "BaseCartridge.h"
#include "VideoDecoder.h"
#include "DebugTypes.h"
#include "DebugBreakHelper.h"
#include "Disassembler.h"

MemoryDumper::MemoryDumper(Debugger* debugger)
{
	_debugger = debugger;
	_disassembler = debugger->GetDisassembler().get();
	_ppu = debugger->GetConsole()->GetPpu().get();
	_spc = debugger->GetConsole()->GetSpc().get();
	_memoryManager = debugger->GetConsole()->GetMemoryManager().get();
	_cartridge = debugger->GetConsole()->GetCartridge().get();
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
	switch(type) {
		default: return nullptr;
		case SnesMemoryType::PrgRom: return _cartridge->DebugGetPrgRom();
		case SnesMemoryType::WorkRam: return _memoryManager->DebugGetWorkRam();
		case SnesMemoryType::SaveRam: return _cartridge->DebugGetSaveRam();
		case SnesMemoryType::VideoRam: return _ppu->GetVideoRam();
		case SnesMemoryType::SpriteRam: return _ppu->GetSpriteRam();
		case SnesMemoryType::CGRam: return _ppu->GetCgRam();
		case SnesMemoryType::SpcRam: return _spc->GetSpcRam();
		case SnesMemoryType::SpcRom: return _spc->GetSpcRom();

		case SnesMemoryType::DspProgramRom: return _cartridge->GetDsp() ? _cartridge->GetDsp()->DebugGetProgramRom() : nullptr;
		case SnesMemoryType::DspDataRom: return _cartridge->GetDsp() ? _cartridge->GetDsp()->DebugGetDataRom() : nullptr;
		case SnesMemoryType::DspDataRam: return _cartridge->GetDsp() ? _cartridge->GetDsp()->DebugGetDataRam() : nullptr;

		case SnesMemoryType::Sa1InternalRam: return _cartridge->GetSa1() ? _cartridge->GetSa1()->DebugGetInternalRam() : nullptr;
		case SnesMemoryType::GsuWorkRam: return _cartridge->GetGsu() ? _cartridge->GetGsu()->DebugGetWorkRam() : nullptr;
		case SnesMemoryType::Cx4DataRam: return _cartridge->GetCx4() ? _cartridge->GetCx4()->DebugGetDataRam() : nullptr;
		case SnesMemoryType::BsxPsRam: return _cartridge->GetBsx() ? _cartridge->GetBsx()->DebugGetPsRam() : nullptr;
		case SnesMemoryType::BsxMemoryPack: return _cartridge->GetBsxMemoryPack() ? _cartridge->GetBsxMemoryPack()->DebugGetMemoryPack() : nullptr;

		case SnesMemoryType::GbPrgRom:
		case SnesMemoryType::GbWorkRam:
		case SnesMemoryType::GbVideoRam:
		case SnesMemoryType::GbCartRam:
		case SnesMemoryType::GbHighRam:
		case SnesMemoryType::GbBootRom:
		case SnesMemoryType::GbSpriteRam:
			return _cartridge->GetGameboy() ? _cartridge->GetGameboy()->DebugGetMemory(type) : nullptr;
	}
}

uint32_t MemoryDumper::GetMemorySize(SnesMemoryType type)
{
	switch(type) {
		default: return 0;
		case SnesMemoryType::CpuMemory: return 0x1000000;
		case SnesMemoryType::SpcMemory: return 0x10000;
		case SnesMemoryType::Sa1Memory: return 0x1000000;
		case SnesMemoryType::GsuMemory: return 0x1000000;
		case SnesMemoryType::Cx4Memory: return 0x1000000;
		case SnesMemoryType::GameboyMemory: return 0x10000;
		case SnesMemoryType::PrgRom: return _cartridge->DebugGetPrgRomSize();
		case SnesMemoryType::WorkRam: return MemoryManager::WorkRamSize;
		case SnesMemoryType::SaveRam: return _cartridge->DebugGetSaveRamSize();
		case SnesMemoryType::VideoRam: return Ppu::VideoRamSize;
		case SnesMemoryType::SpriteRam: return Ppu::SpriteRamSize;
		case SnesMemoryType::CGRam: return Ppu::CgRamSize;
		case SnesMemoryType::SpcRam: return Spc::SpcRamSize;
		case SnesMemoryType::SpcRom: return Spc::SpcRomSize;
		case SnesMemoryType::Register: return 0x10000;

		case SnesMemoryType::DspProgramRom: return _cartridge->GetDsp() ? _cartridge->GetDsp()->DebugGetProgramRomSize() : 0;
		case SnesMemoryType::DspDataRom: return _cartridge->GetDsp() ? _cartridge->GetDsp()->DebugGetDataRomSize() : 0;
		case SnesMemoryType::DspDataRam: return _cartridge->GetDsp() ? _cartridge->GetDsp()->DebugGetDataRamSize() : 0;
		
		case SnesMemoryType::Sa1InternalRam: return _cartridge->GetSa1() ? _cartridge->GetSa1()->DebugGetInternalRamSize() : 0;
		case SnesMemoryType::GsuWorkRam: return _cartridge->GetGsu() ? _cartridge->GetGsu()->DebugGetWorkRamSize() : 0;
		case SnesMemoryType::Cx4DataRam: return _cartridge->GetCx4() ? _cartridge->GetCx4()->DebugGetDataRamSize() : 0;
		case SnesMemoryType::BsxPsRam: return _cartridge->GetBsx() ? _cartridge->GetBsx()->DebugGetPsRamSize() : 0;
		case SnesMemoryType::BsxMemoryPack: return _cartridge->GetBsxMemoryPack() ? _cartridge->GetBsxMemoryPack()->DebugGetMemoryPackSize() : 0;
		
		case SnesMemoryType::GbPrgRom:
		case SnesMemoryType::GbWorkRam:
		case SnesMemoryType::GbVideoRam:
		case SnesMemoryType::GbCartRam:
		case SnesMemoryType::GbHighRam:
		case SnesMemoryType::GbBootRom:
		case SnesMemoryType::GbSpriteRam:
			return _cartridge->GetGameboy() ? _cartridge->GetGameboy()->DebugGetMemorySize(type) : 0;
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
		case SnesMemoryType::CpuMemory: return _memoryManager->Peek(address);
		case SnesMemoryType::SpcMemory: return _spc->DebugRead(address);
		case SnesMemoryType::Sa1Memory: return _cartridge->GetSa1()->GetMemoryMappings()->Peek(address);
		case SnesMemoryType::GsuMemory: return _cartridge->GetGsu()->GetMemoryMappings()->Peek(address);
		case SnesMemoryType::Cx4Memory: return _cartridge->GetCx4()->GetMemoryMappings()->Peek(address);
		case SnesMemoryType::GameboyMemory: return _cartridge->GetGameboy()->GetMemoryManager()->DebugRead(address);
		
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