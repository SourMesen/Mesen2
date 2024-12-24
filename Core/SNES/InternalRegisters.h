#pragma once
#include "pch.h"
#include "SNES/AluMulDiv.h"
#include "SNES/SnesCpu.h"
#include "SNES/SnesPpu.h"
#include "SNES/SnesMemoryManager.h"
#include "SNES/InternalRegisterTypes.h"
#include "Utilities/ISerializable.h"

class SnesConsole;
class SnesMemoryManager;
class SnesControlManager;

class InternalRegisters final : public ISerializable
{
private:
	SnesConsole* _console = nullptr;
	SnesCpu* _cpu = nullptr;
	SnesPpu* _ppu = nullptr;
	SnesMemoryManager* _memoryManager = nullptr;
	SnesControlManager* _controlManager = nullptr;

	AluMulDiv _aluMulDiv = {};

	InternalRegisterState _state = {};
	bool _nmiFlag = false;
	bool _irqLevel = false;
	uint8_t _needIrq = 0;
	bool _irqFlag = false;
	bool _irqEnabled = false;

	uint16_t _hCounter = 0;
	uint16_t _vCounter = 0;

	uint64_t _autoReadClockStart = 0;
	uint64_t _autoReadNextClock = 0;
	bool _autoReadActive = false;
	bool _autoReadDisabled = true;
	uint8_t _autoReadPort1Value = 0;
	uint8_t _autoReadPort2Value = 0;

	void SetIrqFlag(bool irqFlag);
	
	uint8_t ReadControllerData(uint8_t port, bool getMsb);

	__forceinline void UpdateIrqLevel();

public:
	InternalRegisters();
	void Initialize(SnesConsole* console);

	void Reset();

	void SetAutoJoypadReadClock();
	void ProcessAutoJoypad();

	__forceinline void ProcessIrqCounters();

	uint8_t GetIoPortOutput();
	void SetNmiFlag(bool nmiFlag);

	bool IsVerticalIrqEnabled() { return _state.EnableVerticalIrq; }
	bool IsHorizontalIrqEnabled() { return _state.EnableHorizontalIrq; }
	bool IsNmiEnabled() { return _state.EnableNmi; }
	bool IsFastRomEnabled() { return _state.EnableFastRom; }
	uint16_t GetHorizontalTimer() { return _state.HorizontalTimer; }
	uint16_t GetVerticalTimer() { return _state.VerticalTimer; }
	
	uint8_t Peek(uint16_t addr);
	uint8_t Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t value);

	InternalRegisterState GetState();
	AluState GetAluState();

	void Serialize(Serializer &s) override;
};

void InternalRegisters::UpdateIrqLevel()
{
	if(!_irqEnabled) {
		_irqLevel = false;
		return;
	}

	bool irqLevel = (
		(!_state.EnableHorizontalIrq || _state.HorizontalTimer == _hCounter) &&
		(!_state.EnableVerticalIrq || _state.VerticalTimer == _vCounter)
	);

	if(!_irqLevel && irqLevel) {
		if(_state.EnableHorizontalIrq && _memoryManager->GetHClock() == 6) {
			_needIrq = 3;
		} else {
			_needIrq = 2;
		}
	}

	_irqLevel = irqLevel;
}

void InternalRegisters::ProcessIrqCounters()
{
	//This circuit runs at MasterClock/4, and the signal is inverted. This means it ticks at hclock 2, 6, etc. on every scanline.
	//VBLANK is cleared by the PPU at H=0, and which will be picked up on H=2
	//HBLANK however is cleared by the PPU at H=4, and this will only be picked up at H=6, which causes IRQs that depend on the value of H
	//to be delayed by an extra 4 cycles compared to IRQs that trigger only based on V.
	//Once VBLANK is cleared, a 4-cycle reset signal is triggered (from H=2 to H=6) and the V counter is reset to 0 (at H=2)
	//Once HBLANK is cleared (every scanline), a 4-cycle reset signal is triggered and the H counter is reset to 0 (at H=6) and then starts
	//counting again start from H=14+.
	//HBLANK getting cleared also triggers a V counter increment (at H=6).
	//When the H/V counters match the values in $4207-420A (+ the correct IRQ enable flags are turned on), an IRQ is triggered 2 ticks (8 master clocks) later.
	//Some not-quite-figured out edge cases remain:
	// -IRQs for H=0 (i.e when 4207/8 == 0) seem to be delayed by an extra tick (4 master clocks)?
	// -What's causing the 2 ticks/8 master clock delay for the IRQ signal after it was detected?
	// -What's causing the 1 tick/4 master clock delay for the NMI signal after it was detected? (e.g why does the CPU behave like it was set on H=6 instead of H=2?)
	//Notes:
	// -IRQs can't trigger on V=261 H=339 (reg values), because V gets reset to 0 at H=2 at the same time as H increments to 339. This might mean that an IRQ at
	//  V=0 and H=339 triggers both on scanline 0 and scanline 1 (at H=2)? (unverified)
	//See CPU schematics here: https://github.com/rgalland/SNES_S-CPU_Schematics/
	if(_needIrq > 0) {
		_needIrq--;
		if(_needIrq == 1) {
			SetIrqFlag(true);
		} else if(_needIrq == 0) {
			if(_irqFlag) {
				_cpu->SetIrqSource(SnesIrqSource::Ppu);
			} else {
				_cpu->ClearIrqSource(SnesIrqSource::Ppu);
			}
		}
	}

	uint16_t hClock = _memoryManager->GetHClock();

	if(hClock > 10) {
		_hCounter++;
	} else if(hClock == 10) {
		_hCounter = 0;
	} else if(hClock == 6) {
		_hCounter = 0;
		if(_ppu->GetScanline() > 0 && !_ppu->IsInOverclockedScanline()) {
			_vCounter++;
		}

		if(_state.EnableNmi && _ppu->GetScanline() == _ppu->GetNmiScanline()) {
			_cpu->SetNmiFlag(1);
		}
	} else if(hClock == 2) {
		_hCounter++;
		if(_ppu->GetScanline() == _ppu->GetNmiScanline()) {
			_nmiFlag = true;
		} else if(_ppu->GetScanline() == 0) {
			_nmiFlag = false;
			_vCounter = 0;
		}
	}

	UpdateIrqLevel();
}