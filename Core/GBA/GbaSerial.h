#pragma once
#include "pch.h"
#include "GBA/GbaTypes.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Utilities/BitUtilities.h"
#include "Utilities/Serializer.h"

class GbaSerial final : public ISerializable
{
private:
	GbaSerialState _state = {};
	GbaMemoryManager* _memoryManager = nullptr;
	
	void UpdateState()
	{
		if(_state.Active && _memoryManager->GetMasterClock() >= _state.EndMasterClock) {
			_state.Active = false;
			_state.Control &= ~0x80;
		}
	}

public:
	void Init(Emulator* emu, GbaMemoryManager* memoryManager)
	{
		_memoryManager = memoryManager;
		_state.IrqMasterClock = UINT64_MAX;

		if(emu->GetSettings()->GetGbaConfig().SkipBootScreen) {
			//BIOS leaves serial registers in this state, some games expect this
			_state.Mode = 0x8000;
		}
	}

	__forceinline bool HasPendingIrq()
	{
		return _state.IrqMasterClock != UINT64_MAX;
	}

	void CheckForIrq(uint64_t masterClock)
	{
		if(masterClock >= _state.IrqMasterClock) {
			_memoryManager->SetIrqSource(GbaIrqSource::Serial);
			_state.IrqMasterClock = UINT64_MAX;
		}
	}

	uint8_t ReadRegister(uint32_t addr, bool peek)
	{
		//TODOGBA - serial support
		switch(addr) {
			case 0x120: case 0x122: case 0x124: case 0x126: 
				return BitUtilities::GetBits<0>(_state.Data[(addr & 0x06) >> 1]);

			case 0x121: case 0x123: case 0x125: case 0x127:
				return BitUtilities::GetBits<8>(_state.Data[(addr & 0x06) >> 1]);

			case 0x128:
				if(peek) {
					uint8_t control = _state.Control;
					if(_state.Active && _memoryManager->GetMasterClock() >= _state.EndMasterClock) {
						control &= ~0x80;
					}
					return control;
				} else {
					UpdateState();
					return BitUtilities::GetBits<0>(_state.Control);
				}

			case 0x129: return BitUtilities::GetBits<8>(_state.Control);

			case 0x12A: return BitUtilities::GetBits<0>(_state.SendData);
			case 0x12B: return BitUtilities::GetBits<8>(_state.SendData);
			
			case 0x134: return BitUtilities::GetBits<0>(_state.Mode);
			case 0x135: return BitUtilities::GetBits<8>(_state.Mode);
			case 0x136: return 0;
			case 0x137: return 0;

			case 0x140: return BitUtilities::GetBits<0>(_state.JoyControl);
			case 0x141: return BitUtilities::GetBits<8>(_state.JoyControl);
			case 0x142: return 0;
			case 0x143: return 0;

			case 0x150: return BitUtilities::GetBits<0>(_state.JoyReceive);
			case 0x151: return BitUtilities::GetBits<8>(_state.JoyReceive);
			case 0x152: return BitUtilities::GetBits<16>(_state.JoyReceive);
			case 0x153: return BitUtilities::GetBits<24>(_state.JoyReceive);

			case 0x154: return BitUtilities::GetBits<0>(_state.JoySend);
			case 0x155: return BitUtilities::GetBits<8>(_state.JoySend);
			case 0x156: return BitUtilities::GetBits<16>(_state.JoySend);
			case 0x157: return BitUtilities::GetBits<24>(_state.JoySend);

			case 0x158: return _state.JoyStatus;
		}

		return _memoryManager->GetOpenBus(addr);
	}

	void WriteRegister(uint32_t addr, uint8_t value)
	{
		//TODOGBA - serial support
		switch(addr) {
			case 0x120: case 0x122: case 0x124: case 0x126:
				BitUtilities::SetBits<0>(_state.Data[(addr & 0x06) >> 1], value);
				break;

			case 0x121: case 0x123: case 0x125: case 0x127:
				BitUtilities::SetBits<8>(_state.Data[(addr & 0x06) >> 1], value);
				break;

			case 0x128: {
				UpdateState();

				BitUtilities::SetBits<0>(_state.Control, value);

				_state.InternalShiftClock = value & 0x01;
				_state.InternalShiftClockSpeed2MHz = value & 0x02;
				bool active = value & 0x80;
				if(active && !_state.Active) {
					_state.StartMasterClock = _memoryManager->GetMasterClock();
					_state.EndMasterClock = _state.StartMasterClock + (_state.InternalShiftClockSpeed2MHz ? 8 : 64) * (_state.TransferWord ? 32 : 8);
					if(_state.IrqEnabled) {
						_state.IrqMasterClock = _state.EndMasterClock;
						_memoryManager->SetPendingUpdateFlag();
					}
				}
				_state.Active = active;
				break;
			}
			case 0x129:
				UpdateState();

				BitUtilities::SetBits<8>(_state.Control, value);

				_state.TransferWord = value & 0x10;
				_state.IrqEnabled = value & 0x40;
				if(!_state.IrqEnabled) {
					_state.IrqMasterClock = UINT64_MAX;
				}

				if(_state.Active) {
					if(_state.StartMasterClock == _memoryManager->GetMasterClock()) {
						//Update end based on params
						_state.EndMasterClock = _state.StartMasterClock + (_state.InternalShiftClockSpeed2MHz ? 8 : 64) * (_state.TransferWord ? 32 : 8);
					}
					if(_state.IrqEnabled) {
						_state.IrqMasterClock = _state.EndMasterClock;
						_memoryManager->SetPendingUpdateFlag();
					}
				}
				break;

			case 0x12A: BitUtilities::SetBits<0>(_state.SendData, value); break;
			case 0x12B: BitUtilities::SetBits<8>(_state.SendData, value); break;

			case 0x134: BitUtilities::SetBits<0>(_state.Mode, value); break;
			case 0x135: BitUtilities::SetBits<8>(_state.Mode, value & 0xC1); break;

			case 0x140: BitUtilities::SetBits<0>(_state.JoyControl, value); break;
			case 0x141: BitUtilities::SetBits<8>(_state.JoyControl, value); break;

			case 0x150: BitUtilities::SetBits<0>(_state.JoyReceive, value); break;
			case 0x151: BitUtilities::SetBits<8>(_state.JoyReceive, value); break;
			case 0x152: BitUtilities::SetBits<16>(_state.JoyReceive, value); break;
			case 0x153: BitUtilities::SetBits<24>(_state.JoyReceive, value); break;

			case 0x154: BitUtilities::SetBits<0>(_state.JoySend, value); break;
			case 0x155: BitUtilities::SetBits<8>(_state.JoySend, value); break;
			case 0x156: BitUtilities::SetBits<16>(_state.JoySend, value); break;
			case 0x157: BitUtilities::SetBits<24>(_state.JoySend, value); break;

			case 0x158: _state.JoyStatus = value; break;
		}
	}

	void Serialize(Serializer& s) override
	{
		SV(_state.StartMasterClock);
		SV(_state.EndMasterClock);
		SV(_state.IrqMasterClock);

		SVArray(_state.Data, 4);

		SV(_state.Control);
		SV(_state.InternalShiftClock);
		SV(_state.InternalShiftClockSpeed2MHz);
		SV(_state.Active);
		SV(_state.TransferWord);
		SV(_state.IrqEnabled);

		SV(_state.SendData);
		SV(_state.Mode);
		SV(_state.JoyControl);
		SV(_state.JoyReceive);
		SV(_state.JoySend);
		SV(_state.JoyStatus);
	}
};