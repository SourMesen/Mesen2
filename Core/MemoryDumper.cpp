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

	switch(type) {
		default:
		case SnesMemoryType::CpuMemory:
		case SnesMemoryType::SpcMemory:
		case SnesMemoryType::Sa1Memory:
		case SnesMemoryType::GsuMemory:
		case SnesMemoryType::Cx4Memory:
		case SnesMemoryType::GameboyMemory:
			break;
		
		case SnesMemoryType::PrgRom: memcpy(_cartridge->DebugGetPrgRom(), buffer, length); break;
		case SnesMemoryType::WorkRam: memcpy(_memoryManager->DebugGetWorkRam(), buffer, length); break;
		case SnesMemoryType::SaveRam: memcpy(_cartridge->DebugGetSaveRam(), buffer, length); break;
		case SnesMemoryType::VideoRam: memcpy(_ppu->GetVideoRam(), buffer, length); break;
		case SnesMemoryType::SpriteRam: memcpy(_ppu->GetSpriteRam(), buffer, length); break;
		case SnesMemoryType::CGRam: memcpy(_ppu->GetCgRam(), buffer, length); break;
		case SnesMemoryType::SpcRam: memcpy(_spc->GetSpcRam(), buffer, length); break;
		case SnesMemoryType::SpcRom: memcpy(_spc->GetSpcRom(), buffer, length); break;

		case SnesMemoryType::DspProgramRom: memcpy(_cartridge->GetDsp()->DebugGetProgramRom(), buffer, length); _cartridge->GetDsp()->BuildProgramCache(); break;
		case SnesMemoryType::DspDataRom: memcpy(_cartridge->GetDsp()->DebugGetDataRom(), buffer, length); break;
		case SnesMemoryType::DspDataRam: memcpy(_cartridge->GetDsp()->DebugGetDataRam(), buffer, length); break;

		case SnesMemoryType::Sa1InternalRam: memcpy(_cartridge->GetSa1()->DebugGetInternalRam(), buffer, length); break;
		case SnesMemoryType::GsuWorkRam: memcpy(_cartridge->GetGsu()->DebugGetWorkRam(), buffer, length); break;
		case SnesMemoryType::Cx4DataRam: memcpy(_cartridge->GetCx4()->DebugGetDataRam(), buffer, length); break;
		case SnesMemoryType::BsxPsRam: memcpy(_cartridge->GetBsx()->DebugGetPsRam(), buffer, length); break;
		case SnesMemoryType::BsxMemoryPack: memcpy(_cartridge->GetBsxMemoryPack()->DebugGetMemoryPack(), buffer, length); break;

		case SnesMemoryType::GbPrgRom:
		case SnesMemoryType::GbWorkRam:
		case SnesMemoryType::GbVideoRam:
		case SnesMemoryType::GbCartRam:
		case SnesMemoryType::GbHighRam:
			if(_cartridge->GetGameboy()) {
				memcpy(_cartridge->GetGameboy()->DebugGetMemory(type), buffer, length);
			}
			break;

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
			return _cartridge->GetGameboy() ? _cartridge->GetGameboy()->DebugGetMemorySize(type) : 0;
	}
}

