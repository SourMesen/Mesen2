#include "pch.h"
#include "SNES/Coprocessors/OBC1/Obc1.h"
#include "SNES/SnesConsole.h"
#include "SNES/SnesMemoryManager.h"
#include "SNES/MemoryMappings.h"

Obc1::Obc1(SnesConsole* console, uint8_t* saveRam, uint32_t saveRamSize)
{
	MemoryMappings *mappings = console->GetMemoryManager()->GetMemoryMappings();	
	mappings->RegisterHandler(0x00, 0x3F, 0x6000, 0x7FFF, this);
	mappings->RegisterHandler(0x80, 0xBF, 0x6000, 0x7FFF, this);

	_ram = saveRam;
	_mask = saveRamSize - 1;
}

void Obc1::Reset()
{
}

uint8_t Obc1::ReadRam(uint16_t addr)
{
	return _ram[addr & _mask];
}

void Obc1::WriteRam(uint16_t addr, uint8_t value)
{
	_ram[addr & _mask] = value;
}

uint8_t Obc1::Read(uint32_t addr)
{
	addr &= 0x1FFF;

	switch(addr) {
		case 0x1FF0: return ReadRam(GetLowAddress());
		case 0x1FF1: return ReadRam(GetLowAddress() + 1);
		case 0x1FF2: return ReadRam(GetLowAddress() + 2);
		case 0x1FF3: return ReadRam(GetLowAddress() + 3);
		case 0x1FF4: return ReadRam(GetHighAddress());
	}
	return ReadRam(addr);
}

void Obc1::Write(uint32_t addr, uint8_t value)
{
	addr &= 0x1FFF;

	switch(addr) {
		case 0x1FF0: WriteRam(GetLowAddress(), value); break;
		case 0x1FF1: WriteRam(GetLowAddress() + 1, value); break;
		case 0x1FF2: WriteRam(GetLowAddress() + 2, value); break;
		case 0x1FF3: WriteRam(GetLowAddress() + 3, value); break;
		case 0x1FF4: {
			uint8_t shift = (ReadRam(0x1FF6) & 0x03) << 1;
			WriteRam(GetHighAddress(), ((value & 0x03) << shift) | (ReadRam(GetHighAddress()) & ~(0x03 << shift)));
			break;
		}

		default: WriteRam(addr, value); break;
	}
}

//$1FF5: Base address in bit 0 (0 = $1C00, 1 = $1800)
//$1FF6: "OAM" index (0-127) in first 7 bits
uint16_t Obc1::GetBaseAddress()
{
	return 0x1800 | ((~ReadRam(0x1FF5) & 0x01) << 10);
}

uint16_t Obc1::GetLowAddress()
{
	return GetBaseAddress() | ((ReadRam(0x1FF6) & 0x7F) << 2);
}

uint16_t Obc1::GetHighAddress()
{
	return (GetBaseAddress() | ((ReadRam(0x1FF6) & 0x7F) >> 2)) + 0x200;
}

void Obc1::Serialize(Serializer &s)
{
}

uint8_t Obc1::Peek(uint32_t addr)
{
	return 0;
}

void Obc1::PeekBlock(uint32_t addr, uint8_t *output)
{
	memset(output, 0, 0x1000);
}

AddressInfo Obc1::GetAbsoluteAddress(uint32_t address)
{
	return { -1, MemoryType::None };
}