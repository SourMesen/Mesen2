#include "stdafx.h"
#include "Emulator.h"
#include "Gameboy.h"
#include "GbMemoryManager.h"
#include "GbPpu.h"
#include "GbApu.h"
#include "GbTimer.h"
#include "GbTypes.h"
#include "Carts/GbCart.h"
#include "GbDmaController.h"
#include "EmuSettings.h"
#include "SNES/Coprocessors/SGB/SuperGameboy.h"
#include "SNES/ControlManager.h"
#include "SNES/Input/SnesController.h"
#include "MessageManager.h"
#include "Utilities/VirtualFile.h"
#include "Utilities/Serializer.h"
#include "Utilities/HexUtilities.h"
#include "MemoryOperationType.h"

void GbMemoryManager::Init(Emulator* emu, Gameboy* gameboy, GbCart* cart, GbPpu* ppu, GbApu* apu, GbTimer* timer, GbDmaController* dmaController)
{
	_highRam = gameboy->DebugGetMemory(SnesMemoryType::GbHighRam);

	_emu = emu;
	_apu = apu;
	_ppu = ppu;
	_gameboy = gameboy;
	_cart = cart;
	_timer = timer;
	_dmaController = dmaController;
	_controlManager = gameboy->GetControlManager().get();
	_settings = _emu->GetSettings().get();

	memset(_reads, 0, sizeof(_reads));
	memset(_writes, 0, sizeof(_writes));

	_state = {};
	_state.CgbWorkRamBank = 1;
	_state.DisableBootRom = false;
	_state.CycleCount = 8; //Makes boot_sclk_align serial test pass

	MapRegisters(0x8000, 0x9FFF, RegisterAccess::ReadWrite);
	MapRegisters(0xFE00, 0xFFFF, RegisterAccess::ReadWrite);

	_cart->InitCart();
	RefreshMappings();
}

GbMemoryManager::~GbMemoryManager()
{
}

GbMemoryManagerState GbMemoryManager::GetState()
{
	return _state;
}

void GbMemoryManager::RefreshMappings()
{
	Map(0xC000, 0xCFFF, GbMemoryType::WorkRam, 0, false);
	Map(0xD000, 0xDFFF, GbMemoryType::WorkRam, _state.CgbWorkRamBank * 0x1000, false);
	Map(0xE000, 0xEFFF, GbMemoryType::WorkRam, 0, false);
	Map(0xF000, 0xFFFF, GbMemoryType::WorkRam, _state.CgbWorkRamBank * 0x1000, false);

	_cart->RefreshMappings();

	if(!_state.DisableBootRom) {
		Map(0x0000, 0x00FF, GbMemoryType::BootRom, 0, true);
		if(_gameboy->IsCgb()) {
			Map(0x0200, 0x08FF, GbMemoryType::BootRom, 0x200, true);
		}
	}
}

void GbMemoryManager::Exec()
{
	_state.CycleCount += 2;
	_state.ApuCycleCount += _state.CgbHighSpeed ? 1 : 2;
	_timer->Exec();
	_ppu->Exec();
	if((_state.CycleCount & 0x03) == 0) {
		_dmaController->Exec();
	}

	if(_state.SerialBitCount && (_state.CycleCount & 0x1FF) == 0) {
		_state.SerialData = (_state.SerialData << 1) | 0x01;
		if(--_state.SerialBitCount == 0) {
			//"It will be notified that the transfer is complete in two ways:
			//SC's Bit 7 will be cleared"
			_state.SerialControl &= 0x7F;

			//"and the Serial Interrupt handler will be called"
			RequestIrq(GbIrqSource::Serial);
		}
	}
}

void GbMemoryManager::MapRegisters(uint16_t start, uint16_t end, RegisterAccess access)
{
	for(int i = start; i < end; i += 0x100) {
		_state.IsReadRegister[i >> 8] = ((int)access & (int)RegisterAccess::Read) != 0;
		_state.IsWriteRegister[i >> 8] = ((int)access & (int)RegisterAccess::Write) != 0;
	}
}

