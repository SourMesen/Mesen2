#include "pch.h"
#include "GBA/GbaMemoryManager.h"
#include "GBA/GbaConsole.h"
#include "GBA/GbaPpu.h"
#include "GBA/GbaCpu.h"
#include "GBA/GbaDmaController.h"
#include "GBA/GbaTimer.h"
#include "GBA/GbaSerial.h"
#include "GBA/GbaControlManager.h"
#include "GBA/GbaRomPrefetch.h"
#include "GBA/Debugger/MgbaLogHandler.h"
#include "GBA/APU/GbaApu.h"
#include "GBA/Cart/GbaCart.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/MemoryType.h"
#include "Shared/KeyManager.h"
#include "Shared/MessageManager.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/BitUtilities.h"
#include "Utilities/Serializer.h"

GbaMemoryManager::GbaMemoryManager(Emulator* emu, GbaConsole* console, GbaPpu* ppu, GbaDmaController* dmaController, GbaControlManager* controlManager, GbaTimer* timer, GbaApu* apu, GbaCart* cart, GbaSerial* serial, GbaRomPrefetch* prefetch)
{
	_emu = emu;
	_console = console;
	_ppu = ppu;
	_dmaController = dmaController;
	_controlManager = controlManager;
	_timer = timer;
	_apu = apu;
	_cart = cart;
	_serial = serial;
	_prefetch = prefetch;

	_mgbaLog.reset(new MgbaLogHandler());
	
	_prgRom = (uint8_t*)emu->GetMemory(MemoryType::GbaPrgRom).Memory;
	_prgRomSize = emu->GetMemory(MemoryType::GbaPrgRom).Size;
	_bootRom = (uint8_t*)emu->GetMemory(MemoryType::GbaBootRom).Memory;
	_intWorkRam = (uint8_t*)emu->GetMemory(MemoryType::GbaIntWorkRam).Memory;
	_extWorkRam = (uint8_t*)emu->GetMemory(MemoryType::GbaExtWorkRam).Memory;
	_vram = (uint8_t*)emu->GetMemory(MemoryType::GbaVideoRam).Memory;
	_oam = (uint8_t*)emu->GetMemory(MemoryType::GbaSpriteRam).Memory;
	_palette = (uint8_t*)emu->GetMemory(MemoryType::GbaPaletteRam).Memory;
	_saveRam = (uint8_t*)emu->GetMemory(MemoryType::GbaSaveRam).Memory;
	_saveRamSize = emu->GetMemory(MemoryType::GbaSaveRam).Size;

	_waitStatesLut = new uint8_t[0x400];
	GenerateWaitStateLut();

	//Used to get the correct timing for the timer prescaler, based on the "timer" test
	_masterClock = 48;

	if(_emu->GetSettings()->GetGbaConfig().SkipBootScreen) {
		_biosLocked = true;
		_state.BootRomOpenBus[1] = 0xF0;
		_state.BootRomOpenBus[2] = 0x29;
		_state.BootRomOpenBus[3] = 0xE1;
	}
}

GbaMemoryManager::~GbaMemoryManager()
{
	delete[] _waitStatesLut;
}

void GbaMemoryManager::ProcessIdleCycle()
{
	_prefetch->Exec(1, _state.PrefetchEnabled);
	ProcessInternalCycle<true>();
}

void GbaMemoryManager::ProcessStoppedCycle()
{
	//What exactly is disabled when stopped?
	//This at least works the games that were tested (but is very likely inaccurate)
	_masterClock++;
	_ppu->Exec(); //keep PPU running to keep emulation running properly
	_timer->Exec(_masterClock);
}

void GbaMemoryManager::ProcessPendingUpdates(bool allowStartDma)
{
	if(_dmaController->HasPendingDma()) {
		_dmaController->RunPendingDma(allowStartDma);
	}

	_masterClock++;

	if(_state.IrqUpdateCounter) {
		_state.IrqUpdateCounter--;
		
		_state.IrqPending <<= 1;
		_state.IrqPending |= (uint8_t)(bool)(_state.IE & _state.IF);
		
		_state.IrqLine <<= 1;
		_state.IrqLine |= (uint8_t)((bool)(_state.IE & _state.IF) && _state.IME);

		_state.IE = _state.NewIE;
		_state.IF = _state.NewIF;
		_state.IME = _state.NewIME;
	}

	if(_pendingIrqSourceDelay && --_pendingIrqSourceDelay == 0) {
		_state.NewIF |= (int)_pendingIrqSource;
		TriggerIrqUpdate();
	}

	if(_timer->HasPendingTimers()) {
		_timer->ProcessPendingTimers();
	}

	_ppu->Exec();
	_timer->Exec(_masterClock);

	if(_serial->HasPendingIrq()) {
		_serial->CheckForIrq(_masterClock);
	}

	_hasPendingUpdates = (
		_dmaController->HasPendingDma() ||
		_timer->HasPendingTimers() ||
		_state.IrqUpdateCounter ||
		_pendingIrqSourceDelay ||
		_serial->HasPendingIrq()
	);
}

