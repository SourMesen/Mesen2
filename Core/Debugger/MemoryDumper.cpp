#include "pch.h"
#include "Debugger/Debugger.h"
#include "Shared/Emulator.h"
#include "SNES/SnesMemoryManager.h"
#include "SNES/Spc.h"
#include "SNES/Coprocessors/DSP/NecDsp.h"
#include "SNES/Coprocessors/SA1/Sa1.h"
#include "SNES/Coprocessors/CX4/Cx4.h"
#include "SNES/Coprocessors/GSU/Gsu.h"
#include "SNES/Coprocessors/ST018/St018.h"
#include "Gameboy/Gameboy.h"
#include "Gameboy/GbMemoryManager.h"
#include "SNES/Coprocessors/BSX/BsxCart.h"
#include "SNES/Coprocessors/BSX/BsxMemoryPack.h"
#include "SNES/SnesConsole.h"
#include "Debugger/MemoryDumper.h"
#include "SNES/BaseCartridge.h"
#include "NES/NesConsole.h"
#include "PCE/PceConsole.h"
#include "PCE/PceMemoryManager.h"
#include "SMS/SmsConsole.h"
#include "SMS/SmsVdp.h"
#include "SMS/SmsMemoryManager.h"
#include "GBA/GbaConsole.h"
#include "GBA/GbaMemoryManager.h"
#include "WS/WsConsole.h"
#include "WS/WsMemoryManager.h"
#include "Shared/Video/VideoDecoder.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/DebugBreakHelper.h"
#include "Debugger/DebugUtilities.h"
#include "Debugger/Disassembler.h"
#include "Debugger/CdlManager.h"