void GbMemoryManager::Map(uint16_t start, uint16_t end, GbMemoryType type, uint32_t offset, bool readonly)
{
	uint8_t* src = _gameboy->DebugGetMemory((SnesMemoryType)type);
	uint32_t size = _gameboy->DebugGetMemorySize((SnesMemoryType)type);

	if(size > 0) {
		while(offset >= size) {
			offset -= size;
		}
		
		src += offset;
		for(int i = start; i < end; i += 0x100) {
			_reads[i >> 8] = src;
			_writes[i >> 8] = readonly ? nullptr : src;

			_state.MemoryType[i >> 8] = type;
			_state.MemoryOffset[i >> 8] = offset;
			_state.MemoryAccessType[i >> 8] = readonly ? RegisterAccess::Read : RegisterAccess::ReadWrite;

			if(src) {
				src += 0x100;
				offset = (offset + 0x100);
				if(offset >= size) {
					offset = 0;
					src = _gameboy->DebugGetMemory((SnesMemoryType)type);
				}
			}
		}
	} else {
		Unmap(start, end);
	}
}

void GbMemoryManager::Unmap(uint16_t start, uint16_t end)
{
	for(int i = start; i < end; i += 0x100) {
		_reads[i >> 8] = nullptr;
		_writes[i >> 8] = nullptr;

		_state.MemoryType[i >> 8] = GbMemoryType::None;
		_state.MemoryOffset[i >> 8] = 0;
		_state.MemoryAccessType[i >> 8] = RegisterAccess::None;
	}
}

template<MemoryOperationType opType>
uint8_t GbMemoryManager::Read(uint16_t addr)
{
	uint8_t value = 0;
	if(_state.IsReadRegister[addr >> 8]) {
		value = ReadRegister(addr);
	} else if(_reads[addr >> 8]) {
		value = _reads[addr >> 8][(uint8_t)addr];
	}
	_emu->ProcessMemoryRead<CpuType::Gameboy>(addr, value, opType);
	return value;
}

bool GbMemoryManager::IsOamDmaRunning()
{
	return _dmaController->IsOamDmaRunning();
}

void GbMemoryManager::WriteDma(uint16_t addr, uint8_t value)
{
	_emu->ProcessMemoryRead<CpuType::Gameboy>(addr, value, MemoryOperationType::DmaWrite);
	_ppu->WriteOam((uint8_t)addr, value, true);
}

uint8_t GbMemoryManager::ReadDma(uint16_t addr)
{
	uint8_t value = 0;
	if(_reads[addr >> 8]) {
		value = _reads[addr >> 8][(uint8_t)addr];
	} else if(addr >= 0x8000 && addr <= 0x9FFF) {
		value = ReadRegister(addr);
	}
	_emu->ProcessMemoryRead<CpuType::Gameboy>(addr, value, MemoryOperationType::DmaRead);
	return value;
}

template<MemoryOperationType type>
void GbMemoryManager::Write(uint16_t addr, uint8_t value)
{
	_emu->ProcessMemoryWrite<CpuType::Gameboy>(addr, value, type);
	if(_state.IsWriteRegister[addr >> 8]) {
		WriteRegister(addr, value);
	} else if(_writes[addr >> 8]) {
		_writes[addr >> 8][(uint8_t)addr] = value;
	}
}

uint8_t GbMemoryManager::DebugRead(uint16_t addr)
{
	if(_state.IsReadRegister[addr >> 8]) {
		if(addr >= 0xFE00) {
			return PeekRegister(addr);
		} else {
			//Avoid potential read side effects
			return 0xFF;
		}
	} else if(_reads[addr >> 8]) {
		return _reads[addr >> 8][(uint8_t)addr];
	}
	return 0;
}

void GbMemoryManager::DebugWrite(uint16_t addr, uint8_t value)
{
	if(_state.IsWriteRegister[addr >> 8]) {
		//Do not write to registers via debug tools
	} else if(_writes[addr >> 8]) {
		_writes[addr >> 8][(uint8_t)addr] = value;
	}
}

uint8_t* GbMemoryManager::GetMappedBlock(uint16_t addr)
{
	if(_reads[addr >> 8]) {
		return _reads[addr >> 8];
	}
	return nullptr;
}

uint8_t GbMemoryManager::PeekRegister(uint16_t addr)
{
	//Peek on oam/vram to avoid triggering the invalid oam/vram access break options
	if(addr >= 0xFE00 && addr <= 0xFE9F) {
		return _ppu->PeekOam((uint8_t)addr);
	} else if(addr >= 0x8000 && addr <= 0x9FFF) {
		return _ppu->PeekVram(addr);
	} else if(addr >= 0xFF10 && addr <= 0xFF3F) {
		return _apu->Peek(addr);
	} else {
		return ReadRegister(addr);
	}
}