void GbaMemoryManager::ProcessPendingLateUpdates()
{
	if(_timer->HasPendingWrites()) {
		_timer->ProcessPendingWrites();
	}

	_hasPendingLateUpdates = false;
}

void GbaMemoryManager::GenerateWaitStateLut()
{
	for(GbaAccessModeVal mode = 0; mode < 4; mode++) {
		for(int i = 0; i <= 0xFF; i++) {
			uint8_t waitStates = 1;
			switch(i) {
				case 0x02:
					//External work ram
					waitStates = (mode & GbaAccessMode::Word) ? 6 : 3;
					break;

				case 0x05:
				case 0x06:
					//VRAM/Palette
					waitStates = (mode & GbaAccessMode::Word) ? 2 : 1;
					break;

				case 0x08:
				case 0x09:
					waitStates = _state.PrgWaitStates0[mode & GbaAccessMode::Sequential] + ((mode & GbaAccessMode::Word) ? _state.PrgWaitStates0[1] : 0);
					break;

				case 0x0A:
				case 0x0B:
					waitStates = _state.PrgWaitStates1[mode & GbaAccessMode::Sequential] + ((mode & GbaAccessMode::Word) ? _state.PrgWaitStates1[1] : 0);
					break;

				case 0x0C:
				case 0x0D:
					waitStates = _state.PrgWaitStates2[mode & GbaAccessMode::Sequential] + ((mode & GbaAccessMode::Word) ? _state.PrgWaitStates2[1] : 0);
					break;

				case 0x0E:
				case 0x0F:
					//SRAM
					waitStates = _state.SramWaitStates;
					break;
			}

			_waitStatesLut[(i << 2) | mode] = waitStates;
		}
	}
}

uint8_t GbaMemoryManager::GetWaitStates(GbaAccessModeVal mode, uint32_t addr)
{
	return _waitStatesLut[((addr >> 22) & 0x3FC) | (mode & (GbaAccessMode::Word | ((addr & 0x1FFFF) ? GbaAccessMode::Sequential : 0)))];
}

void GbaMemoryManager::ProcessWaitStates(GbaAccessModeVal mode, uint32_t addr)
{
	uint8_t waitStates;
	if(addr < 0x8000000 || addr >= 0x10000000) {
		waitStates = GetWaitStates(mode, addr);
		_prefetch->Exec(waitStates, _state.PrefetchEnabled);
	} else if((mode & GbaAccessMode::Dma) || !(mode & GbaAccessMode::Prefetch)) {
		//Accesses to ROM from DMA or reads not caused by the CPU loading opcodes will reset the cartridge prefetcher
		//When the prefetch is reset on its last cycle, the ROM access takes an extra cycle to complete
		waitStates = GetWaitStates(mode, addr) + (int)_prefetch->Reset();
	} else {
		waitStates = _state.PrefetchEnabled ? _prefetch->Read<true>(mode, addr) : _prefetch->Read<false>(mode, addr);
	}

	ProcessInternalCycle<true>();
	waitStates--;

	while(waitStates >= 3) {
		ProcessInternalCycle();
		ProcessInternalCycle();
		ProcessInternalCycle();
		waitStates -= 3;
	}

	switch(waitStates) {
		case 2: ProcessInternalCycle(); [[fallthrough]];
		case 1: ProcessInternalCycle(); break;
	}
}

void GbaMemoryManager::ProcessVramStalling(uint32_t addr)
{
	//TODOGBA 32-bit vram/palette writes should be split across 2 clocks (and stalled independently)
	uint8_t memType = (addr - 0x4000000) >> 24;
	if(memType == 3) {
		memType = GbaPpuMemAccess::Oam;
	} else if(memType == GbaPpuMemAccess::Vram && addr & 0x10000) {
		memType = GbaPpuMemAccess::VramObj;
	}
	_ppu->RenderScanline(true);
	while(_ppu->IsAccessingMemory(memType)) {
		//Block CPU until PPU is done accessing ram
		ProcessInternalCycle();
		_ppu->RenderScanline(true);
	}
}

