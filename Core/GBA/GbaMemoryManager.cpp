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
#include "GBA/GbaWaitStates.h"
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

	_waitStates.GenerateWaitStateLut(_state);

	//Used to get the correct timing for the timer prescaler, based on the "timer" test
	_masterClock = -1;

	if(_emu->GetSettings()->GetGbaConfig().SkipBootScreen) {
		_biosLocked = true;
		_state.BootRomOpenBus[1] = 0xF0;
		_state.BootRomOpenBus[2] = 0x29;
		_state.BootRomOpenBus[3] = 0xE1;
		_state.PostBootFlag = true;
	}
}

GbaMemoryManager::~GbaMemoryManager()
{
}

void GbaMemoryManager::ProcessParallelIdleCycle()
{
	if(_console->GetCpu()->IsHalted()) {
		//When halted, the IRQ state is always updated, even during DMA, so no processing needs to be done here
		_dmaIrqCounter = 0;
	} else if(_dmaIrqCounter) {
		//Update the IRQ state to match (because the IRQ state does get updated during the idle cycles that run in parallel with DMA)
		_dmaIrqCounter--;
		_state.IrqPending <<= 1;
		_state.IrqPending |= (_dmaIrqPending >> _dmaIrqCounter) & 0x01;
		_state.IrqLine <<= 1;
		_state.IrqLine |= (_dmaIrqLine >> _dmaIrqCounter) & 0x01;
		_irqFirstAccessCycle = _state.IrqLine;
	}
}

void GbaMemoryManager::RunPrefetch()
{
	if(_prefetch->NeedExec(_state.PrefetchEnabled)) {
		_prefetch->Exec(1, _state.PrefetchEnabled);
	}
}

