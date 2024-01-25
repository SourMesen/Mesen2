#include "pch.h"
#include "Gameboy/GbDmaController.h"
#include "Gameboy/GbMemoryManager.h"
#include "Gameboy/GbPpu.h"
#include "Gameboy/Gameboy.h"
#include "Gameboy/GbCpu.h"
#include "Utilities/Serializer.h"

void GbDmaController::Init(Gameboy* gameboy, GbMemoryManager* memoryManager, GbPpu* ppu, GbCpu* cpu)
{
	_gameboy = gameboy;
	_memoryManager = memoryManager;
	_ppu = ppu;
	_cpu = cpu;
	_state = {};
	_state.OamDmaSource = 0xFF;
}

GbDmaControllerState GbDmaController::GetState()
{
	return _state;
}

void GbDmaController::Exec()
{
	if(_cpu->IsHalted()) {
		//OAM DMA is halted while the CPU is in halt mode, and resumes when the CPU resumes
		return;
	}

	bool isRunning = _state.DmaCounter > 0;
	if(isRunning) {
		if(_state.DmaCounter <= 160) {
			_memoryManager->WriteDma(0xFE00 + (160 - _state.DmaCounter), _state.DmaReadBuffer);
		}

		_state.DmaCounter--;
		_state.OamDmaRunning = _state.DmaCounter > 0;

		if(_state.OamDmaRunning) {
			_state.DmaReadBuffer = _memoryManager->ReadDma(GetOamReadAddress());
		}
	}

	if(_state.DmaStartDelay > 0) {
		if(--_state.DmaStartDelay == 0) {
			_state.InternalDest = _state.OamDmaSource;
			_state.DmaCounter = 161;
			_state.OamDmaRunning = isRunning;
		}
	}
}

uint8_t GbDmaController::GetLastWriteAddress()
{
	return _state.DmaCounter <= 160 ? (160 - _state.DmaCounter) : 0;
}

bool GbDmaController::IsOamDmaConflict(uint16_t addr)
{
	uint8_t src = _state.OamDmaSource;
	if(_gameboy->IsCgb()) {
		return (
			(src < 0x80 && addr < 0x8000) ||
			(src >= 0x80 && src <= 0x9F && addr >= 0x8000 && addr <= 0x9FFF) ||
			(src >= 0xA0 && src <= 0xFD && addr >= 0xA000 && addr <= 0xFDFF)
		);
	} else {
		return (
			((src < 0x80 || (src >= 0xA0 && src <= 0xFD)) && (addr < 0x8000 || (addr >= 0xA000 && addr <= 0xFDFF))) ||
			(src >= 0x80 && src <= 0x9F && addr >= 0x8000 && addr <= 0x9FFF)
		);
	}
}

uint16_t GbDmaController::ProcessOamDmaReadConflict(uint16_t addr)
{
	if(IsOamDmaConflict(addr)) {
		//Conflict - DMA is reading from the rom/ram bus (0000-7FFF, A000-FDFF) at the same time as the CPU
		//The CPU will read the value that the DMA is currently reading
		return GetOamReadAddress();
	}
	return addr;
}

bool GbDmaController::IsOamDmaRunning()
{
	return _state.OamDmaRunning;
}

uint16_t GbDmaController::GetOamReadAddress()
{
	return (_state.InternalDest << 8) + (160 - _state.DmaCounter);
}

uint8_t GbDmaController::Read()
{
	return _state.OamDmaSource;
}

void GbDmaController::Write(uint8_t value)
{
	_state.DmaStartDelay = 1;
	_state.OamDmaSource = value;
}

uint8_t GbDmaController::ReadCgb(uint16_t addr)
{
	switch(addr) {
		case 0xFF55: return _state.CgbDmaLength | (_state.CgbHdmaRunning ? 0 : 0x80);
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
				} else {
					//4 cycles for setup
					_memoryManager->Exec();
					_memoryManager->Exec();

					do {
						ProcessDmaBlock();
					} while(_state.CgbDmaLength != 0x7F);
				}
			} else {
				//"After writing a value to HDMA5 that starts the HDMA copy, the upper bit (that indicates HDMA mode when set to '1') will be cleared"
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
		//4 cycles for setup
		_memoryManager->Exec();
		_memoryManager->Exec();

		ProcessDmaBlock();
	}
}

void GbDmaController::ProcessDmaBlock()
{
	//TODO check invalid dma sources/etc.
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
	}
}

void GbDmaController::Serialize(Serializer& s)
{
	SV(_state.OamDmaSource); SV(_state.DmaStartDelay); SV(_state.InternalDest); SV(_state.DmaCounter); SV(_state.DmaReadBuffer);
	SV(_state.CgbDmaDest); SV(_state.CgbDmaLength); SV(_state.CgbDmaSource); SV(_state.CgbHdmaRunning);
	SV(_state.OamDmaRunning);
}