template<uint8_t width>
void GbaMemoryManager::UpdateOpenBus(uint32_t addr, uint32_t value)
{
	if((addr & 0xFF000000) == 0x03000000) {
		//IWRAM appears to have its own open bus value, which overwrites
		//the main bus' open bus value whenever IWRAM is read
		//This is unverified, but passes the openbuster test
		for(int i = 0; i < width; i++) {
			_state.IwramOpenBus[(addr & (4 - width)) + i] = value;
			value >>= 8;
		}
		memcpy(_state.InternalOpenBus, _state.IwramOpenBus, sizeof(_state.IwramOpenBus));
	} else if constexpr(width == 4) {
		_state.InternalOpenBus[0] = value;
		_state.InternalOpenBus[1] = value >> 8;
		_state.InternalOpenBus[2] = value >> 16;
		_state.InternalOpenBus[3] = value >> 24;
	} else if constexpr(width == 2) {
		_state.InternalOpenBus[2] = _state.InternalOpenBus[0] = value;
		_state.InternalOpenBus[3] = _state.InternalOpenBus[1] = value >> 8;
	} else if constexpr(width == 1) {
		_state.InternalOpenBus[0] = value;
		_state.InternalOpenBus[1] = value;
		_state.InternalOpenBus[2] = value;
		_state.InternalOpenBus[3] = value;
	}
}

uint32_t GbaMemoryManager::Read(GbaAccessModeVal mode, uint32_t addr)
{
	ProcessWaitStates(mode, addr);
	if(addr >= 0x5000000 && addr <= 0x7000000) {
		ProcessVramStalling(addr);
	}

	uint32_t value;

	if(mode & GbaAccessMode::Prefetch) {
		_biosLocked = addr >= GbaConsole::BootRomSize;
	}

	bool isSigned = mode & GbaAccessMode::Signed;
	if(mode & GbaAccessMode::Byte) {
		value = InternalRead(mode, addr, addr);
		UpdateOpenBus<1>(addr, value);
		value = isSigned ? (uint32_t)(int8_t)value : (uint8_t)value;
		_emu->ProcessMemoryRead<CpuType::Gba, 1>(addr, value, MemoryOperationType::Read);
	} else if(mode & GbaAccessMode::HalfWord) {
		uint8_t b0 = InternalRead(mode, addr & ~0x01, addr);
		uint8_t b1 = InternalRead(mode, addr | 1, addr);
		value = b0 | (b1 << 8);
		UpdateOpenBus<2>(addr, value);
		value = isSigned ? (uint32_t)(int16_t)value : (uint16_t)value;
		if(!(mode & GbaAccessMode::NoRotate)) {
			value = RotateValue(mode, addr, value, isSigned);
		}
		_emu->ProcessMemoryRead<CpuType::Gba, 2>(addr & ~0x01, value, mode & GbaAccessMode::Prefetch ? MemoryOperationType::ExecOpCode : MemoryOperationType::Read);
	} else {
		uint8_t b0 = InternalRead(mode, addr & ~0x03, addr);
		uint8_t b1 = InternalRead(mode, (addr & ~0x03) | 1, addr);
		uint8_t b2 = InternalRead(mode, (addr & ~0x03) | 2, addr);
		uint8_t b3 = InternalRead(mode, addr | 3, addr);
		value = b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
		UpdateOpenBus<4>(addr, value);
		if(!(mode & GbaAccessMode::NoRotate)) {
			value = RotateValue(mode, addr, value, isSigned);
		}
		_emu->ProcessMemoryRead<CpuType::Gba, 4>(addr & ~0x03, value, mode & GbaAccessMode::Prefetch ? MemoryOperationType::ExecOpCode : MemoryOperationType::Read);
	}

	return value;
}

template<bool debug>
uint32_t GbaMemoryManager::RotateValue(GbaAccessModeVal mode, uint32_t addr, uint32_t value, bool isSigned)
{
	uint8_t shift = (addr & ((mode & GbaAccessMode::HalfWord) ? 0x01 : 0x03)) << 3;
	if(shift) {
		if constexpr(!debug) {
			_emu->BreakIfDebugging(CpuType::Gba, BreakSource::GbaUnalignedMemoryAccess);
		}
		if(isSigned) {
			return (int32_t)value >> shift;
		} else {
			return (value << (32 - shift)) | (value >> shift);
		}
	}
	return value;
}