uint8_t GbMemoryManager::ReadRegister(uint16_t addr)
{
	if(addr >= 0xFF00) {
		if(addr == 0xFFFF) {
			return _state.IrqEnabled; //IE - Interrupt Enable (R/W)
		} else if(addr == 0xFF46) {
			return _dmaController->Read();
		} else if(addr >= 0xFF80) {
			return _highRam[addr & 0x7F]; //80-FE
		} else if(addr >= 0xFF4C) {
			if(_gameboy->IsCgb()) {
				switch(addr) {
					//FF4D - KEY1 - CGB Mode Only - Prepare Speed Switch
					case 0xFF4D:
						if(_ppu->IsCgbEnabled()) {
							return (
								(_state.CgbHighSpeed ? 0x80 : 0) |
								(_state.CgbSwitchSpeedRequest ? 0x01 : 0) |
								0x7E
							);
						}
						return 0xFF;
					
					case 0xFF55: //CGB - DMA
						return _ppu->IsCgbEnabled() ? _dmaController->ReadCgb(addr) : 0xFF;

					case 0xFF4F: //CGB - VRAM bank
					case 0xFF68: case 0xFF69: case 0xFF6A: case 0xFF6B: //CGB - Palette
						return _ppu->ReadCgbRegister(addr);

					//FF70 - SVBK - CGB Mode Only - WRAM Bank
					case 0xFF70: return _ppu->IsCgbEnabled() ? (_state.CgbWorkRamBank | 0xF8) : 0xFF;
					case 0xFF72: return _state.CgbRegFF72;
					case 0xFF73: return _state.CgbRegFF73;
					
					case 0xFF74: 
						if(_ppu->IsCgbEnabled()) {
							return _state.CgbRegFF74;
						}
						return 0xFF;

					case 0xFF75: return _state.CgbRegFF75 | 0x8F;
					
					case 0xFF76: case 0xFF77:
						return _apu->ReadCgbRegister(addr);
				}
			}
			LogDebug("[Debug] GB - Missing read handler: $" + HexUtilities::ToHex(addr));
			return 0xFF; //4C-7F, open bus
		} else if(addr >= 0xFF40) {
			return _ppu->Read(addr); //40-4B
		} else if(addr >= 0xFF10) {
			return _apu->Read(addr); //10-3F
		} else {
			//00-0F
			switch(addr) {
				case 0xFF00: return ReadInputPort(); break;
				
				case 0xFF01: return _state.SerialData; //SB - Serial transfer data (R/W)
				case 0xFF02: return _state.SerialControl | 0x7E; //SC - Serial Transfer Control (R/W)

				case 0xFF04: case 0xFF05: case 0xFF06: case 0xFF07:
					return _timer->Read(addr);

				case 0xFF0F: return _state.IrqRequests | 0xE0; break; //IF - Interrupt flags (R/W)

				default: return 0xFF; //Open bus
			}
		}
	} else if(addr >= 0xFE00) {
		return _ppu->ReadOam((uint8_t)addr);
	} else if(addr >= 0x8000 && addr <= 0x9FFF) {
		return _ppu->ReadVram(addr);
	}

	return _cart->ReadRegister(addr);
}

