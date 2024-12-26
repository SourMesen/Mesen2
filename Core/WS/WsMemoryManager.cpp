#include "pch.h"
#include "WS/WsMemoryManager.h"
#include "WS/WsConsole.h"
#include "WS/WsDmaController.h"
#include "WS/WsControlManager.h"
#include "WS/WsTimer.h"
#include "WS/WsSerial.h"
#include "WS/WsEeprom.h"
#include "WS/Carts/WsCart.h"
#include "Shared/MessageManager.h"
#include "Shared/Emulator.h"
#include "Utilities/Serializer.h"

void WsMemoryManager::Init(Emulator* emu, WsConsole* console, WsCpu* cpu, WsPpu* ppu, WsControlManager* controlManager, WsCart* cart, WsTimer* timer, WsDmaController* dmaController, WsEeprom* eeprom, WsApu* apu, WsSerial* serial)
{
	_emu = emu;
	_console = console;
	_cpu = cpu;
	_cart = cart;
	_ppu = ppu;
	_apu = apu;
	_timer = timer;
	_dmaController = dmaController;
	_controlManager = controlManager;
	_eeprom = eeprom;
	_serial = serial;
	
	_workRamSize = _emu->GetMemory(MemoryType::WsWorkRam).Size;

	_prgRom = (uint8_t*)_emu->GetMemory(MemoryType::WsPrgRom).Memory;
	_prgRomSize = _emu->GetMemory(MemoryType::WsPrgRom).Size;

	_saveRam = (uint8_t*)_emu->GetMemory(MemoryType::WsCartRam).Memory;
	_saveRamSize = _emu->GetMemory(MemoryType::WsCartRam).Size;

	_bootRom = (uint8_t*)_emu->GetMemory(MemoryType::WsBootRom).Memory;
	_bootRomSize = _emu->GetMemory(MemoryType::WsBootRom).Size;

	RefreshMappings();
}

void WsMemoryManager::RefreshMappings()
{
	_cart->RefreshMappings();
	if(_state.ColorEnabled) {
		Map(0, 0xFFFF, MemoryType::WsWorkRam, 0, false);
	} else {
		Unmap(0x4000, 0xFFFF);
		Map(0, 0x3FFF, MemoryType::WsWorkRam, 0, false);
	}

	if(!_state.BootRomDisabled) {
		Map(0x100000 - _bootRomSize, 0xFFFFF, MemoryType::WsBootRom, 0, true);
	}
}

void WsMemoryManager::Map(uint32_t start, uint32_t end, MemoryType type, uint32_t offset, bool readonly)
{
	uint8_t* src = (uint8_t*)_emu->GetMemory(type).Memory;
	uint32_t size = _emu->GetMemory(type).Size;

	if(size > 0) {
		while(offset >= size) {
			offset -= size;
		}

		src += offset;
		for(uint32_t i = start; i < end; i += 0x1000) {
			_reads[i >> 12] = src;
			_writes[i >> 12] = readonly ? nullptr : src;

			if(src) {
				src += 0x1000;
				offset = (offset + 0x1000);
				if(offset >= size) {
					offset = 0;
					src = (uint8_t*)_emu->GetMemory(type).Memory;
				}
			}
		}
	} else {
		Unmap(start, end);
	}
}

void WsMemoryManager::Unmap(uint32_t start, uint32_t end)
{
	for(uint32_t i = start; i < end; i += 0x1000) {
		_reads[i >> 12] = nullptr;
		_writes[i >> 12] = nullptr;
	}
}

uint8_t WsMemoryManager::DebugRead(uint32_t addr)
{
	uint8_t* handler = _reads[addr >> 12];
	if(handler) {
		return handler[addr & 0xFFF];
	}
	return 0;
}

void WsMemoryManager::DebugWrite(uint32_t addr, uint8_t value)
{
	uint8_t* handler = _writes[addr >> 12];
	if(handler) {
		handler[addr & 0xFFF] = value;
	}
}

template<typename T>
T WsMemoryManager::DebugCpuRead(uint16_t seg, uint16_t offset)
{
	uint32_t addr = (seg << 4) + offset;
	if constexpr(std::is_same<T, uint16_t>::value) {
		bool splitReads = !IsWordBus(addr) || (addr & 0x01);
		if(splitReads) {
			uint8_t lo = DebugCpuRead<uint8_t>(seg, offset);
			uint8_t hi = DebugCpuRead<uint8_t>(seg, offset + 1);
			return lo | (hi << 8);
		} else {
			uint8_t lo = DebugRead(addr);
			addr = (seg << 4) + (uint16_t)(offset + 1);
			uint8_t hi = DebugRead(addr);
			return lo | (hi << 8);
		}
	} else {
		return DebugRead(addr);
	}
}