void GbaMemoryManager::Write(GbaAccessModeVal mode, uint32_t addr, uint32_t value)
{
	ProcessWaitStates(mode, addr);
	if(addr >= 0x5000000 && addr <= 0x7000000) {
		ProcessVramStalling(addr);
	}

	if(mode & GbaAccessMode::Byte) {
		if(_emu->ProcessMemoryWrite<CpuType::Gba, 1>(addr, value, MemoryOperationType::Write)) {
			InternalWrite(mode, addr, (uint8_t)value, addr, value);
		}
	} else if(mode & GbaAccessMode::HalfWord) {
		if(_emu->ProcessMemoryWrite<CpuType::Gba, 2>(addr & ~0x01, value, MemoryOperationType::Write)) {
			InternalWrite(mode, addr & ~0x01, (uint8_t)value, addr, value);
			InternalWrite(mode, (addr & ~0x01) | 0x01, (uint8_t)(value >> 8), addr, value);
		}
	} else {
		if(_emu->ProcessMemoryWrite<CpuType::Gba, 4>(addr & ~0x03, value, MemoryOperationType::Write)) {
			InternalWrite(mode, (addr & ~0x03), (uint8_t)value, addr, value);
			InternalWrite(mode, (addr & ~0x03) | 0x01, (uint8_t)(value >> 8), addr, value);
			InternalWrite(mode, (addr & ~0x03) | 0x02, (uint8_t)(value >> 16), addr, value);
			InternalWrite(mode, (addr & ~0x03) | 0x03, (uint8_t)(value >> 24), addr, value);
		}
	}
}

uint8_t GbaMemoryManager::InternalRead(GbaAccessModeVal mode, uint32_t addr, uint32_t readAddr)
{
	uint8_t bank = (addr >> 24);
	addr &= 0xFFFFFF;
	switch(bank) {
		case 0x00:
			//bootrom
			if(addr < GbaConsole::BootRomSize) {
				if(!_biosLocked) {
					return _state.BootRomOpenBus[addr & 0x03] = _bootRom[addr];
				} else {
					return _state.BootRomOpenBus[addr & 0x03];
				}
			}
			return _state.InternalOpenBus[addr & 0x03];

		case 0x02: return _extWorkRam[addr & (GbaConsole::ExtWorkRamSize - 1)];
		case 0x03: return _intWorkRam[addr & (GbaConsole::IntWorkRamSize - 1)];

		case 0x04: return ReadRegister(addr);

		case 0x05: return _palette[addr & (GbaConsole::PaletteRamSize - 1)];

		case 0x06:
			if(addr & 0x10000) {
				if(addr >= 0x18000 && _ppu->IsBitmapMode() && !(addr & 0x4000)) {
					//reads to mirrors of the first 0x4000 when in bitmap mode return 0 instead
					return 0;
				} else {
					return _vram[addr & 0x17FFF];
				}
			} else {
				return _vram[addr & 0xFFFF];
			}

		case 0x07: return _oam[addr & (GbaConsole::SpriteRamSize - 1)];

		case 0x08:
		case 0x09:
		case 0x0A:
		case 0x0B:
		case 0x0C:
			return _cart->ReadRom<false>((bank << 24) | addr);

		case 0x0D:
			return _cart->ReadRom<true>((bank << 24) | addr);

		case 0x0E:
		case 0x0F:
			return _cart->ReadRam((bank << 24) | addr, readAddr);
	}

	return _state.InternalOpenBus[addr & 0x03];
}

