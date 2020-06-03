#include "stdafx.h"
#include "GbDmaController.h"
#include "GbMemoryManager.h"
#include "GbPpu.h"

GbDmaController::GbDmaController(GbMemoryManager* memoryManager, GbPpu* ppu)
{
	_memoryManager = memoryManager;
	_ppu = ppu;
	_state = {};
}

void GbDmaController::Exec()
{
	if(_state.DmaCounter > 0) {
		if(_state.DmaCounter <= 160) {
			_memoryManager->WriteDma(0xFE00 + (160 - _state.DmaCounter), _state.DmaReadBuffer);
		}

		_state.DmaCounter--;
		_state.DmaReadBuffer = _memoryManager->ReadDma((_state.OamDmaDest << 8) + (160 - _state.DmaCounter));
	}

	if(_state.DmaStartDelay > 0) {
		if(--_state.DmaStartDelay == 0) {
			_state.InternalDest = _state.OamDmaDest;
			_state.DmaCounter = 161;
		}
	}
}

bool GbDmaController::IsOamDmaRunning()
{
	return _state.DmaCounter > 0 && _state.DmaCounter < 161;
}

uint8_t GbDmaController::Read()
{
	return _state.OamDmaDest;
}

void GbDmaController::Write(uint8_t value)
{
	_state.DmaStartDelay = 1;
	_state.OamDmaDest = value;
}

uint8_t GbDmaController::ReadCgb(uint16_t addr)
{
	switch(addr) {
		case 0xFF51: return _state.CgbDmaSource >> 8;
		case 0xFF52: return _state.CgbDmaSource & 0xFF;
		case 0xFF53: return _state.CgbDmaDest >> 8;
		case 0xFF54: return _state.CgbDmaDest & 0xFF;
		case 0xFF55: return _state.CgbDmaLength | (_state.CgbHdmaDone ? 0x80 : 0);
	}

	return 0;
}

void GbDmaController::WriteCgb(uint16_t addr, uint8_t value)
{
	switch(addr) {
		case 0xFF51: _state.CgbDmaSource = (_state.CgbDmaSource & 0xFF) | (value << 8); break;
		case 0xFF52: _state.CgbDmaSource = (_state.CgbDmaSource & 0xFF00) | (value & 0xF0); break;
		case 0xFF53: _state.CgbDmaDest = (_state.CgbDmaDest & 0xFF) | (value << 8); break;
		case 0xFF54: _state.CgbDmaDest = (_state.CgbDmaDest & 0xFF00) | (value & 0xF0); break;
		case 0xFF55: {
			bool hdmaMode = (value & 0x80) != 0;
			_state.CgbDmaLength = value & 0x7F;

			if(!hdmaMode) {
				if(_state.CgbHdmaRunning) {
					//"If HDMA5 is written during a HDMA copy, the behaviour depends on the written bit 7. 
					//  - New bit 7 is 0: Stop copy. HDMA5 new value is ( 80h OR written_value ).
					//  - New bit 7 is 1: Restart copy. New size is the value of written_value bits 0-6.
					//This means that HDMA can't switch to GDMA with only one write. It must be stopped first."
					_state.CgbHdmaRunning = false;
					_state.CgbHdmaDone = true;
				} else {
					do {
						ProcessDmaBlock();
					} while(_state.CgbDmaLength != 0x7F);
				}
			} else {
				//"After writing a value to HDMA5 that starts the HDMA copy, the upper bit (that indicates HDMA mode when set to '1') will be cleared"
				_state.CgbHdmaDone = false;
				_state.CgbHdmaRunning = true;

				//"If a HDMA transfer is started when the screen is off, one block is copied. "
				//" When a HDMA transfer is started during HBL it will start right away."
				if(!_ppu->IsLcdEnabled() || _ppu->GetMode() == PpuMode::HBlank) {
					ProcessHdma();
				}
			}
			break;
		}
	}
}

void GbDmaController::ProcessHdma()
{
	if(_state.CgbHdmaRunning) {
		ProcessDmaBlock();
	}
}

void GbDmaController::ProcessDmaBlock()
{
	//TODO check invalid dma sources/etc.
	//4 cycles for setup
	_memoryManager->Exec();
	_memoryManager->Exec();

	for(int i = 0; i < 16; i++) {
		uint16_t dst = 0x8000 | ((_state.CgbDmaDest + i) & 0x1FFF);

		//2 or 4 cycles per byte transfered (2x more cycles in high speed mode - effective speed is the same in both modes
		_memoryManager->Exec();
		uint8_t value = _memoryManager->Read<MemoryOperationType::DmaRead>(_state.CgbDmaSource + i);
		if(_memoryManager->IsHighSpeed()) {
			_memoryManager->Exec();
		}
		_memoryManager->Write<MemoryOperationType::DmaWrite>(dst, value);
	}

	//Source/Dest/Length are all modified by the DMA process and keep their last value after DMA completes
	_state.CgbDmaSource += 16;
	_state.CgbDmaDest += 16;
	_state.CgbDmaLength = (_state.CgbDmaLength - 1) & 0x7F;

	if(_state.CgbHdmaRunning && _state.CgbDmaLength == 0x7F) {
		_state.CgbHdmaRunning = false;
		_state.CgbHdmaDone = true;
	}
}

void GbDmaController::Serialize(Serializer& s)
{
	s.Stream(
		_state.OamDmaDest, _state.DmaStartDelay, _state.InternalDest, _state.DmaCounter, _state.DmaReadBuffer,
		_state.CgbDmaDest, _state.CgbDmaLength, _state.CgbDmaSource, _state.CgbHdmaDone, _state.CgbHdmaRunning
	);
}