void GbMemoryManager::WriteRegister(uint16_t addr, uint8_t value)
{
	 if(addr >= 0xFF00) {
		if(addr == 0xFFFF) {
			_state.IrqEnabled = value; //IE register
		} else if(addr == 0xFF46) {
			_dmaController->Write(value);
		} else if(addr >= 0xFF80) {
			_highRam[addr & 0x7F] = value; //80-FE
		} else if(addr >= 0xFF4C) {
			//4C-7F
			if(addr == 0xFF50) {
				if(value & 0x01) {
					_state.DisableBootRom = true;
					RefreshMappings();
				}
			} else if(_gameboy->IsCgb()) {
				switch(addr) {
					case 0xFF4D:
						//FF4D - KEY1 - CGB Mode Only - Prepare Speed Switch
						if(_ppu->IsCgbEnabled()) {
							_state.CgbSwitchSpeedRequest = (value & 0x01) != 0;
						}
						break;

					case 0xFF51: case 0xFF52: case 0xFF53: case 0xFF54: case 0xFF55: //CGB - DMA
						if(_ppu->IsCgbEnabled()) {
							_dmaController->WriteCgb(addr, value);
						}
						break;

					case 0xFF4C: //CGB - "LCDMODE", set by boot rom to turn off CGB features for the LCD for DMG games
						if(!_state.DisableBootRom) {
							_ppu->WriteCgbRegister(addr, value);
						}
						break;

					case 0xFF4F: //CGB - VRAM banking
					case 0xFF68: case 0xFF69: case 0xFF6A: case 0xFF6B: //CGB - Palette
						_ppu->WriteCgbRegister(addr, value);
						break;

					case 0xFF70:
						//FF70 - SVBK - CGB Mode Only - WRAM Bank
						if(_ppu->IsCgbEnabled()) {
							_state.CgbWorkRamBank = std::max(1, value & 0x07);
							RefreshMappings();
						}
						break;

					case 0xFF72: _state.CgbRegFF72 = value; break;
					case 0xFF73: _state.CgbRegFF73 = value; break;
					case 0xFF74:
						if(_ppu->IsCgbEnabled()) {
							_state.CgbRegFF74 = value;
						}
						break;

					case 0xFF75: _state.CgbRegFF75 = value; break;

					default:
						LogDebug("[Debug] GBC - Missing write handler: $" + HexUtilities::ToHex(addr));
						break;
				}
			}
		} else if(addr >= 0xFF40) {
			_ppu->Write(addr, value); //40-4B
		} else if(addr >= 0xFF10) {
			_apu->Write(addr, value); //10-3F
		} else {
			//00-0F
			switch(addr) {
				case 0xFF00: WriteInputPort(value); break;
				case 0xFF01: _state.SerialData = value; break; //FF01 - SB - Serial transfer data (R/W)
				case 0xFF02: 
					//FF02 - SC - Serial Transfer Control (R/W)
					_state.SerialControl = value & (_gameboy->IsCgb() ? 0x83 : 0x81);
					if((_state.SerialControl & 0x80) && (_state.SerialControl & 0x01)) {
						_state.SerialBitCount = 8;
					} else {
						_state.SerialBitCount = 0;
					}
					break;

				case 0xFF04: case 0xFF05: case 0xFF06: case 0xFF07:
					_timer->Write(addr, value);
					break;

				case 0xFF0F: _state.IrqRequests = value & 0x1F; break; //IF - Interrupt flags (R/W)
			}
		}
	} else if(addr >= 0xFE00) {
		_ppu->WriteOam((uint8_t)addr, value, false);
	} else if(addr >= 0x8000 && addr <= 0x9FFF) {
		_ppu->WriteVram(addr, value);
	} else {
		_cart->WriteRegister(addr, value);
	}
}

void GbMemoryManager::RequestIrq(uint8_t source)
{
	_state.IrqRequests |= source;
}

void GbMemoryManager::ClearIrqRequest(uint8_t source)
{
	_state.IrqRequests &= ~source;
}

uint8_t GbMemoryManager::ProcessIrqRequests()
{
	uint8_t irqsToProcess = _state.IrqRequests & _state.IrqEnabled;
	if(irqsToProcess) {
		if(irqsToProcess & GbIrqSource::VerticalBlank) {
			return GbIrqSource::VerticalBlank;
		} else if(irqsToProcess & GbIrqSource::LcdStat) {
			return GbIrqSource::LcdStat;
		} else if(irqsToProcess & GbIrqSource::Timer) {
			return GbIrqSource::Timer;
		} else if(irqsToProcess & GbIrqSource::Serial) {
			return GbIrqSource::Serial;
		} else if(irqsToProcess & GbIrqSource::Joypad) {
			return GbIrqSource::Joypad;
		}
	}
	return 0;
}

void GbMemoryManager::ToggleSpeed()
{
	_state.CgbSwitchSpeedRequest = false;
	_state.CgbHighSpeed = !_state.CgbHighSpeed;
}

bool GbMemoryManager::IsHighSpeed()
{
	return _state.CgbHighSpeed;
}

bool GbMemoryManager::IsBootRomDisabled()
{
	return _state.DisableBootRom;
}

uint64_t GbMemoryManager::GetCycleCount()
{
	return _state.CycleCount;
}

uint64_t GbMemoryManager::GetApuCycleCount()
{
	return _state.ApuCycleCount;
}