void GbaMemoryManager::InternalWrite(GbaAccessModeVal mode, uint32_t addr, uint8_t value, uint32_t writeAddr, uint32_t fullValue)
{
	uint8_t bank = (addr >> 24);
	addr &= 0xFFFFFF;

	_state.InternalOpenBus[addr & 0x03] = value;

	switch(bank) {
		case 0x00:
			//bootrom
			break;

		case 0x02: _extWorkRam[addr & (GbaConsole::ExtWorkRamSize - 1)] = value; break;
		case 0x03:
			_intWorkRam[addr & (GbaConsole::IntWorkRamSize - 1)] = value;
			_state.IwramOpenBus[addr & 0x03] = value;
			break;

		case 0x04:
			//registers
			if(addr < 0x3FF) {
				WriteRegister(mode, addr, value);
			} else if(addr >= 0xFFF600 && addr <= 0xFFF783) {
				if(_emu->GetSettings()->GetGbaConfig().EnableMgbaLogApi) {
					_mgbaLog->Write(addr, value);
				}
			}
			break;

		case 0x05:
			if(!(mode & GbaAccessMode::Byte)) {
				_palette[addr & (GbaConsole::PaletteRamSize - 1)] = value;
			} else {
				//Mirror the value over a half-word
				_palette[(addr & (GbaConsole::PaletteRamSize - 1)) & ~0x01] = value;
				_palette[(addr & (GbaConsole::PaletteRamSize - 1)) | 0x01] = value;
			}
			break;

		case 0x06:
			//vram
			if(addr & 0x10000) {
				if(addr >= 0x18000 && _ppu->IsBitmapMode() && !(addr & 0x4000)) {
					//Ignore writes to mirrors of the first 0x4000 when in bitmap mode
					//Fixes a visual glitch on the title screen of Acrobat Kid
					break;
				}

				if(!(mode & GbaAccessMode::Byte)) {
					_vram[addr & 0x17FFF] = value;
				} else if(_ppu->IsBitmapMode() && addr < 0x14000) {
					//Ignore byte mode writes above 0x10000 in non-bitmap modes and above 0x14000 in bitmap modes
					//Mirror the value over a half-word
					_vram[addr & 0x17FFE] = value;
					_vram[(addr & 0x17FFF) | 0x01] = value;
				}
			} else {
				if(mode & GbaAccessMode::Byte) {
					_vram[addr & 0xFFFE] = value;
					_vram[(addr & 0xFFFF) | 0x01] = value;
				} else {
					_vram[addr & 0xFFFF] = value;
				}
			}
			break;

		case 0x07:
			if(!(mode & GbaAccessMode::Byte)) {
				//OAM ignores all byte mode writes
				_oam[addr & (GbaConsole::SpriteRamSize - 1)] = value;
			}
			break;

		case 0x08:
		case 0x09:
		case 0x0A:
		case 0x0B:
		case 0x0C:
		case 0x0D:
			//ROM
			_cart->WriteRom((bank << 24) | addr, value);
			break;

		case 0x0E:
		case 0x0F:
			_cart->WriteRam(mode, addr, value, writeAddr, fullValue);
			break;
	}
}

uint32_t GbaMemoryManager::ReadRegister(uint32_t addr)
{
	if(addr < 0x60) {
		return _ppu->ReadRegister(addr);
	} else if(addr < 0xB0) {
		return _apu->ReadRegister(addr);
	} else if(addr < 0xE0) {
		return _dmaController->ReadRegister(addr);
	} else if(addr >= 0x100 && addr < 0x110) {
		return _timer->ReadRegister(addr);
	} else if((addr >= 0x120 && addr <= 0x12B) || (addr >= 0x134 && addr <= 0x159)) {
		return _serial->ReadRegister(addr, false);
	} else {
		switch(addr) {
			case 0x130:
			case 0x131:
			case 0x132:
			case 0x133:
				return _controlManager->ReadInputPort(addr);

			case 0x15A: return 0;
			case 0x15B: return 0;

			case 0x200: return (uint8_t)_state.IE;
			case 0x201: return (uint8_t)(_state.IE >> 8);
			case 0x202: return (uint8_t)_state.IF;
			case 0x203: return (uint8_t)(_state.IF >> 8);
			case 0x204: return (uint8_t)_state.WaitControl;
			case 0x205: return (uint8_t)(_state.WaitControl >> 8);
			case 0x206: return 0;
			case 0x207: return 0;

			case 0x208: return _state.IME;
			case 0x209: return 0;
			case 0x20A: return 0;
			case 0x20B: return 0;

			case 0x300: return (uint8_t)_state.PostBootFlag;
			case 0x301: return 0;
			case 0x302: return 0;
			case 0x303: return 0;

			default:
				if(addr >= 0xFFF780 && addr <= 0xFFF783) {
					if(_emu->GetSettings()->GetGbaConfig().EnableMgbaLogApi) {
						return _mgbaLog->Read(addr);
					}
				}

				LogDebug("Read unimplemented register: " + HexUtilities::ToHex32(addr));
				return _state.InternalOpenBus[addr & 0x01];
		}
	}
}