template<typename T>
T WsMemoryManager::ReadPort(uint16_t port)
{
	if constexpr(std::is_same<T, uint16_t>::value) {
		bool splitReads = !IsWordPort(port) || (port & 0x01);
		if(splitReads) {
			uint8_t lo = ReadPort<uint8_t>(port);
			uint8_t hi = ReadPort<uint8_t>(port + 1);
			return lo | (hi << 8);
		} else {
			Exec();

			uint16_t value;
			if(IsUnmappedPort(port)) {
				//TODOWS open bus
				value = 0x9090;
			} else {
				uint8_t lo = InternalReadPort(port, true);
				uint8_t hi = InternalReadPort(port + 1, true);
				value = lo | (hi << 8);
			}

			_emu->ProcessMemoryAccess<CpuType::Ws, MemoryType::WsPort, MemoryOperationType::Read>(port, value);
			return value;
		}
	} else {
		Exec();

		//TODOWS open bus
		uint8_t value = IsUnmappedPort(port) ? 0x90 : InternalReadPort(port, false);
		_emu->ProcessMemoryAccess<CpuType::Ws, MemoryType::WsPort, MemoryOperationType::Read>(port, value);
		return value;
	}
}

template<typename T>
void WsMemoryManager::WritePort(uint16_t port, T value)
{
	if constexpr(std::is_same<T, uint16_t>::value) {
		bool splitWrites = !IsWordPort(port) || (port & 0x01);
		if(splitWrites) {
			WritePort<uint8_t>(port, value);
			WritePort<uint8_t>(port + 1, value >> 8);
		} else {
			Exec();
			_emu->ProcessMemoryAccess<CpuType::Ws, MemoryType::WsPort, MemoryOperationType::Write>(port, value);

			if(!IsUnmappedPort(port)) {
				InternalWritePort(port, value, true);
				InternalWritePort(port + 1, value >> 8, true);
			}
		}
	} else {
		Exec();
		_emu->ProcessMemoryAccess<CpuType::Ws, MemoryType::WsPort, MemoryOperationType::Write>(port, value);

		if(!IsUnmappedPort(port)) {
			InternalWritePort(port, value, false);
		}
	}
}

template<typename T>
T WsMemoryManager::DebugReadPort(uint16_t port)
{
	if constexpr(std::is_same<T, uint16_t>::value) {
		uint8_t lo = DebugReadPort<uint8_t>(port);
		uint8_t hi = DebugReadPort<uint8_t>(port + 1);
		return lo | (hi << 8);
	} else {
		if(IsUnmappedPort(port)) {
			//TODOWS open bus
			return 0x90;
		}

		if(port == 0xB1 || port == 0xB3 || port == 0xB5) {
			//TODOWS implement peek to avoid side-effects for these ports
			return 0;
		}

		return InternalReadPort(port, false);
	}
}

bool WsMemoryManager::IsUnmappedPort(uint16_t port)
{
	return (port & 0x100) || (port >= 0x100 && (port & 0xFF) >= 0xB7);
}

uint8_t WsMemoryManager::InternalReadPort(uint16_t port, bool isWordAccess)
{
	port &= 0xFF;

	if(port <= 0x3F) {
		return _ppu->ReadPort(port);
	} else if(port >= 0x40 && port <= 0x53 && _state.ColorEnabled) {
		return _dmaController->ReadPort(port);
	} else if(port >= 0x64 && port <= 0x6B && _state.ColorEnabled) {
		return _apu->Read(port); //HyperVoice
	} else if(port >= 0x70 && port <= 0x77 && _console->GetModel() == WsModel::SwanCrystal) {
		return _ppu->ReadLcdConfigPort(port);
	} else if(port >= 0x80 && port <= 0x9F) {
		return _apu->Read(port);
	} else if(port >= 0xA2 && port <= 0xAB) {
		return _timer->ReadPort(port);
	} else if(port >= 0xBA && port <= 0xBF) {
		if(_console->GetModel() != WsModel::Monochrome || isWordAccess || !(port & 0x01)) {
			return _eeprom->ReadPort(port - 0xBA);
		} else {
			//TODOWS open bus
			return 0x90;
		}
	} else if(port >= 0xC0) {
		return _cart->ReadPort(port);
	} else {
		switch(port) {
			case 0x60: return _state.SystemControl2;
			case 0x62: return _console->GetModel() == WsModel::SwanCrystal ? 0x80 : 0;
			case 0xA0:
				return (
					(_state.BootRomDisabled ? 0x01 : 0) |
					(_console->GetModel() == WsModel::Monochrome ? 0 : 0x02) |
					(_state.CartWordBus ? 0x04 : 0) |
					(_state.SlowRom ? 0x08 : 0) |
					0x80 //mbc authentication?
					);

			case 0xB0: return GetIrqVector();
			case 0xB1: return _serial->Read(port);
			case 0xB2: return _state.EnabledIrqs;
			case 0xB3: return _serial->Read(port);
			case 0xB4: return GetActiveIrqs();

			case 0xB5:
				_controlManager->SetInputReadFlag();
				return _controlManager->Read();

			case 0xB7: return _state.EnableLowBatteryNmi ? 0x10 : 0;
			default:
				//TODOWS open bus
				LogDebug("[Debug] Read - missing handler: $" + HexUtilities::ToHex(port));
				return 0x90;
		}
	}
}

