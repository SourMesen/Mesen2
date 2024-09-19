#include "pch.h"
#include "WS/WsDmaController.h"
#include "WS/WsMemoryManager.h"
#include "WS/APU/WsApu.h"
#include "Utilities/BitUtilities.h"
#include "Utilities/Serializer.h"

void WsDmaController::Init(WsMemoryManager* memoryManager, WsApu* apu)
{
	_memoryManager = memoryManager;
	_apu = apu;
}

void WsDmaController::RunGeneralDma()
{
	int offset = (_state.GdmaControl & 0x40) ? -2 : 2;

	if(_state.GdmaLength == 0 || _memoryManager->GetWaitStates(_state.GdmaSrc) > 1 || !_memoryManager->IsWordBus(_state.GdmaSrc)) {
		//When length is 0, or if transfer has an invalid source address, stop processing immediately
		//The 5 cycles of setup time aren't executed in this scenario
		return;
	}

	//GDMA takes 5+2*length cycles
	_memoryManager->Exec();
	_memoryManager->Exec();
	_memoryManager->Exec();
	_memoryManager->Exec();
	_memoryManager->Exec();

	while(_state.GdmaLength) {
		if(_memoryManager->GetWaitStates(_state.GdmaSrc) > 1 || !_memoryManager->IsWordBus(_state.GdmaSrc)) {
			//ROM with wait states and 8-bit buses are not allowed (SRAM, cart when 16-bit flag isn't set)
			break;
		}

		uint16_t value = _memoryManager->Read<uint16_t>((_state.GdmaSrc >> 4) & 0xF000, (uint16_t)_state.GdmaSrc, MemoryOperationType::DmaRead);
		_memoryManager->Write<uint16_t>(0, _state.GdmaDest, value, MemoryOperationType::DmaWrite);

		_state.GdmaSrc = (_state.GdmaSrc + offset) & 0xFFFFF;
		_state.GdmaDest = _state.GdmaDest + offset;
		_state.GdmaLength -= 2;
	}

	_state.GdmaControl &= ~0x80;
}

void WsDmaController::ProcessSoundDma()
{
	if(!_state.SdmaEnabled || _state.SdmaLength == 0) {
		//TODOWS what does enabling when length = 0 do?
		return;
	}

	if(_state.SdmaTimer == 0) {
		_state.SdmaTimer = _state.SdmaFrequency;
		
		//Sound DMA steals 6 cycles + N access cycles to run
		//4 cycles at the start + N read cycles + 1 dma write cycle
		_memoryManager->Exec();
		_memoryManager->Exec();
		_memoryManager->Exec();
		_memoryManager->Exec();
		_memoryManager->Exec();

		uint8_t sampleValue = _memoryManager->Read<uint8_t>((_state.SdmaSrc >> 4) & 0xF000, (uint16_t)_state.SdmaSrc, MemoryOperationType::DmaRead);

		if(_state.SdmaHold) {
			sampleValue = 0;
		}

		if(!_state.SdmaHold) {
			int offset = _state.SdmaDecrement ? -1 : 1;
			_state.SdmaSrc = (_state.SdmaSrc + offset) & 0xFFFFF;
			_state.SdmaLength--;

			if(_state.SdmaLength == 0) {
				if(_state.SdmaRepeat) {
					_state.SdmaSrc = _state.SdmaSrcReloadValue;
					_state.SdmaLength = _state.SdmaLengthReloadValue;
				} else {
					_state.SdmaEnabled = false;
					_state.SdmaControl &= 0x7F;
				}
			}
		} else {
			sampleValue = 0;
		}

		_memoryManager->Exec();
		_apu->WriteDma(_state.SdmaHyperVoice, sampleValue);
	} else {
		_state.SdmaTimer--;
	}
}

