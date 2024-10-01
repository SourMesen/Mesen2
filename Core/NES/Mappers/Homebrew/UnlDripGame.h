#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"
#include "NES/Mappers/Homebrew/UnlDripGameAudio.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"

class UnlDripGame : public BaseMapper
{
private:
	unique_ptr<UnlDripGameAudio> _audioChannels[2];
	uint8_t _extendedAttributes[2][0x400] = {};
	uint8_t _lowByteIrqCounter = 0;
	uint16_t _irqCounter = 0;
	uint16_t _lastNametableFetchAddr = 0;
	bool _irqEnabled = false;
	bool _extAttributesEnabled = false;
	bool _wramWriteEnabled = false;

protected:
	uint32_t GetDipSwitchCount() override { return 1; }
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x800; }
	bool AllowRegisterRead() override { return true; }
	uint16_t RegisterStartAddress() override { return 0x8000; }
	uint16_t RegisterEndAddress() override { return 0xFFFF; }
	bool EnableCpuClockHook() override { return true; }
	bool EnableCustomVramRead() override { return true; }

	void InitMapper() override
	{
		_audioChannels[0].reset(new UnlDripGameAudio(_console));
		_audioChannels[1].reset(new UnlDripGameAudio(_console));

		_lowByteIrqCounter = 0;
		_irqCounter = 0;
		_irqEnabled = false;
		_extAttributesEnabled = false;
		_wramWriteEnabled = false;
		_lastNametableFetchAddr = 0;

		_console->InitializeRam(_extendedAttributes[0], 0x400);
		_console->InitializeRam(_extendedAttributes[1], 0x400);

		SelectPrgPage(1, -1);

		AddRegisterRange(0x4800, 0x5FFF, MemoryOperation::Read);
		RemoveRegisterRange(0x8000, 0xFFFF, MemoryOperation::Read);
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);

		SVArray(_extendedAttributes[0], 0x400);
		SVArray(_extendedAttributes[1], 0x400);
		SV(_audioChannels[0]);
		SV(_audioChannels[1]);

		SV(_lowByteIrqCounter);
		SV(_irqCounter);
		SV(_irqEnabled);
		SV(_extAttributesEnabled);
		SV(_wramWriteEnabled);
		
		if(!s.IsSaving()) {
			UpdateWorkRamState();
		}
	}

	void ProcessCpuClock() override
	{
		BaseProcessCpuClock();

		if(_irqEnabled) {
			if(_irqCounter > 0) {
				_irqCounter--;
				if(_irqCounter == 0) {
					//While the IRQ counter is enabled, the timer is decremented once per CPU
					//cycle.Once the timer reaches zero, the /IRQ line is set to logic 0 and the
					//timer stops decrementing
					_irqEnabled = false;
					_console->GetCpu()->SetIrqSource(IRQSource::External);
				}
			}
		}

		_audioChannels[0]->Clock();
		_audioChannels[1]->Clock();
	}

	void UpdateWorkRamState()
	{
		SetCpuMemoryMapping(0x6000, 0x7FFF, 0, PrgMemoryType::WorkRam, _wramWriteEnabled ? MemoryAccessType::ReadWrite : MemoryAccessType::Read);
	}
	
	uint8_t MapperReadVram(uint16_t addr, MemoryOperationType memoryOperationType) override
	{
		if(_extAttributesEnabled && memoryOperationType == MemoryOperationType::PpuRenderingRead) {
			if(addr >= 0x2000 && (addr & 0x3FF) < 0x3C0) {
				//Nametable fetches
				_lastNametableFetchAddr = addr & 0x03FF;
			} else if(addr >= 0x2000 && (addr & 0x3FF) >= 0x3C0) {
				//Attribute fetches
				uint8_t bank;
				switch(GetMirroringType()) {
					default:
					case MirroringType::ScreenAOnly: bank = 0; break;
					case MirroringType::ScreenBOnly: bank = 1; break;
					case MirroringType::Horizontal: bank = (addr & 0x800) ? 1 : 0; break;
					case MirroringType::Vertical: bank = (addr & 0x400) ? 1 : 0; break;
				}
				
				//Return a byte containing the same palette 4 times - this allows the PPU to select the right palette no matter the shift value
				uint8_t value = _extendedAttributes[bank][_lastNametableFetchAddr & 0x3FF] & 0x03;
				return (value << 6) | (value << 4) | (value << 2) | value;
			}
		}
		return InternalReadVram(addr);
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		switch(addr & 0x5800) {
			case 0x4800: return (GetDipSwitches() ? 0x80 : 0) | 0x64;
			case 0x5000: return _audioChannels[0]->ReadRegister();
			case 0x5800: return _audioChannels[1]->ReadRegister();
		}
		return 0;
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr <= 0xBFFF) {
			switch(addr & 0x800F) {
				case 0x8000: case 0x8001: case 0x8002: case 0x8003:
					_audioChannels[0]->WriteRegister(addr, value);
					break;

				case 0x8004: case 0x8005: case 0x8006: case 0x8007:
					_audioChannels[1]->WriteRegister(addr, value);
					break;

				case 0x8008:
					_lowByteIrqCounter = value;
					break;

				case 0x8009:
					//Data written to the IRQ Counter Low register is buffered until writing to IRQ
					//Counter High, at which point the composite data is written directly to the IRQ	timer.
					_irqCounter = ((value & 0x7F) << 8) | _lowByteIrqCounter;
					_irqEnabled = (value & 0x80) != 0;

					//Writing to the IRQ Enable register will acknowledge the interrupt and return the /IRQ signal to logic 1.
					_console->GetCpu()->ClearIrqSource(IRQSource::External);
					break;

				case 0x800A:
					switch(value & 0x03) {
						case 0: SetMirroringType(MirroringType::Vertical); break;
						case 1: SetMirroringType(MirroringType::Horizontal); break;
						case 2: SetMirroringType(MirroringType::ScreenAOnly); break;
						case 3: SetMirroringType(MirroringType::ScreenBOnly); break;
					}
					_extAttributesEnabled = (value & 0x04) != 0;
					_wramWriteEnabled = (value & 0x08) != 0;
					UpdateWorkRamState();
					break;

				case 0x800B: SelectPrgPage(0, value & 0x0F); break;
				case 0x800C: SelectChrPage(0, value & 0x0F); break;
				case 0x800D: SelectChrPage(1, value & 0x0F); break;
				case 0x800E: SelectChrPage(2, value & 0x0F); break;
				case 0x800F: SelectChrPage(3, value & 0x0F); break;
			}
		} else {
			//Attribute expansion memory at $C000-$C7FF is mirrored throughout $C000-$FFFF.
			_extendedAttributes[(addr & 0x400) ? 1 : 0][addr & 0x3FF] = value;
		}
	}
};
