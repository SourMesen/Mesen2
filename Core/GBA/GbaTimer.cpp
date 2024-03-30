#include "pch.h"
#include "GBA/GbaTimer.h"
#include "GBA/GbaMemoryManager.h"
#include "GBA/APU/GbaApu.h"
#include "Utilities/Serializer.h"
#include "Utilities/BitUtilities.h"

void GbaTimer::Init(GbaMemoryManager* memoryManager, GbaApu* apu)
{
	_memoryManager = memoryManager;
	_apu = apu;
}

void GbaTimer::ClockTimer(int i)
{
	_state.Timer[i].Timer = _state.Timer[i].ReloadValue;
	if(i < 3) {
		if(_state.Timer[i + 1].Mode && _state.Timer[i + 1].Enabled) {
			if(++_state.Timer[i + 1].Timer == 0) {
				ClockTimer(i + 1);
			}
		}
	}

	if(_state.Timer[i].IrqEnabled) {
		_memoryManager->SetIrqSource((GbaIrqSource)((int)GbaIrqSource::Timer0 << i));
	}

	_apu->ClockFifo(i);
}

void GbaTimer::ProcessPendingWrites()
{
	_hasPendingWrites = false;
	for(int i = 0; i < 4; i++) {
		GbaTimerState& timer = _state.Timer[i];
		if(timer.WritePending) {
			timer.WritePending = false;

			switch(timer.Control & 0x03) {
				case 0: timer.PrescaleMask = 0; break;
				case 1: timer.PrescaleMask = 0x3F; break;
				case 2: timer.PrescaleMask = 0xFF; break;
				case 3: timer.PrescaleMask = 0x3FF; break;
			}
			timer.Mode = timer.Control & 0x04;
			timer.IrqEnabled = timer.Control & 0x40;

			bool enabled = (timer.Control & 0x80);
			if(!timer.Enabled && enabled) {
				timer.EnableDelay = 2;
				timer.Enabled = true;
				_hasPendingTimers = true;
				_memoryManager->SetPendingUpdateFlag();
			} else if(timer.Enabled && !enabled) {
				timer.Enabled = false;
				timer.ProcessTimer = false;
				timer.EnableDelay = 0;
			}
			
			timer.ReloadValue = timer.NewReloadValue;
		}
	}
}

void GbaTimer::ProcessPendingTimers()
{
	_hasPendingTimers = false;
	for(int i = 0; i < 4; i++) {
		GbaTimerState& timer = _state.Timer[i];

		if(timer.EnableDelay) {
			timer.EnableDelay--;
			if(timer.EnableDelay == 1) {
				timer.Timer = timer.ReloadValue;
				if(timer.Mode) {
					timer.EnableDelay = 0;
					timer.ProcessTimer = false;
				}
			} else if(timer.EnableDelay == 0) {
				timer.ProcessTimer = true;
			}
			_hasPendingTimers |= timer.EnableDelay != 0;
		}
	}
}

void GbaTimer::TriggerUpdate(GbaTimerState& timer)
{
	timer.WritePending = true;
	_hasPendingWrites = true;
	_memoryManager->SetPendingLateUpdateFlag();
}

void GbaTimer::WriteRegister(uint32_t addr, uint8_t value)
{
	uint8_t timerIndex = (addr & 0x0C) >> 2;
	GbaTimerState& timer = _state.Timer[timerIndex];
	switch(addr) {
		case 0x100: case 0x104: case 0x108: case 0x10C:
			BitUtilities::SetBits<0>(timer.NewReloadValue, value);
			TriggerUpdate(timer);
			break;

		case 0x101: case 0x105: case 0x109: case 0x10D:
			BitUtilities::SetBits<8>(timer.NewReloadValue, value);
			TriggerUpdate(timer);
			break;

		case 0x102: case 0x106: case 0x10A: case 0x10E: {
			if(timerIndex == 0) {
				//Mode is always disabled and always returns 0 for timer 0
				value &= ~0x04;
			}

			timer.Control = value & 0xC7;
			TriggerUpdate(timer);
			break;
		}

		case 0x103: case 0x107: case 0x10B: case 0x10F:
			break;
	}
}

uint8_t GbaTimer::ReadRegister(uint32_t addr)
{
	GbaTimerState& timer = _state.Timer[(addr & 0x0C) >> 2];
	switch(addr) {
		case 0x100: case 0x104: case 0x108: case 0x10C: return BitUtilities::GetBits<0>(timer.Timer);
		case 0x101: case 0x105: case 0x109: case 0x10D: return BitUtilities::GetBits<8>(timer.Timer);
		case 0x102: case 0x106: case 0x10A: case 0x10E: return timer.Control;
		case 0x103: case 0x107: case 0x10B: case 0x10F: return 0;
	}

	return 0;
}

void GbaTimer::Serialize(Serializer& s)
{
	for(int i = 0; i < 4; i++) {
		SVI(_state.Timer[i].ReloadValue);
		SVI(_state.Timer[i].NewReloadValue);
		SVI(_state.Timer[i].Control);
		SVI(_state.Timer[i].PrescaleMask);
		SVI(_state.Timer[i].Timer);
		SVI(_state.Timer[i].EnableDelay);
		SVI(_state.Timer[i].WritePending);
		SVI(_state.Timer[i].Mode);
		SVI(_state.Timer[i].IrqEnabled);
		SVI(_state.Timer[i].Enabled);
		SVI(_state.Timer[i].ProcessTimer);
	}
	SV(_hasPendingTimers);
}