uint8_t WsDmaController::ReadPort(uint16_t port)
{
	switch(port) {
		case 0x40: return BitUtilities::GetBits<0>(_state.GdmaSrc);
		case 0x41: return BitUtilities::GetBits<8>(_state.GdmaSrc);
		case 0x42: return BitUtilities::GetBits<16>(_state.GdmaSrc);
		case 0x43: return 0;
		case 0x44: return BitUtilities::GetBits<0>(_state.GdmaDest);
		case 0x45: return BitUtilities::GetBits<8>(_state.GdmaDest);
		case 0x46: return BitUtilities::GetBits<0>(_state.GdmaLength);
		case 0x47: return BitUtilities::GetBits<8>(_state.GdmaLength);
		case 0x48: return _state.GdmaControl;

		case 0x4A: return BitUtilities::GetBits<0>(_state.SdmaSrc);
		case 0x4B: return BitUtilities::GetBits<8>(_state.SdmaSrc);
		case 0x4C: return BitUtilities::GetBits<16>(_state.SdmaSrc);
		case 0x4D: return 0;
		case 0x4E: return BitUtilities::GetBits<0>(_state.SdmaLength);
		case 0x4F: return BitUtilities::GetBits<8>(_state.SdmaLength);
		case 0x50: return BitUtilities::GetBits<16>(_state.SdmaLength);
		case 0x51: return 0;
		
		case 0x52: return _state.SdmaControl;
		case 0x53: return 0;
	}

	//TODOWS openbus
	return 0x90;
}

void WsDmaController::WritePort(uint16_t port, uint8_t value)
{
	switch(port) {
		case 0x40: BitUtilities::SetBits<0>(_state.GdmaSrc, value & 0xFE); break;
		case 0x41: BitUtilities::SetBits<8>(_state.GdmaSrc, value); break;
		case 0x42: BitUtilities::SetBits<16>(_state.GdmaSrc, value & 0x0F); break;
		case 0x43: break;
		case 0x44: BitUtilities::SetBits<0>(_state.GdmaDest, value & 0xFE); break;
		case 0x45: BitUtilities::SetBits<8>(_state.GdmaDest, value); break;
		case 0x46: BitUtilities::SetBits<0>(_state.GdmaLength, value & 0xFE); break;
		case 0x47: BitUtilities::SetBits<8>(_state.GdmaLength, value); break;
		case 0x48:
			_state.GdmaControl = value & 0xC0;
			if(_state.GdmaControl & 0x80) {
				RunGeneralDma();
			}
			break;

		case 0x4A:
			BitUtilities::SetBits<0>(_state.SdmaSrc, value);
			BitUtilities::SetBits<0>(_state.SdmaSrcReloadValue, value);
			break;

		case 0x4B:
			BitUtilities::SetBits<8>(_state.SdmaSrc, value);
			BitUtilities::SetBits<8>(_state.SdmaSrcReloadValue, value);
			break;
		
		case 0x4C:
			BitUtilities::SetBits<16>(_state.SdmaSrc, value & 0x0F);
			BitUtilities::SetBits<16>(_state.SdmaSrcReloadValue, value & 0x0F);
			break;

		case 0x4E:
			BitUtilities::SetBits<0>(_state.SdmaLength, value);
			BitUtilities::SetBits<0>(_state.SdmaLengthReloadValue, value);
			break;

		case 0x4F:
			BitUtilities::SetBits<8>(_state.SdmaLength, value);
			BitUtilities::SetBits<8>(_state.SdmaLengthReloadValue, value);
			break;

		case 0x50:
			BitUtilities::SetBits<16>(_state.SdmaLength, value & 0x0F);
			BitUtilities::SetBits<16>(_state.SdmaLengthReloadValue, value & 0x0F);
			break;

		case 0x52:
			_state.SdmaControl = value & 0xDF;

			switch(value & 0x03) {
				default:
				case 0: _state.SdmaFrequency = 5; break;
				case 1: _state.SdmaFrequency = 3; break;
				case 2: _state.SdmaFrequency = 1; break;
				case 3: _state.SdmaFrequency = 0; break;
			}
		
			_state.SdmaHold = value & 0x04;
			_state.SdmaRepeat = value & 0x08;
			_state.SdmaHyperVoice = value & 0x10;
			_state.SdmaDecrement = value & 0x40;
			_state.SdmaEnabled = value & 0x80;
			break;
	}
}

void WsDmaController::Serialize(Serializer& s)
{
	SV(_state.GdmaSrc);
	SV(_state.SdmaSrc);
	SV(_state.SdmaLength);
	SV(_state.SdmaSrcReloadValue);
	SV(_state.SdmaLengthReloadValue);
	SV(_state.GdmaDest);
	SV(_state.GdmaLength);
	SV(_state.GdmaControl);
	SV(_state.SdmaControl);
	SV(_state.SdmaEnabled);
	SV(_state.SdmaDecrement);
	SV(_state.SdmaHyperVoice);
	SV(_state.SdmaRepeat);
	SV(_state.SdmaHold);
	SV(_state.SdmaFrequency);
	SV(_state.SdmaTimer);
}