void GbaMemoryManager::WriteRegister(GbaAccessModeVal mode, uint32_t addr, uint8_t value)
{
	static constexpr uint8_t defaultWaitStates[4] = { 4,3,2,8 };

	switch(addr) {
		case 0x132:
		case 0x133:
			_controlManager->WriteInputPort(mode, addr, value);
			break;

		case 0x200: _state.NewIE = (_state.NewIE & 0xFF00) | value; TriggerIrqUpdate(); break;
		case 0x201: _state.NewIE = (_state.NewIE & 0xFF) | (value << 8); TriggerIrqUpdate(); break;

		case 0x202: _state.NewIF = (_state.NewIF & 0xFF00) | ((_state.NewIF & 0xFF) & ~value); TriggerIrqUpdate(); break;
		case 0x203: _state.NewIF = ((_state.NewIF & 0xFF00) & ~(value << 8)) | (_state.NewIF & 0xFF); TriggerIrqUpdate(); break;

		case 0x204:
			BitUtilities::SetBits<0>(_state.WaitControl, value);
			_state.SramWaitStates = defaultWaitStates[value & 0x03] + 1;
			_state.PrgWaitStates0[0] = defaultWaitStates[(value >> 2) & 0x03] + 1;
			_state.PrgWaitStates0[1] = (value & 0x10) ? 2 : 3;
			_state.PrgWaitStates1[0] = (defaultWaitStates[(value >> 5) & 0x03] + 1);
			_state.PrgWaitStates1[1] = (value & 0x80) ? 2 : 5;
			GenerateWaitStateLut();
			break;

		case 0x205:
			BitUtilities::SetBits<8>(_state.WaitControl, value & 0x7F);
			_state.PrgWaitStates2[0] = defaultWaitStates[value & 0x03] + 1;
			_state.PrgWaitStates2[1] = (value & 0x04) ? 2 : 9;
			_state.PrefetchEnabled = (value & 0x40);
			GenerateWaitStateLut();
			break;
		
		case 0x206: break; //waitcontrol
		case 0x207: break; //waitcontrol

		case 0x208: _state.NewIME = value & 0x01; TriggerIrqUpdate(); break;
		case 0x209: break; //ime
		case 0x20A: break; //ime
		case 0x20B: break; //ime

		case 0x300:
			if(!_biosLocked && !_state.PostBootFlag) {
				//Only accessible from BIOS, and can't be cleared once set
				_state.PostBootFlag = value & 0x01;
			}
			break;

		case 0x301:
			if(!_biosLocked) {
				_haltModeUsed = true;
				_console->GetCpu()->SetStopFlag();
				_state.StopMode = value & 0x80;
			}
			break;

		//TODOGBA case 0x800: break;

		default:
			if(addr < 0x60) {
				_ppu->WriteRegister(addr, value);
			} else if(addr < 0xB0) {
				_apu->WriteRegister(mode, addr, value);
			} else if(addr < 0xE0) {
				_dmaController->WriteRegister(addr, value);
			} else if(addr >= 0x100 && addr < 0x110) {
				_timer->WriteRegister(addr, value);
			} else if((addr >= 0x120 && addr <= 0x12B) || (addr >= 0x134 && addr <= 0x159)) {
				_serial->WriteRegister(addr, value);
			} else {
				LogDebug("Write unimplemented register: " + HexUtilities::ToHex32(addr) + " = " + HexUtilities::ToHex(value));
			}
			break;
	}
}

void GbaMemoryManager::TriggerIrqUpdate()
{
	_state.IrqUpdateCounter = 3;
	SetPendingUpdateFlag();
}

void GbaMemoryManager::SetDelayedIrqSource(GbaIrqSource source, uint8_t delay)
{
	_pendingIrqSource = source;
	_pendingIrqSourceDelay = delay;
	SetPendingUpdateFlag();
}

void GbaMemoryManager::SetIrqSource(GbaIrqSource source)
{
	_state.NewIF |= (int)source;
	TriggerIrqUpdate();
}

bool GbaMemoryManager::HasPendingIrq()
{
	//Keep track of whether or not a pending IRQ exists - used to break out of HALT mode
	return _state.IrqPending & 0x01;
}

bool GbaMemoryManager::ProcessIrq()
{
	//Keep track of the IRQ line used by the CPU to decide if the IRQ handler should be executed
	//(requires the IF+IE IRQ flags and IME to all be set)
	return _state.IrqLine & 0x02;
}