uint8_t GbMemoryManager::ReadInputPort()
{
	//Bit 7 - Not used
	//Bit 6 - Not used
	//Bit 5 - P15 Select Button Keys      (0=Select)
	//Bit 4 - P14 Select Direction Keys   (0=Select)
	//Bit 3 - P13 Input Down  or Start    (0=Pressed) (Read Only)
	//Bit 2 - P12 Input Up    or Select   (0=Pressed) (Read Only)
	//Bit 1 - P11 Input Left  or Button B (0=Pressed) (Read Only)
	//Bit 0 - P10 Input Right or Button A (0=Pressed) (Read Only)
	uint8_t result = 0x0F;

	SuperGameboy* sgb = _gameboy->GetSgb();
	if(sgb) {
		if((_state.InputSelect & 0x30) == 0x30) {
			result = sgb->GetInputIndex();
		} else {
			if(!(_state.InputSelect & 0x20)) {
				result &= sgb->GetInput() >> 4;
			}
			if(!(_state.InputSelect & 0x10)) {
				result &= sgb->GetInput() & 0x0F;
			}
		}
	} else {
		BaseControlDevice* controller = (SnesController*)_controlManager->GetControlDevice(0).get();
		if(controller && controller->GetControllerType() == ControllerType::SnesController) {
			if(!(_state.InputSelect & 0x20)) {
				result &= ~(controller->IsPressed(SnesController::A) ? 0x01 : 0);
				result &= ~(controller->IsPressed(SnesController::B) ? 0x02 : 0);
				result &= ~(controller->IsPressed(SnesController::Select) ? 0x04 : 0);
				result &= ~(controller->IsPressed(SnesController::Start) ? 0x08 : 0);
			}
			if(!(_state.InputSelect & 0x10)) {
				result &= ~(controller->IsPressed(SnesController::Right) ? 0x01 : 0);
				result &= ~(controller->IsPressed(SnesController::Left) ? 0x02 : 0);
				result &= ~(controller->IsPressed(SnesController::Up) ? 0x04 : 0);
				result &= ~(controller->IsPressed(SnesController::Down) ? 0x08 : 0);
			}
		}
	}

	return result | (_state.InputSelect & 0x30) | 0xC0;
}

void GbMemoryManager::WriteInputPort(uint8_t value)
{
	_state.InputSelect = value;
	SuperGameboy* sgb = _gameboy->GetSgb();
	if(sgb) {
		sgb->ProcessInputPortWrite(value & 0x30);
	}
}

void GbMemoryManager::Serialize(Serializer& s)
{
	s.Stream(
		_state.DisableBootRom, _state.IrqEnabled, _state.IrqRequests, _state.InputSelect,
		_state.ApuCycleCount, _state.CgbHighSpeed, _state.CgbSwitchSpeedRequest, _state.CgbWorkRamBank,
		_state.SerialData, _state.SerialControl, _state.SerialBitCount, _state.CycleCount,
		_state.CgbRegFF72, _state.CgbRegFF73, _state.CgbRegFF74, _state.CgbRegFF75
	);
	s.StreamArray(_state.MemoryType, 0x100);
	s.StreamArray(_state.MemoryOffset, 0x100);
	s.StreamArray(_state.MemoryAccessType, 0x100);
	s.StreamArray(_state.IsReadRegister, 0x100);
	s.StreamArray(_state.IsWriteRegister, 0x100);

	if(!s.IsSaving()) {
		//Restore mappings based on state
		for(int i = 0; i < 0x100; i++) {
			Map(i*0x100, i*0x100+0xFF, _state.MemoryType[i], _state.MemoryOffset[i], _state.MemoryAccessType[i] == RegisterAccess::ReadWrite ? false : true);
		}
		RefreshMappings();
	}
}

template uint8_t GbMemoryManager::Read<MemoryOperationType::Read>(uint16_t addr);
template uint8_t GbMemoryManager::Read<MemoryOperationType::ExecOpCode>(uint16_t addr);
template uint8_t GbMemoryManager::Read<MemoryOperationType::ExecOperand>(uint16_t addr);
template uint8_t GbMemoryManager::Read<MemoryOperationType::DmaRead>(uint16_t addr);
template void GbMemoryManager::Write<MemoryOperationType::Write>(uint16_t addr, uint8_t value);
template void GbMemoryManager::Write<MemoryOperationType::DmaWrite>(uint16_t addr, uint8_t value);