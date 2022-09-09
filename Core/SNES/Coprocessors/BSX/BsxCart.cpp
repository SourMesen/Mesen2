#include "pch.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "SNES/SnesConsole.h"
#include "SNES/SnesMemoryManager.h"
#include "SNES/BaseCartridge.h"
#include "SNES/Coprocessors/BSX/BsxCart.h"
#include "SNES/Coprocessors/BSX/BsxMemoryPack.h"
#include "SNES/Coprocessors/BSX/BsxSatellaview.h"
#include "SNES/RamHandler.h"
#include "SNES/MemoryMappings.h"
#include "Utilities/Serializer.h"

BsxCart::BsxCart(SnesConsole* console, BsxMemoryPack* memPack)
{
	_console = console;
	_memoryManager = _console->GetMemoryManager();
	_memPack = memPack;

	MemoryMappings* mm = _console->GetMemoryManager()->GetMemoryMappings();
	mm->RegisterHandler(0x00, 0x0F, 0x5000, 0x5FFF, this);
	mm->RegisterHandler(0x10, 0x1F, 0x5000, 0x5FFF, _console->GetCartridge()->GetSaveRamHandlers());

	//Override regular B-bus handler
	_satellaview.reset(new BsxSatellaview(console, mm->GetHandler(0x2000)));
	mm->RegisterHandler(0x00, 0x3F, 0x2000, 0x2FFF, _satellaview.get());
	mm->RegisterHandler(0x80, 0xBF, 0x2000, 0x2FFF, _satellaview.get());

	_psRamSize = 512 * 1024;
	_psRam = new uint8_t[_psRamSize];
	_console->GetEmulator()->RegisterMemory(MemoryType::BsxPsRam, _psRam, _psRamSize);
	_console->InitializeRam(_psRam, _psRamSize);

	for(uint32_t i = 0; i < _psRamSize / 0x1000; i++) {
		_psRamHandlers.push_back(unique_ptr<IMemoryHandler>(new RamHandler(_psRam, i * 0x1000, _psRamSize, MemoryType::BsxPsRam)));
	}

	Reset();
}
 
BsxCart::~BsxCart()
{
	delete[] _psRam;
}

uint8_t BsxCart::Read(uint32_t addr)
{
	uint8_t openBus = _memoryManager->GetOpenBus();
	if((addr & 0xFFFF) != 0x5000) {
		return openBus;
	} else {
		uint8_t reg = (addr >> 16) & 0x0F;
		
		if(reg <= 0x0D) {
			return (_regs[reg] << 7) | (openBus & 0x7F);
		} else {
			//E & F are write-only
			return openBus & 0x7F;
		}
	}
}

void BsxCart::Write(uint32_t addr, uint8_t value)
{
	if((addr & 0xFFFF) != 0x5000) {
		return;
	}

	uint8_t reg = (addr >> 16) & 0x0F;
	if(reg == 0x0E) {
		if(_dirty) {
			memcpy(_regs, _dirtyRegs, sizeof(_regs));
			UpdateMemoryMappings();
			_dirty = false;
		}
	} else {
		uint8_t regValue = (value >> 7);
		if(_regs[reg] != regValue) {
			_dirtyRegs[reg] = regValue;
			_dirty = true;
		}
	}
}