bool GbaMemoryManager::IsHaltOver()
{
	return _state.StopMode ? _controlManager->CheckInputCondition() : HasPendingIrq();
}

bool GbaMemoryManager::UseInlineHalt()
{
	bool result = _haltModeUsed;
	_haltModeUsed = false;
	return result;
}

uint8_t GbaMemoryManager::GetOpenBus(uint32_t addr)
{
	return _state.InternalOpenBus[addr & 0x01];
}

uint32_t GbaMemoryManager::DebugCpuRead(GbaAccessModeVal mode, uint32_t addr)
{
	uint32_t value;
	bool isSigned = mode & GbaAccessMode::Signed;
	if(mode & GbaAccessMode::Byte) {
		value = DebugRead(addr);
		value = isSigned ? (uint32_t)(int8_t)value : (uint8_t)value;
	} else if(mode & GbaAccessMode::HalfWord) {
		uint8_t b0 = DebugRead(addr & ~0x01);
		uint8_t b1 = DebugRead(addr | 1);
		value = b0 | (b1 << 8);
		value = isSigned ? (uint32_t)(int16_t)value : (uint16_t)value;
		if(!(mode & GbaAccessMode::NoRotate)) {
			value = RotateValue<true>(mode, addr, value, isSigned);
		}
	} else {
		uint8_t b0 = DebugRead(addr & ~0x03);
		uint8_t b1 = DebugRead((addr & ~0x03) | 1);
		uint8_t b2 = DebugRead((addr & ~0x03) | 2);
		uint8_t b3 = DebugRead(addr | 3);
		value = b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
		if(!(mode & GbaAccessMode::NoRotate)) {
			value = RotateValue<true>(mode, addr, value, isSigned);
		}
	}
	return value;
}

uint8_t GbaMemoryManager::DebugRead(uint32_t addr)
{
	uint8_t bank = (addr >> 24);
	addr &= 0xFFFFFF;
	switch(bank) {
		case 0x00:
			if(addr < GbaConsole::BootRomSize) {
				return _bootRom[addr];
			}
			break;

		case 0x02: return _extWorkRam[addr & (GbaConsole::ExtWorkRamSize - 1)];
		case 0x03: return _intWorkRam[addr & (GbaConsole::IntWorkRamSize - 1)];

		case 0x04:
			if(addr < 0x3FF) {
				if((addr >= 0x120 && addr <= 0x12B) || (addr >= 0x134 && addr <= 0x159)) {
					return _serial->ReadRegister(addr, true);
				} else {
					return ReadRegister(addr);
				}
			}
			break;

		case 0x05: return _palette[addr & (GbaConsole::PaletteRamSize - 1)];

		case 0x06:
			if(addr & 0x10000) {
				return _vram[addr & 0x17FFF];
			} else {
				return _vram[addr & 0xFFFF];
			}

		case 0x07: return _oam[addr & (GbaConsole::SpriteRamSize - 1)];

		case 0x08: case 0x09: case 0x0A:
		case 0x0B: case 0x0C: case 0x0D: {
			uint32_t romAddr = ((bank & 0x01) << 24) | addr;
			if(romAddr < _prgRomSize) {
				return _prgRom[romAddr];
			}
			break;
		}

		case 0x0E: case 0x0F:
			return _cart->ReadRam(addr, addr);
	}

	return _state.InternalOpenBus[addr & 0x01];
}

void GbaMemoryManager::DebugWrite(uint32_t addr, uint8_t value)
{
	uint8_t bank = (addr >> 24);
	addr &= 0xFFFFFF;
	switch(bank) {
		case 0x00:
			if(addr < GbaConsole::BootRomSize) {
				_bootRom[addr] = value;
			}
			break;

		case 0x02: _extWorkRam[addr & (GbaConsole::ExtWorkRamSize - 1)] = value; break;
		case 0x03: _intWorkRam[addr & (GbaConsole::IntWorkRamSize - 1)] = value; break;

		case 0x04:
			//todogba debugger - allow writing to registers
			break;

		case 0x05: _palette[addr & (GbaConsole::PaletteRamSize - 1)] = value; break;

		case 0x06:
			if(addr & 0x10000) {
				_vram[addr & 0x17FFF] = value; break;
			} else {
				_vram[addr & 0xFFFF] = value; break;
			}

		case 0x07: _oam[addr & (GbaConsole::SpriteRamSize - 1)] = value; break;

		case 0x08: case 0x09: case 0x0A:
		case 0x0B: case 0x0C: case 0x0D:
			if(addr < _prgRomSize) {
				_prgRom[addr] = value;
			}
			break;

		case 0x0E: case 0x0F:
			_cart->DebugWriteRam(addr, value);
			break;
	}
}

