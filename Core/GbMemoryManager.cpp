#include "stdafx.h"
#include "Console.h"
#include "Gameboy.h"
#include "GbMemoryManager.h"
#include "GbPpu.h"
#include "GbApu.h"
#include "GbTimer.h"
#include "GbTypes.h"
#include "GbCart.h"
#include "GbDmaController.h"
#include "EmuSettings.h"
#include "ControlManager.h"
#include "SnesController.h"
#include "MessageManager.h"
#include "../Utilities/VirtualFile.h"
#include "../Utilities/Serializer.h"
#include "../Utilities/HexUtilities.h"

void GbMemoryManager::Init(Console* console, Gameboy* gameboy, GbCart* cart, GbPpu* ppu, GbApu* apu, GbTimer* timer, GbDmaController* dmaController)
{
	_state = {};
	_state.CgbWorkRamBank = 1;

	_prgRom = gameboy->DebugGetMemory(SnesMemoryType::GbPrgRom);
	_prgRomSize = gameboy->DebugGetMemorySize(SnesMemoryType::GbPrgRom);
	_cartRam = gameboy->DebugGetMemory(SnesMemoryType::GbCartRam);
	_cartRamSize = gameboy->DebugGetMemorySize(SnesMemoryType::GbCartRam);
	_workRam = gameboy->DebugGetMemory(SnesMemoryType::GbWorkRam);
	_workRamSize = gameboy->DebugGetMemorySize(SnesMemoryType::GbWorkRam);
	_highRam = gameboy->DebugGetMemory(SnesMemoryType::GbHighRam);

#ifdef USEBOOTROM
	VirtualFile bootRom("Firmware\\boot.bin");
	bootRom.ReadFile(_bootRom, 256);
	_state.DisableBootRom = false;
#else
	_state.DisableBootRom = true;
#endif

	_apu = apu;
	_ppu = ppu;
	_gameboy = gameboy;
	_cart = cart;
	_timer = timer;
	_console = console;
	_dmaController = dmaController;
	_controlManager = console->GetControlManager().get();
	_settings = console->GetSettings().get();

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
		_reads[0] = _bootRom;
	}
}

void GbMemoryManager::Exec()
{
	_state.ApuCycleCount += _state.CgbHighSpeed ? 2 : 4;
	_timer->Exec();
	_ppu->Exec();
	_dmaController->Exec();
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
				offset = (offset + 0x100) & (size - 1);
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

uint8_t GbMemoryManager::Read(uint16_t addr, MemoryOperationType opType)
{
	uint8_t value = 0;
	if(_state.IsReadRegister[addr >> 8]) {
		value = ReadRegister(addr);
	} else if(_reads[addr >> 8]) {
		value = _reads[addr >> 8][(uint8_t)addr];
	}
	_console->ProcessMemoryRead<CpuType::Gameboy>(addr, value, opType);
	return value;
}

bool GbMemoryManager::IsOamDmaRunning()
{
	return _dmaController->IsOamDmaRunning();
}

void GbMemoryManager::WriteDma(uint16_t addr, uint8_t value)
{
	_console->ProcessMemoryRead<CpuType::Gameboy>(addr, value, MemoryOperationType::DmaWrite);
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
	_console->ProcessMemoryRead<CpuType::Gameboy>(addr, value, MemoryOperationType::DmaRead);
	return value;
}

void GbMemoryManager::Write(uint16_t addr, uint8_t value)
{
	_console->ProcessMemoryWrite<CpuType::Gameboy>(addr, value, MemoryOperationType::Write);
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
			return ReadRegister(addr);
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

uint8_t GbMemoryManager::ReadRegister(uint16_t addr)
{
	if(addr >= 0xFF00) {
		if(addr == 0xFFFF) {
			return _state.IrqEnabled; //IE - Interrupt Enable (R/W)
		} else if(addr == 0xFF46) {
			return _dmaController->Read();
		} else if(addr >= 0xFF80) {
			return _highRam[addr & 0x7F]; //80-FE
		} else if(addr >= 0xFF4D) {
			if(_gameboy->IsCgb()) {
				switch(addr) {
					//FF4D - KEY1 - CGB Mode Only - Prepare Speed Switch
					case 0xFF4D: return _state.CgbHighSpeed ? 0x80 : 0;
					
					case 0xFF51: case 0xFF52: case 0xFF53: case 0xFF54: case 0xFF55: //CGB - DMA
						return _dmaController->ReadCgb(addr);

					case 0xFF4F: //CGB - VRAM bank
					case 0xFF68: case 0xFF69: case 0xFF6A: case 0xFF6B: //CGB - Palette
						return _ppu->ReadCgbRegister(addr);

					//FF70 - SVBK - CGB Mode Only - WRAM Bank
					case 0xFF70: return _state.CgbWorkRamBank;
				}
			}
			LogDebug("[Debug] GB - Missing read handler: $" + HexUtilities::ToHex(addr));
			return 0xFF; //4D-7F, open bus
		} else if(addr >= 0xFF40) {
			return _ppu->Read(addr); //40-4C
		} else if(addr >= 0xFF10) {
			return _apu->Read(addr); //10-3F
		} else {
			//00-0F
			switch(addr) {
				case 0xFF00: return ReadInputPort(); break;
				
				case 0xFF01: return 0; //SB - Serial transfer data (R/W)
				case 0xFF02: return 0x7E; //SC - Serial Transfer Control (R/W)

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
		} else if(addr >= 0xFF4D) {
			//4D-7F
			if(addr == 0xFF50) {
				if(value & 0x01) {
					_state.DisableBootRom = true;
					RefreshMappings();
				}
			} else if(_gameboy->IsCgb()) {
				switch(addr) {
					case 0xFF4D:
						//FF4D - KEY1 - CGB Mode Only - Prepare Speed Switch
						_state.CgbSwitchSpeedRequest = (value & 0x01) != 0;
						break;

					case 0xFF51: case 0xFF52: case 0xFF53: case 0xFF54: case 0xFF55: //CGB - DMA
						_dmaController->WriteCgb(addr, value);
						break;

					case 0xFF4F: //CGB - VRAM banking
					case 0xFF68: case 0xFF69: case 0xFF6A: case 0xFF6B: //CGB - Palette
						_ppu->WriteCgbRegister(addr, value);
						break;

					case 0xFF70:
						//FF70 - SVBK - CGB Mode Only - WRAM Bank
						_state.CgbWorkRamBank = std::max(1, value & 0x07);
						RefreshMappings();
						break;

					default:
						LogDebug("[Debug] GBC - Missing write handler: $" + HexUtilities::ToHex(addr));
						break;
				}
			}
		} else if(addr >= 0xFF40) {
			_ppu->Write(addr, value); //40-4C
		} else if(addr >= 0xFF10) {
			_apu->Write(addr, value); //10-3F
		} else {
			//00-0F
			switch(addr) {
				case 0xFF00: _state.InputSelect = value; break;
				case 0xFF01: break; //Serial

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
	BaseControlDevice* controller = (SnesController*)_controlManager->GetControlDevice(0).get();
	uint8_t result = 0x0F;
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

	return result | (_state.InputSelect & 0x30) | 0xC0;
}

void GbMemoryManager::Serialize(Serializer& s)
{
	s.Stream(
		_state.DisableBootRom, _state.IrqEnabled, _state.IrqRequests, _state.InputSelect,
		_state.ApuCycleCount, _state.CgbHighSpeed, _state.CgbSwitchSpeedRequest, _state.CgbWorkRamBank
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
