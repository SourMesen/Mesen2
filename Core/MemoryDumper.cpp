#include "stdafx.h"
#include "Debugger.h"
#include "MemoryManager.h"
#include "Ppu.h"
#include "Spc.h"
#include "NecDsp.h"
#include "Sa1.h"
#include "Gsu.h"
#include "MemoryDumper.h"
#include "BaseCartridge.h"
#include "VideoDecoder.h"
#include "DebugTypes.h"

MemoryDumper::MemoryDumper(shared_ptr<Ppu> ppu, shared_ptr<Spc> spc, shared_ptr<MemoryManager> memoryManager, shared_ptr<BaseCartridge> cartridge)
{
	_ppu = ppu;
	_spc = spc;
	_memoryManager = memoryManager;
	_cartridge = cartridge;
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
			break;
		
		case SnesMemoryType::PrgRom: memcpy(_cartridge->DebugGetPrgRom(), buffer, length); break;
		case SnesMemoryType::WorkRam: memcpy(_memoryManager->DebugGetWorkRam(), buffer, length); break;
		case SnesMemoryType::SaveRam: memcpy(_cartridge->DebugGetSaveRam(), buffer, length); break;
		case SnesMemoryType::VideoRam: memcpy(_ppu->GetVideoRam(), buffer, length); break;
		case SnesMemoryType::SpriteRam: memcpy(_ppu->GetSpriteRam(), buffer, length); break;
		case SnesMemoryType::CGRam: memcpy(_ppu->GetCgRam(), buffer, length); break;
		case SnesMemoryType::SpcRam: memcpy(_spc->GetSpcRam(), buffer, length); break;
		case SnesMemoryType::SpcRom: memcpy(_spc->GetSpcRom(), buffer, length); break;

		case SnesMemoryType::DspProgramRom: memcpy(_cartridge->GetDsp()->DebugGetProgramRom(), buffer, length); break;
		case SnesMemoryType::DspDataRom: memcpy(_cartridge->GetDsp()->DebugGetDataRom(), buffer, length); break;
		case SnesMemoryType::DspDataRam: memcpy(_cartridge->GetDsp()->DebugGetDataRam(), buffer, length); break;

		case SnesMemoryType::Sa1InternalRam: memcpy(_cartridge->GetSa1()->DebugGetInternalRam(), buffer, length); break;
		
		case SnesMemoryType::GsuWorkRam:  memcpy(_cartridge->GetGsu()->DebugGetWorkRam(), buffer, length); break;
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
		
		case SnesMemoryType::Sa1InternalRam: return _cartridge->GetSa1() ? _cartridge->GetSa1()->DebugGetInternalRamSize() : 0;;
		
		case SnesMemoryType::GsuWorkRam: return _cartridge->GetGsu() ? _cartridge->GetGsu()->DebugGetWorkRamSize() : 0;;
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
			for(int i = 0; i <= 0xFFFFFF; i+=0x1000) {
				_cartridge->GetSa1()->GetMemoryMappings()->PeekBlock(i, buffer + i);
			}
			break;

		case SnesMemoryType::GsuMemory:
			for(int i = 0; i <= 0xFFFFFF; i += 0x1000) {
				_cartridge->GetGsu()->GetMemoryMappings()->PeekBlock(i, buffer + i);
			}
			break;

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
	}
}

void MemoryDumper::SetMemoryValues(SnesMemoryType memoryType, uint32_t address, uint8_t* data, uint32_t length)
{
	for(uint32_t i = 0; i < length; i++) {
		SetMemoryValue(memoryType, address+i, data[i], true);
	}
}

void MemoryDumper::SetMemoryValue(SnesMemoryType memoryType, uint32_t address, uint8_t value, bool disableSideEffects)
{
	if(address >= GetMemorySize(memoryType)) {
		return;
	}

	switch(memoryType) {
		default: break;

		//TODO: Avoid side effects
		case SnesMemoryType::CpuMemory: _memoryManager->Write(address, value, MemoryOperationType::Write); break;
		case SnesMemoryType::SpcMemory: _spc->DebugWrite(address, value); break;
		case SnesMemoryType::Sa1Memory: _cartridge->GetSa1()->WriteSa1(address, value, MemoryOperationType::Write); break;
		case SnesMemoryType::GsuMemory: _cartridge->GetGsu()->WriteGsu(address, value, MemoryOperationType::Write); break;

		case SnesMemoryType::PrgRom: _cartridge->DebugGetPrgRom()[address] = value; break;
		case SnesMemoryType::WorkRam: _memoryManager->DebugGetWorkRam()[address] = value; break;
		case SnesMemoryType::SaveRam: _cartridge->DebugGetSaveRam()[address] = value; break;

		case SnesMemoryType::VideoRam: _ppu->GetVideoRam()[address] = value;
		case SnesMemoryType::SpriteRam: _ppu->GetSpriteRam()[address] = value; break;
		case SnesMemoryType::CGRam: _ppu->GetCgRam()[address] = value; break;
		case SnesMemoryType::SpcRam: _spc->GetSpcRam()[address] = value; break;
		case SnesMemoryType::SpcRom: _spc->GetSpcRom()[address] = value; break;
			
		case SnesMemoryType::DspProgramRom: _cartridge->GetDsp()->DebugGetProgramRom()[address] = value;
		case SnesMemoryType::DspDataRom: _cartridge->GetDsp()->DebugGetDataRom()[address] = value;
		case SnesMemoryType::DspDataRam: _cartridge->GetDsp()->DebugGetDataRam()[address] = value;

		case SnesMemoryType::Sa1InternalRam: _cartridge->GetSa1()->DebugGetInternalRam()[address] = value;

		case SnesMemoryType::GsuWorkRam: _cartridge->GetGsu()->DebugGetWorkRam()[address] = value;
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
	SetMemoryValue(memoryType, address, (uint8_t)value, disableSideEffects);
	SetMemoryValue(memoryType, address + 1, (uint8_t)(value >> 8), disableSideEffects);
}