AddressInfo GbaMemoryManager::GetAbsoluteAddress(uint32_t addr)
{
	uint8_t bank = (addr >> 24);
	addr &= 0xFFFFFF;
	switch(bank) {
		case 0x00:
			if(addr < GbaConsole::BootRomSize) {
				return { (int32_t)addr, MemoryType::GbaBootRom };
			}
			break;

		case 0x02: return { (int32_t)addr & (GbaConsole::ExtWorkRamSize - 1), MemoryType::GbaExtWorkRam };

		case 0x03: return { (int32_t)addr & (GbaConsole::IntWorkRamSize - 1), MemoryType::GbaIntWorkRam };

		case 0x04:
			//registers
			if(addr < 0x3FF) {
				return { -1, MemoryType::None };
			}
			break;

		case 0x05: return { (int32_t)addr & (GbaConsole::PaletteRamSize - 1), MemoryType::GbaPaletteRam };

		case 0x06:
			if(addr & 0x10000) {
				return { (int32_t)addr & 0x17FFF, MemoryType::GbaVideoRam };
			} else {
				return { (int32_t)addr & 0xFFFF, MemoryType::GbaVideoRam };
			}

		case 0x07: return { (int32_t)addr & (GbaConsole::SpriteRamSize - 1), MemoryType::GbaSpriteRam };

		case 0x08:
		case 0x09:
		case 0x0A:
		case 0x0B:
		case 0x0C:
		case 0x0D:
			addr |= (bank & 0x01) << 24;
			if(addr < _prgRomSize) {
				return { (int32_t)addr, MemoryType::GbaPrgRom };
			}
			break;

		case 0x0E:
		case 0x0F:
			return _cart->GetRamAbsoluteAddress(addr);
	}

	return { -1, MemoryType::None };
}

int64_t GbaMemoryManager::GetRelativeAddress(AddressInfo& absAddress)
{
	switch(absAddress.Type) {
		default:
		case MemoryType::GbaBootRom: return absAddress.Address;
		case MemoryType::GbaExtWorkRam: return 0x2000000 | absAddress.Address;
		case MemoryType::GbaIntWorkRam: return 0x3000000 | absAddress.Address;
		case MemoryType::GbaPaletteRam: return 0x5000000 | absAddress.Address;
		case MemoryType::GbaPrgRom: return 0x8000000 | absAddress.Address;
		case MemoryType::GbaSaveRam: return _cart->GetRamRelativeAddress(absAddress);
		case MemoryType::GbaSpriteRam: return 0x7000000 | absAddress.Address;
		case MemoryType::GbaVideoRam: return 0x6000000 | absAddress.Address;
		case MemoryType::GbaMemory: return absAddress.Address;
	}
}

void GbaMemoryManager::Serialize(Serializer& s)
{
	SV(_masterClock);
	SV(_hasPendingUpdates);
	SV(_hasPendingLateUpdates);
	SV(_pendingIrqSource);
	SV(_pendingIrqSourceDelay);
	SV(_haltModeUsed);
	SV(_biosLocked);

	SV(_state.IE);
	SV(_state.IF);
	SV(_state.NewIE);
	SV(_state.NewIF);
	SV(_state.NewIME);

	SV(_state.WaitControl);
	SVArray(_state.PrgWaitStates0, 2);
	SVArray(_state.PrgWaitStates1, 2);
	SVArray(_state.PrgWaitStates2, 2);
	SV(_state.SramWaitStates);
	SV(_state.PrefetchEnabled);
	SV(_state.IME);
	SV(_state.IrqUpdateCounter);
	SV(_state.IrqPending);
	SV(_state.IrqLine);
	SV(_state.BusLocked);
	SV(_state.StopMode);
	SV(_state.PostBootFlag);

	SVArray(_state.BootRomOpenBus, 4);
	SVArray(_state.InternalOpenBus, 4);
	SVArray(_state.IwramOpenBus, 4);

	if(!s.IsSaving()) {
		GenerateWaitStateLut();
	}
}