void WsMemoryManager::InternalWritePort(uint16_t port, uint8_t value, bool isWordAccess)
{
	port &= 0xFF;

	if(port <= 0x3F) {
		_ppu->WritePort(port, value);
	} else if(port >= 0x40 && port <= 0x53 && _state.ColorEnabled) {
		return _dmaController->WritePort(port, value);
	} else if(port >= 0x64 && port <= 0x6B && _state.ColorEnabled) {
		_apu->Write(port, value); //HyperVoice
	} else if(port >= 0x70 && port <= 0x77 && !_state.BootRomDisabled && _console->GetModel() == WsModel::SwanCrystal) {
		_ppu->WriteLcdConfigPort(port, value);
	} else if(port >= 0x80 && port <= 0x9F) {
		_apu->Write(port, value);
	} else if(port >= 0xA2 && port <= 0xAB) {
		_timer->WritePort(port, value);
	} else if(port >= 0xBA && port <= 0xBF) {
		if(_console->GetModel() != WsModel::Monochrome || isWordAccess || !(port & 0x01)) {
			_eeprom->WritePort(port - 0xBA, value);
		}
	} else if(port >= 0xC0) {
		_cart->WritePort(port, value);
	} else {
		switch(port) {
			case 0x60:
				if(_console->GetModel() != WsModel::Monochrome) {
					_state.SystemControl2 = value & 0xEF;
					_state.ColorEnabled = value & 0x80;
					_state.Enable4bpp = value & 0x40;
					_state.Enable4bppPacked = value & 0x20;
					_state.SlowSram = value & 0x02;
					_state.SlowPort = value & 0x08;
					RefreshMappings();
					WsVideoMode mode = WsVideoMode::Monochrome;
					if(_state.ColorEnabled) {
						mode = WsVideoMode::Color2bpp;
						if(_state.Enable4bpp) {
							mode = WsVideoMode::Color4bpp;
							if(_state.Enable4bppPacked) {
								mode = WsVideoMode::Color4bppPacked;
							}
						}
					}
					_ppu->SetVideoMode(mode);
				}
				break;

			case 0x62:
				if(value & 0x01) {
					MessageManager::DisplayMessage("WS", "Power off.");
				}
				break;

			case 0xA0:
				_state.BootRomDisabled = (value & 0x01) || _state.BootRomDisabled;
				_state.CartWordBus = value & 0x04;
				_state.SlowRom = value & 0x08;
				RefreshMappings();
				break;

			case 0xB0: _state.IrqVectorOffset = value & 0xF8; break;
			case 0xB1: _serial->Write(port, value); break;
			case 0xB2: _state.EnabledIrqs = value; break;
			case 0xB3: _serial->Write(port, value); break;
			case 0xB5: _controlManager->Write(value); break;
			case 0xB6: _state.ActiveIrqs &= ~value; break;
			case 0xB7: _state.EnableLowBatteryNmi = value & 0x10; break;

			default:
				LogDebug("[Debug] Write - missing handler: $" + HexUtilities::ToHex(port) + "  = " + HexUtilities::ToHex(value));
				break;
		}
	}
}

bool WsMemoryManager::IsWordBus(uint32_t addr)
{
	switch(addr >> 16) {
		case 0: return true;
		case 1: return false;
		default: return _state.CartWordBus;
	}
}

uint8_t WsMemoryManager::GetWaitStates(uint32_t addr)
{
	switch(addr >> 16) {
		case 0: return 1;
		case 1: return 1 + (uint8_t)_state.SlowSram;
		default: return 1 + (uint8_t)_state.SlowRom;
	}
}