void GbaMemoryManager::ProcessStoppedCycle()
{
	//What exactly is disabled when stopped?
	//This at least works for the games that were tested (but is very likely inaccurate)
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

	if(_dmaController->IsRunning() && _dmaIrqCounter < 10) {
		//Keep track of the IRQ state during the first few cycles of DMA
		//This is needed to update the IRQ state after DMA ends, if idle cycles executed during DMA
		_dmaIrqPending <<= 1;
		_dmaIrqPending |= (uint8_t)(bool)(_state.IE & _state.IF);
		_dmaIrqLine <<= 1;
		_dmaIrqLine |= (uint8_t)((bool)(_state.IE & _state.IF) && _state.IME);
		_dmaIrqCounter++;
	}

	if(_state.IrqUpdateCounter) {
		//The IRQ line updates appear to be paused while the CPU is paused for DMA
		//This is needed to pass the Internal_Cycle_DMA_IRQ test
		if(!_dmaController->IsRunning() || _console->GetCpu()->IsHalted()) {
			_state.IrqUpdateCounter--;
			_state.IrqPending <<= 1;
			_state.IrqPending |= (uint8_t)(bool)(_state.IE & _state.IF);

			_state.IrqLine <<= 1;
			_state.IrqLine |= (uint8_t)((bool)(_state.IE & _state.IF) && _state.IME);
		}

		_state.IE = _state.NewIE;
		_state.IF = _state.NewIF;
		_state.IME = _state.NewIME;
	}

	if(_pendingIrqs.size()) {
		for(int i = (int)_pendingIrqs.size() - 1; i >= 0; i--) {
			if(_pendingIrqs[i].Delay && --_pendingIrqs[i].Delay == 0) {
				_state.NewIF |= (int)_pendingIrqs[i].Source;
				_pendingIrqs.erase(_pendingIrqs.begin() + i);
				TriggerIrqUpdate();
			}
		}
	}

	if(_timer->HasPendingTimers()) {
		_timer->ProcessPendingTimers();
	}

	_ppu->Exec();
	_timer->Exec(_masterClock);

	if(_serial->HasPendingIrq()) {
		_serial->CheckForIrq(_masterClock);
	}

	if(_haltDelay && --_haltDelay == 0) {
		_console->GetCpu()->SetStopFlag();
	}

	if(_objEnableDelay && --_objEnableDelay == 0) {
		_ppu->ProcessObjEnableChange();
	}

	_hasPendingUpdates = (
		_dmaController->HasPendingDma() ||
		_timer->HasPendingTimers() ||
		_state.IrqUpdateCounter ||
		_pendingIrqs.size() ||
		_haltDelay ||
		_objEnableDelay ||
		(_dmaController->IsRunning() && _dmaIrqCounter < 10) ||
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

void GbaMemoryManager::ProcessWaitStates(GbaAccessModeVal mode, uint32_t addr)
{
	uint8_t waitStates;

	//Process first cycle before checking prefetch
	//If DMA is triggered by this cycle, the prefetch's state can change, which needs to be taken into account
	ProcessInternalCycle<true>();
	_dmaController->ResetIdleCounter(mode);

	if(addr < 0x8000000 || addr >= 0x10000000) {
		waitStates = _waitStates.GetWaitStates(mode, addr);
		if(_prefetch->NeedExec(_state.PrefetchEnabled)) {
			_prefetch->Exec(waitStates, _state.PrefetchEnabled);
		}
	} else if((mode & GbaAccessMode::Dma) || !(mode & GbaAccessMode::Prefetch)) {
		//Accesses to ROM from DMA or reads not caused by the CPU loading opcodes will reset the cartridge prefetcher
		//When the prefetch is reset on its last cycle, the ROM access takes an extra cycle to complete
		waitStates = _waitStates.GetWaitStates(mode, addr) + (int)_prefetch->Reset();
	} else {
		if((addr & 0x1FFFE) == 0) {
			//When reading a 128kb rom boundary, reset the prefetcher's state (and add the 1-cycle penalty as needed)
			mode &= ~GbaAccessMode::Sequential;
			uint8_t penalty = (uint8_t)_prefetch->Reset();
			waitStates = penalty + (_state.PrefetchEnabled ? _prefetch->Read<true>(mode, addr) : _prefetch->Read<false>(mode, addr));
		} else {
			waitStates = _state.PrefetchEnabled ? _prefetch->Read<true>(mode, addr) : _prefetch->Read<false>(mode, addr);
		}
	}
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

void GbaMemoryManager::ProcessVramAccess(GbaAccessModeVal mode, uint32_t addr)
{
	uint8_t memType = (addr - 0x4000000) >> 24;
	if(memType == 3) {
		//TODOGBA what about byte accesses? don't stall?
		memType = GbaPpuMemAccess::Oam;
	} else if(memType == GbaPpuMemAccess::Vram && addr & 0x10000) {
		//TODOGBA what about blocked accesses when in bitmap mode? don't stall?
		memType = GbaPpuMemAccess::VramObj;
	}

	ProcessInternalCycle<true>();
	ProcessVramStalling(memType);

	if(addr < 0x7000000 && (mode & GbaAccessMode::Word)) {
		ProcessInternalCycle<false>();
		ProcessVramStalling(memType);
	}
}

void GbaMemoryManager::ProcessVramStalling(uint8_t memType)
{
	RunPrefetch();
	_dmaController->ResetIdleCounter();

	_ppu->RenderScanline(true);
	while(_ppu->IsAccessingMemory(memType)) {
		//Block CPU until PPU is done accessing ram
		ProcessInternalCycle();
		RunPrefetch();
		_ppu->RenderScanline(true);
	}
}

template<uint8_t width>
void GbaMemoryManager::UpdateOpenBus(uint32_t addr, uint32_t value)
{
	if(addr >= 0x10000000) {
		//Accessing open bus addresses should probably not update the current open bus value
		//This is needed to pass the test rom that dumps the bios to save ram by abusing open bus (See: https://gist.github.com/profi200/c7fef99003fa5d07235d97296da23db3)
		//TODOGBA reading other open bus addresses probably should behave the same? (e.g registers that don't exist, etc.)
		return;
	}

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
	if(addr < 0x8000000 && addr >= 0x5000000) {
		ProcessVramAccess(mode, addr);
	} else {
		ProcessWaitStates(mode, addr);
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
		if(!(mode & GbaAccessMode::NoRotate) && (addr & 0x01)) {
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
		if(!(mode & GbaAccessMode::NoRotate) && (addr & 0x03)) {
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
	if(addr >= 0x5000000 && addr < 0x8000000) {
		ProcessVramAccess(mode, addr);
	} else {
		ProcessWaitStates(mode, addr);
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
				return _state.InternalOpenBus[addr & 0x03];
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
			_waitStates.GenerateWaitStateLut(_state);
			break;

		case 0x205:
			BitUtilities::SetBits<8>(_state.WaitControl, value & 0x7F);
			_state.PrgWaitStates2[0] = defaultWaitStates[value & 0x03] + 1;
			_state.PrgWaitStates2[1] = (value & 0x04) ? 2 : 9;
			_state.PrefetchEnabled = (value & 0x40);
			_waitStates.GenerateWaitStateLut(_state);
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
				//The CPU executes for one more clock before getting halted
				//This is needed to pass the 4 halt_pc tests
				_haltDelay = 1;
				SetPendingUpdateFlag();
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

void GbaMemoryManager::ProcessDmaStart()
{
	_dmaIrqCounter = 0;
	_dmaIrqLine = 0;
	_dmaIrqPending = 0;
	SetPendingUpdateFlag();
}

void GbaMemoryManager::TriggerIrqUpdate()
{
	_state.IrqUpdateCounter = 3;
	SetPendingUpdateFlag();
}

void GbaMemoryManager::TriggerObjEnableUpdate()
{
	_objEnableDelay = 3;
	SetPendingUpdateFlag();
}

void GbaMemoryManager::SetDelayedIrqSource(GbaIrqSource source, uint8_t delay)
{
	_pendingIrqs.push_back({ source, delay });
	SetPendingUpdateFlag();
}

void GbaMemoryManager::SetIrqSource(GbaIrqSource source)
{
	_state.NewIF |= (int)source;
	TriggerIrqUpdate();
}

bool GbaMemoryManager::HasPendingIrq()
{
	//Keep track of the IRQ line used by the CPU to decide if the IRQ handler should be executed
	//(requires the IF+IE IRQ flags and IME to all be set)
	return _irqFirstAccessCycle & 0x02;
}

bool GbaMemoryManager::IsHaltOver()
{
	if(_state.StopMode) {
		return _controlManager->CheckInputCondition();
	} else {
		return (bool)(_state.IrqPending & 0x01);
	}
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

	SV(_state.IE);
	SV(_state.IF);
	SV(_state.IME);

	SV(_state.NewIE);
	SV(_state.NewIF);
	SV(_state.NewIME);

	SV(_state.WaitControl);
	SV(_state.PrefetchEnabled);

	if(s.GetFormat() != SerializeFormat::Map) {
		SV(_hasPendingUpdates);
		SV(_hasPendingLateUpdates);

		SVVector(_pendingIrqs);

		SV(_haltModeUsed);
		SV(_biosLocked);
		SV(_irqFirstAccessCycle);
		SV(_haltDelay);
		SV(_objEnableDelay);

		SV(_dmaIrqCounter);
		SV(_dmaIrqPending);
		SV(_dmaIrqLine);

		SVArray(_state.BootRomOpenBus, 4);
		SVArray(_state.InternalOpenBus, 4);
		SVArray(_state.IwramOpenBus, 4);

		SVArray(_state.PrgWaitStates0, 2);
		SVArray(_state.PrgWaitStates1, 2);
		SVArray(_state.PrgWaitStates2, 2);
		SV(_state.SramWaitStates);

		SV(_state.IrqUpdateCounter);
		SV(_state.IrqPending);
		SV(_state.IrqLine);
		SV(_state.BusLocked);
		SV(_state.StopMode);
		SV(_state.PostBootFlag);
	}

	if(!s.IsSaving()) {
		_waitStates.GenerateWaitStateLut(_state);
	}
}