MemoryDumper::MemoryDumper(Debugger* debugger)
{
	_debugger = debugger;
	_emu = debugger->GetEmulator();

	IConsole* console = _debugger->GetConsole();
	//TODOv2 - generic code? (or at least rename SNES-specific members)
	if(SnesConsole* snes = dynamic_cast<SnesConsole*>(console)) {
		_spc = snes->GetSpc();
		_memoryManager = snes->GetMemoryManager();
		_cartridge = snes->GetCartridge();
		_gameboy = snes->GetCartridge()->GetGameboy();
	} else if(NesConsole* nes = dynamic_cast<NesConsole*>(console)) {
		_nesConsole = nes;
	} else if(Gameboy* gb = dynamic_cast<Gameboy*>(console)) {
		_gameboy = gb;
	} else if(PceConsole* pce = dynamic_cast<PceConsole*>(console)) {
		_pceConsole = pce;
	} else if(SmsConsole* sms = dynamic_cast<SmsConsole*>(console)) {
		_smsConsole = sms;
	} else if(GbaConsole* gba = dynamic_cast<GbaConsole*>(console)) {
		_gbaConsole = gba;
	} else if(WsConsole* ws = dynamic_cast<WsConsole*>(console)) {
		_wsConsole = ws;
	}

	for(int i = 0; i < DebugUtilities::GetMemoryTypeCount(); i++) {
		MemoryType memType = (MemoryType)i;
		if(memType != MemoryType::None) {
			_isMemorySupported[i] = _emu->GetMemory(memType).Memory != nullptr || _debugger->HasCpuType(DebugUtilities::ToCpuType(memType));
		}
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
	if(!_isMemorySupported[(int)type]) {
		return 0;
	}

	switch(type) {
		case MemoryType::SnesMemory: return 0x1000000;
		case MemoryType::SpcMemory: return 0x10000;
		case MemoryType::NecDspMemory: return _emu->GetMemory(MemoryType::DspProgramRom).Size;
		case MemoryType::Sa1Memory: return 0x1000000;
		case MemoryType::GsuMemory: return 0x1000000;
		case MemoryType::Cx4Memory: return 0x1000000;
		case MemoryType::St018Memory: return 0x20000;
		case MemoryType::GameboyMemory: return 0x10000;
		case MemoryType::NesMemory: return 0x10000;
		case MemoryType::NesPpuMemory: return 0x4000;
		case MemoryType::PceMemory: return 0x10000;
		case MemoryType::SmsMemory: return 0x10000;
		case MemoryType::GbaMemory: return 0x10000000;
		case MemoryType::WsMemory: return 0x100000;
		case MemoryType::SnesRegister: return 0x10000;
		case MemoryType::SmsPort: return 0x100;
		case MemoryType::WsPort: return 0x10000;
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

		case MemoryType::NecDspMemory:
			GetMemoryState(MemoryType::DspProgramRom, buffer);
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

		case MemoryType::St018Memory:
			if(_cartridge->GetSt018()) {
				for(int i = 0; i < 0x20000; i += 0x1000) {
					_cartridge->GetSt018()->PeekBlock(i, buffer + i);
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

		case MemoryType::PceMemory: {
			if(_pceConsole) {
				PceMemoryManager* memManager = _pceConsole->GetMemoryManager();
				for(int i = 0; i <= 0xFFFF; i++) {
					buffer[i] = memManager->DebugRead(i);
				}
			}
			break;
		}

		case MemoryType::SmsMemory: {
			if(_smsConsole) {
				SmsMemoryManager* memManager = _smsConsole->GetMemoryManager();
				for(int i = 0; i <= 0xFFFF; i++) {
					buffer[i] = memManager->DebugRead(i);
				}
			}
			break;
		}

		case MemoryType::GbaMemory: {
			if(_gbaConsole) {
				GbaMemoryManager* memManager = _gbaConsole->GetMemoryManager();
				for(int i = 0; i <= 0xFFFFFFF; i++) {
					buffer[i] = memManager->DebugRead(i);
				}
			}
			break;
		}

		case MemoryType::WsMemory: {
			if(_wsConsole) {
				WsMemoryManager* memManager = _wsConsole->GetMemoryManager();
				for(int i = 0; i <= 0xFFFFF; i++) {
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

void MemoryDumper::InternalSetMemoryValues(MemoryType originalMemoryType, uint32_t startAddress, uint8_t* data, uint32_t length, bool disableSideEffects, bool undoAllowed)
{
	uint32_t memSize = GetMemorySize(originalMemoryType);
	
	UndoBatch undoBatch = {};
	UndoEntry undoEntry = { MemoryType::None };

	Disassembler* disassembler = _debugger->GetDisassembler();

	for(uint32_t i = 0; i < length; i++) {
		uint32_t address = startAddress + i;
		if(address >= memSize) {
			break;
		}

		uint8_t value = data[i];

		MemoryType memoryType = originalMemoryType;
		if(disableSideEffects && DebugUtilities::IsRelativeMemory(memoryType)) {
			AddressInfo addr = { (int32_t)address, memoryType };
			addr = _debugger->GetAbsoluteAddress(addr);
			if(addr.Address < 0) {
				continue;
			}
			address = addr.Address;
			memoryType = addr.Type;
		}

		switch(memoryType) {
			case MemoryType::SnesMemory: _memoryManager->GetMemoryMappings()->DebugWrite(address, value); break;
			case MemoryType::SpcMemory: _spc->DebugWrite(address, value); break;
			case MemoryType::Sa1Memory: _cartridge->GetSa1()->GetMemoryMappings()->DebugWrite(address, value); break;
			case MemoryType::NecDspMemory: SetMemoryValue(MemoryType::DspProgramRom, address, value, disableSideEffects); return;
			case MemoryType::GsuMemory: _cartridge->GetGsu()->GetMemoryMappings()->DebugWrite(address, value); break;
			case MemoryType::Cx4Memory: _cartridge->GetCx4()->GetMemoryMappings()->DebugWrite(address, value); break;
			case MemoryType::St018Memory: _cartridge->GetSt018()->DebugWrite(address, value); break;
			case MemoryType::GameboyMemory: _gameboy->GetMemoryManager()->DebugWrite(address, value); break;
			case MemoryType::NesMemory: _nesConsole->DebugWrite(address, value, disableSideEffects); break;
			case MemoryType::NesPpuMemory: _nesConsole->DebugWriteVram(address, value); break;
			case MemoryType::PceMemory: _pceConsole->GetMemoryManager()->DebugWrite(address, value); break;
			case MemoryType::SmsMemory: _smsConsole->GetMemoryManager()->DebugWrite(address, value); break;
			case MemoryType::GbaMemory: _gbaConsole->GetMemoryManager()->DebugWrite(address, value); break;
			case MemoryType::WsMemory: _wsConsole->GetMemoryManager()->DebugWrite(address, value); break;
			case MemoryType::SpcDspRegisters: _spc->DebugWriteDspReg(address, value); break;

			default:
				uint8_t* src = GetMemoryBuffer(memoryType);
				if(src) {
					if(undoAllowed) {
						if(undoEntry.MemType != memoryType) {
							if(undoEntry.OriginalData.size() > 0) {
								undoBatch.Entries.push_back(undoEntry);
							}
							undoEntry = { memoryType, address };
						}

						uint8_t originalValue = src[address];
						undoEntry.OriginalData.push_back(originalValue);
					}

					//TODOv2 find a cleaner way to implement this
					//Prevent invalid memory values
					switch(memoryType) {
						case MemoryType::SnesCgRam: src[address] = (address & 0x01) ? (value & 0x7F) : value; break;
						case MemoryType::NesSpriteRam: case MemoryType::NesSecondarySpriteRam: src[address] = (address & 0x03) == 0x02 ? (value & 0xE3) : value; break;
						case MemoryType::NesPaletteRam: src[address] = value & 0x3F; break;
						case MemoryType::PcePaletteRam: src[address] = (address & 0x01) ? (value & 0x01) : value; break;
						case MemoryType::SmsPaletteRam: _smsConsole->GetVdp()->DebugWritePalette(address, value); break;

						default:
							src[address] = value;

							AddressInfo addr = { (int32_t)address, memoryType };
							disassembler->InvalidateCache(addr, DebugUtilities::ToCpuType(memoryType));
							break;
					}
				}
				break;
		}
	}

	if(undoAllowed && undoEntry.MemType != MemoryType::None) {
		undoBatch.Entries.push_back(undoEntry);

		auto lock = _undoLock.AcquireSafe();
		_undoHistory.push_back(undoBatch);
		if(_undoHistory.size() > 200) {
			_undoHistory.pop_front();
		}
	}
}

void MemoryDumper::SetMemoryValues(MemoryType memoryType, uint32_t address, uint8_t* data, uint32_t length)
{
	DebugBreakHelper helper(_debugger);
	InternalSetMemoryValues(memoryType, address, data, length, true, true);
}

void MemoryDumper::SetMemoryValue(MemoryType memoryType, uint32_t address, uint8_t value, bool disableSideEffects)
{
	InternalSetMemoryValues(memoryType, address, &value, 1, disableSideEffects, true);
}

void MemoryDumper::GetMemoryValues(MemoryType memoryType, uint32_t start, uint32_t end, uint8_t* output)
{
	int x = 0;
	uint32_t size = GetMemorySize(memoryType);
	for(uint32_t i = start; i <= end && i < size; i++) {
		output[x++] = InternalGetMemoryValue(memoryType, i);
	}

	if(end >= size) {
		memset(output + x, 0, end - start - x + 1);
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
		case MemoryType::SnesMemory: return _memoryManager->Peek(address);
		case MemoryType::SpcMemory: return _spc->DebugRead(address);
		case MemoryType::Sa1Memory: return _cartridge->GetSa1()->GetMemoryMappings()->Peek(address);
		case MemoryType::NecDspMemory: return GetMemoryValue(MemoryType::DspProgramRom, address);
		case MemoryType::GsuMemory: return _cartridge->GetGsu()->GetMemoryMappings()->Peek(address);
		case MemoryType::Cx4Memory: return _cartridge->GetCx4()->GetMemoryMappings()->Peek(address);
		case MemoryType::St018Memory: return _cartridge->GetSt018()->DebugRead(address);
		case MemoryType::GameboyMemory: return _gameboy->GetMemoryManager()->DebugRead(address);
		case MemoryType::NesMemory: return _nesConsole->DebugRead(address);
		case MemoryType::NesPpuMemory: return _nesConsole->DebugReadVram(address);
		case MemoryType::PceMemory: return _pceConsole->GetMemoryManager()->DebugRead(address);
		case MemoryType::SmsMemory: return _smsConsole->GetMemoryManager()->DebugRead(address);
		case MemoryType::SmsPort: return _smsConsole->GetMemoryManager()->DebugReadPort(address);
		case MemoryType::GbaMemory: return _gbaConsole->GetMemoryManager()->DebugRead(address);
		case MemoryType::WsMemory: return _wsConsole->GetMemoryManager()->DebugRead(address);
		case MemoryType::WsPort: return _wsConsole->GetMemoryManager()->DebugReadPort<uint8_t>(address);

		default:
			uint8_t* src = GetMemoryBuffer(memoryType);
			return src ? src[address] : 0;
	}
}

uint16_t MemoryDumper::GetMemoryValue16(MemoryType memoryType, uint32_t address, bool disableSideEffects)
{
	uint32_t memorySize = GetMemorySize(memoryType);
	uint8_t lsb = GetMemoryValue(memoryType, address);
	uint8_t msb = GetMemoryValue(memoryType, address + 1 >= memorySize ? 0 : address + 1);
	return (msb << 8) | lsb;
}

uint32_t MemoryDumper::GetMemoryValue32(MemoryType memoryType, uint32_t address, bool disableSideEffects)
{
	uint32_t memorySize = GetMemorySize(memoryType);
	uint8_t b0 = GetMemoryValue(memoryType, address);
	uint8_t b1 = GetMemoryValue(memoryType, address + 1 >= memorySize ? 0 : address + 1);
	uint8_t b2 = GetMemoryValue(memoryType, address + 2 >= memorySize ? 0 : address + 2);
	uint8_t b3 = GetMemoryValue(memoryType, address + 3 >= memorySize ? 0 : address + 3);
	return (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
}

void MemoryDumper::SetMemoryValue16(MemoryType memoryType, uint32_t address, uint16_t value, bool disableSideEffects)
{
	DebugBreakHelper helper(_debugger);
	SetMemoryValue(memoryType, address, (uint8_t)value, disableSideEffects);
	SetMemoryValue(memoryType, address + 1, (uint8_t)(value >> 8), disableSideEffects);
}

void MemoryDumper::SetMemoryValue32(MemoryType memoryType, uint32_t address, uint32_t value, bool disableSideEffects)
{
	DebugBreakHelper helper(_debugger);
	SetMemoryValue(memoryType, address, (uint8_t)value, disableSideEffects);
	SetMemoryValue(memoryType, address + 1, (uint8_t)(value >> 8), disableSideEffects);
	SetMemoryValue(memoryType, address + 2, (uint8_t)(value >> 16), disableSideEffects);
	SetMemoryValue(memoryType, address + 3, (uint8_t)(value >> 24), disableSideEffects);
}

bool MemoryDumper::HasUndoHistory()
{
	auto lock = _undoLock.AcquireSafe();
	return _undoHistory.size() > 0;
}

void MemoryDumper::PerformUndo()
{
	auto lock = _undoLock.AcquireSafe();
	if(!_undoHistory.empty()) {
		DebugBreakHelper helper(_debugger);
		UndoBatch& batch = _undoHistory.back();
		for(auto entry : batch.Entries) {
			InternalSetMemoryValues(entry.MemType, entry.StartAddress, entry.OriginalData.data(), (uint32_t)entry.OriginalData.size(), true, false);
		}
		_undoHistory.pop_back();
		_debugger->GetCdlManager()->RefreshCodeCache();
	}
}