bool WsMemoryManager::IsWordPort(uint16_t port)
{
	return port < 0xC0;
}

uint8_t WsMemoryManager::GetPortWaitStates(uint16_t port)
{
	return 1 + (uint8_t)(_state.SlowPort && port >= 0xC0 && port <= 0xFF);
}

void WsMemoryManager::SetIrqSource(WsIrqSource src)
{
	if(_state.EnabledIrqs & (uint8_t)src) {
		_state.ActiveIrqs |= (uint8_t)src;
	}
}

void WsMemoryManager::ClearIrqSource(WsIrqSource src)
{
	_state.ActiveIrqs &= (uint8_t)src;
}

uint8_t WsMemoryManager::GetActiveIrqs()
{
	if(_serial->HasSendIrq() && (_state.EnabledIrqs & (uint8_t)WsIrqSource::UartSendReady)) {
		_state.ActiveIrqs |= (uint8_t)WsIrqSource::UartSendReady;
	}
	return _state.ActiveIrqs;
}

uint8_t WsMemoryManager::GetIrqVector()
{
	uint8_t activeIrqs = GetActiveIrqs();
	for(int i = 7; i >= 0; i--) {
		if(activeIrqs & (1 << i)) {
			return _state.IrqVectorOffset + i;
		}
	}
	return _state.IrqVectorOffset;
}

AddressInfo WsMemoryManager::GetAbsoluteAddress(uint32_t relAddr)
{
	if(relAddr < _workRamSize) {
		return { (int)(relAddr & (_workRamSize - 1)), MemoryType::WsWorkRam };
	}

	uint8_t* ptr = _reads[relAddr >> 12];
	if(ptr >= _prgRom && ptr < _prgRom + _prgRomSize) {
		return { (int)(ptr - _prgRom + (relAddr & 0xFFF)), MemoryType::WsPrgRom };
	} else if(ptr >= _saveRam && ptr < _saveRam + _saveRamSize) {
		return { (int)(ptr - _saveRam + (relAddr & 0xFFF)), MemoryType::WsCartRam };
	} else if(ptr >= _bootRom && ptr < _bootRom + _bootRomSize) {
		return { (int)(ptr - _bootRom + (relAddr & 0xFFF)), MemoryType::WsBootRom };
	}

	return { -1, MemoryType::None };
}

int WsMemoryManager::GetRelativeAddress(AddressInfo& absAddress)
{
	switch(absAddress.Type) {
		case MemoryType::WsPrgRom:
		case MemoryType::WsCartRam:
		case MemoryType::WsBootRom: {
			//Use the closest mirror to the current code segment
			uint8_t startBank = (_cpu->GetState().CS >> 12) << 4;
			for(int32_t i = 0; i < 256; i++) {
				uint8_t bank = (uint8_t)(startBank + i);
				AddressInfo blockAddr = GetAbsoluteAddress(bank * 0x1000);
				if(blockAddr.Type == absAddress.Type && (blockAddr.Address & ~0xFFF) == (absAddress.Address & ~0xFFF)) {
					return (bank << 12) | (absAddress.Address & 0xFFF);
				}
			}
			return -1;
		}
		case MemoryType::WsWorkRam:
			return absAddress.Address & (_workRamSize - 1);
	}

	return -1;
}

void WsMemoryManager::Serialize(Serializer& s)
{
	SV(_state.ActiveIrqs);
	SV(_state.EnabledIrqs);
	SV(_state.IrqVectorOffset);
	SV(_state.SystemControl2);
	SV(_state.ColorEnabled);
	SV(_state.Enable4bpp);
	SV(_state.Enable4bppPacked);
	SV(_state.BootRomDisabled);
	SV(_state.SlowPort);
	SV(_state.SlowRom);
	SV(_state.SlowSram);
	SV(_state.CartWordBus);
	SV(_state.EnableLowBatteryNmi);

	if(!s.IsSaving()) {
		RefreshMappings();
	}
}

template uint8_t WsMemoryManager::ReadPort(uint16_t port);
template uint16_t WsMemoryManager::ReadPort(uint16_t port);
template uint8_t WsMemoryManager::DebugReadPort(uint16_t port);
template uint16_t WsMemoryManager::DebugReadPort(uint16_t port);

template void WsMemoryManager::WritePort(uint16_t port, uint8_t value);
template void WsMemoryManager::WritePort(uint16_t port, uint16_t value);

template uint8_t WsMemoryManager::DebugCpuRead(uint16_t seg, uint16_t offset);
template uint16_t WsMemoryManager::DebugCpuRead(uint16_t seg, uint16_t offset);