void MemoryDumper::GetMemoryState(SnesMemoryType type, uint8_t *buffer)
{
	switch(type) {
		default: break;

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

		case SnesMemoryType::PrgRom: memcpy(buffer, _cartridge->DebugGetPrgRom(), _cartridge->DebugGetPrgRomSize()); break;
		case SnesMemoryType::WorkRam: memcpy(buffer, _memoryManager->DebugGetWorkRam(), MemoryManager::WorkRamSize); break;
		case SnesMemoryType::SaveRam: memcpy(buffer, _cartridge->DebugGetSaveRam(), _cartridge->DebugGetSaveRamSize()); break;
		case SnesMemoryType::VideoRam: memcpy(buffer, _ppu->GetVideoRam(), Ppu::VideoRamSize);	break;
		case SnesMemoryType::SpriteRam: memcpy(buffer, _ppu->GetSpriteRam(), Ppu::SpriteRamSize);	break;
		case SnesMemoryType::CGRam: memcpy(buffer, _ppu->GetCgRam(), Ppu::CgRamSize); break;
		case SnesMemoryType::SpcRam: memcpy(buffer, _spc->GetSpcRam(), Spc::SpcRamSize); break;
		case SnesMemoryType::SpcRom: memcpy(buffer, _spc->GetSpcRom(), Spc::SpcRomSize); break;

		case SnesMemoryType::DspProgramRom: memcpy(buffer, _cartridge->GetDsp()->DebugGetProgramRom(), _cartridge->GetDsp()->DebugGetProgramRomSize()); break;
		case SnesMemoryType::DspDataRom: memcpy(buffer, _cartridge->GetDsp()->DebugGetDataRom(), _cartridge->GetDsp()->DebugGetDataRomSize()); break;
		case SnesMemoryType::DspDataRam: memcpy(buffer, _cartridge->GetDsp()->DebugGetDataRam(), _cartridge->GetDsp()->DebugGetDataRamSize()); break;

		case SnesMemoryType::Sa1InternalRam: memcpy(buffer, _cartridge->GetSa1()->DebugGetInternalRam(), _cartridge->GetSa1()->DebugGetInternalRamSize()); break;
		case SnesMemoryType::GsuWorkRam: memcpy(buffer, _cartridge->GetGsu()->DebugGetWorkRam(), _cartridge->GetGsu()->DebugGetWorkRamSize()); break;
		case SnesMemoryType::Cx4DataRam: memcpy(buffer, _cartridge->GetCx4()->DebugGetDataRam(), _cartridge->GetCx4()->DebugGetDataRamSize()); break;
		case SnesMemoryType::BsxPsRam: memcpy(buffer, _cartridge->GetBsx()->DebugGetPsRam(), _cartridge->GetBsx()->DebugGetPsRamSize()); break;
		case SnesMemoryType::BsxMemoryPack: memcpy(buffer, _cartridge->GetBsxMemoryPack()->DebugGetMemoryPack(), _cartridge->GetBsxMemoryPack()->DebugGetMemoryPackSize()); break;

		case SnesMemoryType::GbPrgRom:
		case SnesMemoryType::GbWorkRam:
		case SnesMemoryType::GbVideoRam:
		case SnesMemoryType::GbCartRam:
		case SnesMemoryType::GbHighRam:
			if(_cartridge->GetGameboy()) {
				memcpy(buffer, _cartridge->GetGameboy()->DebugGetMemory(type), _cartridge->GetGameboy()->DebugGetMemorySize(type));
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
		default: break;

		case SnesMemoryType::CpuMemory: _memoryManager->GetMemoryMappings()->DebugWrite(address, value); break;
		case SnesMemoryType::SpcMemory: _spc->DebugWrite(address, value); break;
		case SnesMemoryType::Sa1Memory: _cartridge->GetSa1()->GetMemoryMappings()->DebugWrite(address, value); break;
		case SnesMemoryType::GsuMemory: _cartridge->GetGsu()->GetMemoryMappings()->DebugWrite(address, value); break;
		case SnesMemoryType::Cx4Memory: _cartridge->GetCx4()->GetMemoryMappings()->DebugWrite(address, value); break;
		case SnesMemoryType::GameboyMemory: _cartridge->GetGameboy()->GetMemoryManager()->DebugWrite(address, value); break;

		case SnesMemoryType::PrgRom: _cartridge->DebugGetPrgRom()[address] = value; invalidateCache(); break;
		case SnesMemoryType::WorkRam: _memoryManager->DebugGetWorkRam()[address] = value; invalidateCache(); break;
		case SnesMemoryType::SaveRam: _cartridge->DebugGetSaveRam()[address] = value; invalidateCache(); break;

		case SnesMemoryType::VideoRam: _ppu->GetVideoRam()[address] = value; break;
		case SnesMemoryType::SpriteRam: _ppu->GetSpriteRam()[address] = value; break;
		case SnesMemoryType::CGRam: _ppu->GetCgRam()[address] = value; break;
		case SnesMemoryType::SpcRam: _spc->GetSpcRam()[address] = value; invalidateCache(); break;
		case SnesMemoryType::SpcRom: _spc->GetSpcRom()[address] = value; invalidateCache(); break;
			
		case SnesMemoryType::DspProgramRom: _cartridge->GetDsp()->DebugGetProgramRom()[address] = value; _cartridge->GetDsp()->BuildProgramCache(); break;
		case SnesMemoryType::DspDataRom: _cartridge->GetDsp()->DebugGetDataRom()[address] = value; break;
		case SnesMemoryType::DspDataRam: _cartridge->GetDsp()->DebugGetDataRam()[address] = value; break;

		case SnesMemoryType::Sa1InternalRam: _cartridge->GetSa1()->DebugGetInternalRam()[address] = value; invalidateCache(); break;
		case SnesMemoryType::GsuWorkRam: _cartridge->GetGsu()->DebugGetWorkRam()[address] = value; invalidateCache(); break;
		case SnesMemoryType::Cx4DataRam: _cartridge->GetCx4()->DebugGetDataRam()[address] = value; break;
		case SnesMemoryType::BsxPsRam: _cartridge->GetBsx()->DebugGetPsRam()[address] = value; break;
		case SnesMemoryType::BsxMemoryPack: _cartridge->GetBsxMemoryPack()->DebugGetMemoryPack()[address] = value; break;

		case SnesMemoryType::GbPrgRom:
		case SnesMemoryType::GbWorkRam:
		case SnesMemoryType::GbVideoRam:
		case SnesMemoryType::GbCartRam:
		case SnesMemoryType::GbHighRam:
			if(_cartridge->GetGameboy()) {
				_cartridge->GetGameboy()->DebugGetMemory(memoryType)[address] = value;
			}
			break;
	}
}

uint8_t MemoryDumper::GetMemoryValue(SnesMemoryType memoryType, uint32_t address, bool disableSideEffects)
{
	if(address >= GetMemorySize(memoryType)) {
		return 0;
	}

	switch(memoryType) {
		default: return 0;

		case SnesMemoryType::CpuMemory: return _memoryManager->Peek(address);
		case SnesMemoryType::SpcMemory: return _spc->DebugRead(address);
		case SnesMemoryType::Sa1Memory: return _cartridge->GetSa1()->GetMemoryMappings()->Peek(address);
		case SnesMemoryType::GsuMemory: return _cartridge->GetGsu()->GetMemoryMappings()->Peek(address);
		case SnesMemoryType::Cx4Memory: return _cartridge->GetCx4()->GetMemoryMappings()->Peek(address);
		case SnesMemoryType::GameboyMemory: return _cartridge->GetGameboy()->GetMemoryManager()->DebugRead(address);

		case SnesMemoryType::PrgRom: return _cartridge->DebugGetPrgRom()[address];
		case SnesMemoryType::WorkRam: return  _memoryManager->DebugGetWorkRam()[address];
		case SnesMemoryType::SaveRam: return _cartridge->DebugGetSaveRam()[address];

		case SnesMemoryType::VideoRam: return _ppu->GetVideoRam()[address];
		case SnesMemoryType::SpriteRam: return _ppu->GetSpriteRam()[address];
		case SnesMemoryType::CGRam: return _ppu->GetCgRam()[address];
		case SnesMemoryType::SpcRam: return _spc->GetSpcRam()[address];
		case SnesMemoryType::SpcRom: return _spc->GetSpcRom()[address];
		
		case SnesMemoryType::DspProgramRom: return _cartridge->GetDsp()->DebugGetProgramRom()[address];
		case SnesMemoryType::DspDataRom: return _cartridge->GetDsp()->DebugGetDataRom()[address];
		case SnesMemoryType::DspDataRam: return _cartridge->GetDsp()->DebugGetDataRam()[address];

		case SnesMemoryType::Sa1InternalRam: return _cartridge->GetSa1()->DebugGetInternalRam()[address];
		case SnesMemoryType::GsuWorkRam: return _cartridge->GetGsu()->DebugGetWorkRam()[address];
		case SnesMemoryType::Cx4DataRam: return _cartridge->GetCx4()->DebugGetDataRam()[address];
		case SnesMemoryType::BsxPsRam: return _cartridge->GetBsx()->DebugGetPsRam()[address];
		case SnesMemoryType::BsxMemoryPack: return _cartridge->GetBsxMemoryPack()->DebugGetMemoryPack()[address];

		case SnesMemoryType::GbPrgRom:
		case SnesMemoryType::GbWorkRam:
		case SnesMemoryType::GbVideoRam:
		case SnesMemoryType::GbCartRam:
		case SnesMemoryType::GbHighRam:
			return _cartridge->GetGameboy() ? _cartridge->GetGameboy()->DebugGetMemory(memoryType)[address] : 0;
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