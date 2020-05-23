#include "stdafx.h"
#include "GbDmaController.h"
#include "MessageManager.h"
#include "CpuTypes.h"
#include "GbMemoryManager.h"

GbDmaController::GbDmaController(GbMemoryManager* memoryManager)
{
	_memoryManager = memoryManager;
	_state = {};
}

void GbDmaController::Exec()
{
	if(_state.DmaCounter > 0) {
		if(_state.DmaCounter <= 160) {
			_memoryManager->Write(0xFE00 + (160 - _state.DmaCounter), _state.DmaReadBuffer);
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
		case 0xFF55: return _state.CgbDmaLength | (_state.CgbHdmaMode ? 0x80 : 0);
	}

	return 0;
}

void GbDmaController::WriteCgb(uint16_t addr, uint8_t value)
{
	switch(addr) {
		case 0xFF51: _state.CgbDmaSource = (_state.CgbDmaSource & 0xFF) | (value << 8); break;
		case 0xFF52: _state.CgbDmaSource = (_state.CgbDmaSource & 0xFF00) | value; break;
		case 0xFF53: _state.CgbDmaDest = (_state.CgbDmaDest & 0xFF) | (value << 8); break;
		case 0xFF54: _state.CgbDmaDest = (_state.CgbDmaDest & 0xFF00) | value; break;
		case 0xFF55:
			_state.CgbDmaLength = value & 0x7F;
			_state.CgbHdmaMode = (value & 0x80) != 0;

			if(!_state.CgbHdmaMode) {
				//TODO check invalid dma sources/etc.
				//TODO timing
				for(int i = 0; i < _state.CgbDmaLength * 16; i++) {
					uint16_t dst = 0x8000 | (((_state.CgbDmaDest & 0x1FF0) + i) & 0x1FFF);
					_memoryManager->Write(dst, _memoryManager->Read((_state.CgbDmaSource & 0xFFF0) + i, MemoryOperationType::DmaRead));
				}
				_state.CgbDmaLength = 0x7F;
			} else {
				MessageManager::Log("TODO HDMA");
			}
			break;
	}
}

void GbDmaController::Serialize(Serializer& s)
{
	s.Stream(
		_state.OamDmaDest, _state.DmaStartDelay, _state.InternalDest, _state.DmaCounter, _state.DmaReadBuffer,
		_state.CgbDmaDest, _state.CgbDmaLength, _state.CgbDmaSource, _state.CgbHdmaMode
	);
}