void BsxCart::UpdateMemoryMappings()
{
	MemoryMappings* mm = _console->GetMemoryManager()->GetMemoryMappings();
	vector<unique_ptr<IMemoryHandler>>& prgHandlers = _console->GetCartridge()->GetPrgRomHandlers();
	vector<unique_ptr<IMemoryHandler>>& memPackHandlers = _memPack->GetMemoryHandlers();

	uint8_t unmappedBank = (_regs[0x0B] << 5);
	uint8_t psRamBank = (_regs[0x05] << 4) | (_regs[0x06] << 5);

	if(!_regs[0x02]) {
		//LoROM

		//Memory pack mapping
		mm->RegisterHandler(0x00, 0x7D, 0x8000, 0xFFFF, memPackHandlers);
		mm->RegisterHandler(0x40, 0x7D, 0x0000, 0x7FFF, memPackHandlers);
		mm->RegisterHandler(0x80, 0xFF, 0x8000, 0xFFFF, memPackHandlers);
		mm->RegisterHandler(0xC0, 0xFF, 0x0000, 0x7FFF, memPackHandlers);

		//Memory hole mapping
		uint16_t unmappedAddr = _regs[0x0B] ? 0x0000 : 0x8000;
		if(_regs[0x09]) {
			mm->RegisterHandler(0x00 | (unmappedBank << 1), 0x1F | (unmappedBank << 1), unmappedAddr, 0xFFFF, nullptr);
		}

		if(_regs[0x0A]) {
			mm->RegisterHandler(0x80 | (unmappedBank << 1), 0x9F | (unmappedBank << 1), unmappedAddr, 0xFFFF, nullptr);
		}

		//PSRAM mapping
		uint16_t psRamAddr = (psRamBank & 0x20) ? 0x0000 : 0x8000;
		if(_regs[0x03]) {
			mm->RegisterHandler(0x00 | (psRamBank << 1), 0x0F | (psRamBank << 1), psRamAddr, 0xFFFF, _psRamHandlers);
			mm->RegisterHandler(0x70, 0x7D, 0x0000, 0x7FFF, _psRamHandlers);
		}

		if(_regs[0x04]) {
			mm->RegisterHandler(0x80 | (psRamBank << 1), 0x8F | (psRamBank << 1), psRamAddr, 0xFFFF, _psRamHandlers);
			mm->RegisterHandler(0xF0, 0xFF, 0x0000, 0x7FFF, _psRamHandlers);
		}
	} else {
		//HiROM
		
		//Memory pack mapping
		mm->RegisterHandler(0x00, 0x3F, 0x8000, 0xFFFF, memPackHandlers, 8);
		mm->RegisterHandler(0x40, 0x7D, 0x0000, 0xFFFF, memPackHandlers, 0);
		mm->RegisterHandler(0x80, 0xBF, 0x8000, 0xFFFF, memPackHandlers, 8);
		mm->RegisterHandler(0xC0, 0xFF, 0x0000, 0xFFFF, memPackHandlers, 0);

		//Memory hole mapping
		if(_regs[0x09]) {
			mm->RegisterHandler(0x00 | unmappedBank, 0x0F | unmappedBank, 0x8000, 0xFFFF, nullptr);
			mm->RegisterHandler(0x40 | unmappedBank, 0x4F | unmappedBank, 0x0000, 0xFFFF, nullptr);
		}

		if(_regs[0x0A]) {
			mm->RegisterHandler(0x80 | unmappedBank, 0x8F | unmappedBank, 0x8000, 0xFFFF, nullptr);
			mm->RegisterHandler(0xC0 | unmappedBank, 0xCF | unmappedBank, 0x0000, 0xFFFF, nullptr);
		}

		//PSRAM mapping
		if(_regs[0x03]) {
			//Lower Banks (0x00-0x7D)
			mm->RegisterHandler(0x00 | psRamBank, 0x07 | psRamBank, 0x8000, 0xFFFF, _psRamHandlers, 8);
			mm->RegisterHandler(0x40 | psRamBank, 0x47 | psRamBank, 0x0000, 0xFFFF, _psRamHandlers);
			mm->RegisterHandler(0x20, 0x3F, 0x6000, 0x7FFF, _psRamHandlers, 6);
		}

		if(_regs[0x04]) {
			//Higher Banks (0x80-0xFF)
			mm->RegisterHandler(0x80 | psRamBank, 0x87 | psRamBank, 0x8000, 0xFFFF, _psRamHandlers, 8);
			mm->RegisterHandler(0xC0 | psRamBank, 0xC7 | psRamBank, 0x0000, 0xFFFF, _psRamHandlers);
			mm->RegisterHandler(0xA0, 0xBF, 0x6000, 0x7FFF, _psRamHandlers, 6);
		}
	}

	//BS-X BIOS mapping (can override other mappings above)
	if(_regs[0x07]) {
		mm->RegisterHandler(0x00, 0x3F, 0x8000, 0xFFFF, prgHandlers);
	}
	if(_regs[0x08]) {
		mm->RegisterHandler(0x80, 0xBF, 0x8000, 0xFFFF, prgHandlers);
	}
}

void BsxCart::Reset()
{
	for(int i = 0; i < 0x10; i++) {
		_regs[i] = true;
	}

	_regs[0x04] = false;
	_regs[0x0A] = false;
	_regs[0x0C] = false;
	_regs[0x0D] = false;

	_dirty = false;
	memcpy(_dirtyRegs, _regs, sizeof(_regs));

	_satellaview->Reset();

	UpdateMemoryMappings();
}

void BsxCart::Serialize(Serializer& s)
{
	SVArray(_psRam, _psRamSize);
	SVArray(_regs, 0x10);
	SVArray(_dirtyRegs, 0x10);
	SV(_dirty);
	SV(_satellaview);

	if(!s.IsSaving()) {
		UpdateMemoryMappings();
	}
}

uint8_t BsxCart::Peek(uint32_t addr)
{
	return 0;
}

void BsxCart::PeekBlock(uint32_t addr, uint8_t* output)
{
	memset(output, 0, 0x1000);
}

AddressInfo BsxCart::GetAbsoluteAddress(uint32_t address)
{
	return { -1, MemoryType::None };
}

uint8_t* BsxCart::DebugGetPsRam()
{
	return _psRam;
}

uint32_t BsxCart::DebugGetPsRamSize()
{
	return _psRamSize;
}
