#include "pch.h"
#include "PCE/CdRom/PceArcadeCard.h"
#include "PCE/PceConsole.h"
#include "Shared/EmuSettings.h"
#include "Shared/Emulator.h"
#include "Utilities/Serializer.h"
#include "Shared/MessageManager.h"

PceArcadeCard::PceArcadeCard(PceConsole* console, Emulator* emu)
{
	_emu = emu;
	_ram = new uint8_t[PceArcadeCard::ArcadeRamMemSize];
	_isRamUsed = false;
	console->InitializeRam(_ram, PceArcadeCard::ArcadeRamMemSize);
	emu->RegisterMemory(MemoryType::PceArcadeCardRam, _ram, PceArcadeCard::ArcadeRamMemSize);

	_mappedBanks[0x40] = true;
	_mappedBanks[0x41] = true;
	_mappedBanks[0x42] = true;
	_mappedBanks[0x43] = true;
	_mappedBanks[0xFF] = true;
}

PceArcadeCard::~PceArcadeCard()
{
	delete[] _ram;
}

uint32_t PceArcadeCard::GetAddress(PceArcadeCardPortConfig& port)
{
	uint32_t addr = port.BaseAddress;
	if(port.AddOffset) {
		addr += port.Offset;
		if(port.SignedOffset) {
			addr += 0xFF0000;
		}
	}
	return addr & 0x1FFFFF;
}

void PceArcadeCard::ProcessAutoInc(PceArcadeCardPortConfig& port)
{
	if(port.AutoIncrement) {
		if(port.AddIncrementToBase) {
			port.BaseAddress = (port.BaseAddress + port.IncValue) & 0xFFFFFF;
		} else {
			port.Offset += port.IncValue;
		}
	}
}

void PceArcadeCard::AddOffsetToBase(PceArcadeCardPortConfig& port)
{
	uint32_t addr = port.BaseAddress + port.Offset;
	if(port.SignedOffset) {
		addr += 0xFF0000;
	}
	port.BaseAddress = addr & 0xFFFFFF;
}

uint8_t PceArcadeCard::ReadPortValue(uint8_t portNumber)
{
	PceArcadeCardPortConfig& port = _state.Port[portNumber];
	uint32_t addr = GetAddress(port);
	ProcessAutoInc(port);
	uint8_t value = _ram[addr];
	_emu->ProcessMemoryAccess<CpuType::Pce, MemoryType::PceArcadeCardRam, MemoryOperationType::Read>(addr, value);
	return value;
}

void PceArcadeCard::WritePortValue(uint8_t portNumber, uint8_t value)
{
	PceArcadeCardPortConfig& port = _state.Port[portNumber];
	uint32_t addr = GetAddress(port);
	ProcessAutoInc(port);
	_isRamUsed = true;
	_emu->ProcessMemoryAccess<CpuType::Pce, MemoryType::PceArcadeCardRam, MemoryOperationType::Write>(addr, value);
	_ram[addr] = value;
}

uint8_t PceArcadeCard::ReadPortRegister(uint8_t portNumber, uint8_t reg)
{
	PceArcadeCardPortConfig& port = _state.Port[portNumber];

	//LogDebug("[Arcade Card] Port register read: Port = " + std::to_string(portNumber) + "  Reg = $" + HexUtilities::ToHex(reg));

	switch(reg) {
		case 0x00:
		case 0x01:
			return ReadPortValue(portNumber);

		case 0x02: return port.BaseAddress & 0xFF;
		case 0x03: return (port.BaseAddress >> 8) & 0xFF;
		case 0x04: return (port.BaseAddress >> 16) & 0xFF;
		case 0x05: return port.Offset & 0xFF;
		case 0x06: return (port.Offset >> 8) & 0xFF;
		case 0x07: return port.IncValue & 0xFF;
		case 0x08: return (port.IncValue >> 8) & 0xFF;
		case 0x09: return port.Control;
		case 0x0A: return 0;
		default:
			LogDebug("[Arcade Card] Unknown port register read: Port = " + std::to_string(portNumber) + "  Reg = $" + HexUtilities::ToHex(reg));
			return 0xFF;
	}
}

void PceArcadeCard::WritePortRegister(uint8_t portNumber, uint8_t reg, uint8_t value)
{
	PceArcadeCardPortConfig& port = _state.Port[portNumber];

	//LogDebug("[Arcade Card] Port register write: Port = " + std::to_string(portNumber) + "  Reg = $" + HexUtilities::ToHex(reg) + " = $" + HexUtilities::ToHex(value));

	switch(reg) {
		case 0x00:
		case 0x01:
			WritePortValue(portNumber, value);
			break;

		case 0x02: port.BaseAddress = (port.BaseAddress & 0xFFFF00) | value; break;
		case 0x03: port.BaseAddress = (port.BaseAddress & 0xFF00FF) | (value << 8); break;
		case 0x04: port.BaseAddress = (port.BaseAddress & 0x00FFFF) | (value << 16); break;

		case 0x05:
			port.Offset = (port.Offset & 0xFF00) | value;
			if(port.AddOffsetTrigger == PceArcadePortOffsetTrigger::AddOnLowWrite) {
				AddOffsetToBase(port);
			}
			break;

		case 0x06:
			port.Offset = (port.Offset & 0x00FF) | (value << 8);
			if(port.AddOffsetTrigger == PceArcadePortOffsetTrigger::AddOnHighWrite) {
				AddOffsetToBase(port);
			}
			break;

		case 0x07: port.IncValue = (port.IncValue & 0xFF00) | value; break;
		case 0x08: port.IncValue = (port.IncValue & 0x00FF) | (value << 8); break;

		case 0x09:
			port.Control = value & 0x7F; //bit 7 is unused?
			port.AutoIncrement = (value & 0x01) != 0;
			port.AddOffset = (value & 0x02) != 0;
			port.SignedIncrement = (value & 0x04) != 0; //not used?
			port.SignedOffset = (value & 0x08) != 0;
			port.AddIncrementToBase = (value & 0x10) != 0;
			port.AddOffsetTrigger = (PceArcadePortOffsetTrigger)((value & 0x60) >> 5);
			break;

		case 0x0A:
			if(port.AddOffsetTrigger == PceArcadePortOffsetTrigger::AddOnReg0AWrite) {
				AddOffsetToBase(port);
			}
			break;

		default:
			LogDebug("[Arcade Card] Unknown port register write: Port = " + std::to_string(portNumber) + "  Reg = $" + HexUtilities::ToHex(reg) + " = $" + HexUtilities::ToHex(value));
			break;

	}
}

uint8_t PceArcadeCard::Read(uint8_t bank, uint16_t addr, uint8_t value)
{
	if(bank == 0xFF) {
		addr &= 0x1FFF;

		if(addr >= 0x1A00 && addr <= 0x1BFF) {
			if(addr <= 0x1A7F) {
				return ReadPortRegister((addr & 0x30) >> 4, addr & 0x0F);
			} else {
				//LogDebug("[Arcade Card] Register read: $" + HexUtilities::ToHex(addr));

				switch(addr) {
					case 0x1AE0: return _state.ValueReg & 0xFF;
					case 0x1AE1: return (_state.ValueReg >> 8) & 0xFF;
					case 0x1AE2: return (_state.ValueReg >> 16) & 0xFF;
					case 0x1AE3: return (_state.ValueReg >> 24) & 0xFF;

					case 0x1AE4: return _state.ShiftReg;
					case 0x1AE5: return _state.RotateReg;

					//Arcade card version+signature
					case 0x1AFE: return 0x10;
					case 0x1AFF: return 0x51;

					default:
						LogDebug("[Arcade Card] Unknown register read: $" + HexUtilities::ToHex(addr));
						break;
				}
			}
		}
	} else if(bank >= 0x40 && bank <= 0x43) {
		return ReadPortValue(bank & 0x03);
	}

	return value;
}

void PceArcadeCard::Write(uint8_t bank, uint16_t addr, uint8_t value)
{
	if(bank == 0xFF) {
		addr &= 0x1FFF;

		if(addr >= 0x1A00 && addr <= 0x1BFF) {
			if(addr <= 0x1A7F) {
				WritePortRegister((addr & 0x30) >> 4, addr & 0x0F, value);
			} else {
				//LogDebug("[Arcade Card] Register write: $" + HexUtilities::ToHex(addr) + " = $" + HexUtilities::ToHex(value));

				switch(addr) {
					case 0x1AE0: _state.ValueReg = (_state.ValueReg & 0xFFFFFF00) | value; break;
					case 0x1AE1: _state.ValueReg = (_state.ValueReg & 0xFFFF00FF) | (value << 8); break;
					case 0x1AE2: _state.ValueReg = (_state.ValueReg & 0xFF00FFFF) | (value << 16); break;
					case 0x1AE3: _state.ValueReg = (_state.ValueReg & 0x00FFFFFF) | (value << 24); break;

					case 0x1AE4:
						_state.ShiftReg = value;
						if(value) {
							if(value & 0x08) {
								_state.ValueReg >>= (~value & 0x07) + 1;
							} else {
								_state.ValueReg <<= (value & 0x07);
							}
						}
						break;

					case 0x1AE5:
						//untested
						_state.RotateReg = value;
						if(value) {
							if(value & 0x08) {
								uint8_t rotateRight = (~value & 0x07) + 1;
								_state.ValueReg = (_state.ValueReg >> rotateRight) | (_state.ValueReg << (32 - rotateRight));
							} else {
								uint8_t rotateLeft = (value & 0x07);
								_state.ValueReg = (_state.ValueReg << rotateLeft) | (_state.ValueReg >> (32 - rotateLeft));
							}
						}
						break;

					default:
						LogDebug("[Arcade Card] Unknown register write: $" + HexUtilities::ToHex(addr) + " = $" + HexUtilities::ToHex(value));
						break;
				}
			}
		}
	} else if(bank >= 0x40 && bank <= 0x43) {
		WritePortValue(bank & 0x03, value);
	}
}

void PceArcadeCard::Serialize(Serializer& s)
{
	SV(_isRamUsed);
	if(_isRamUsed) {
		SVArray(_ram, PceArcadeCard::ArcadeRamMemSize);
	}

	for(int i = 0; i < 4; i++) {
		SVI(_state.Port[i].AddIncrementToBase);
		SVI(_state.Port[i].AddOffset);
		SVI(_state.Port[i].AddOffsetTrigger);
		SVI(_state.Port[i].AutoIncrement);
		SVI(_state.Port[i].BaseAddress);
		SVI(_state.Port[i].Control);
		SVI(_state.Port[i].IncValue);
		SVI(_state.Port[i].Offset);
		SVI(_state.Port[i].SignedIncrement);
		SVI(_state.Port[i].SignedOffset);
	}
	SV(_state.RotateReg);
	SV(_state.ShiftReg);
	SV(_state.ValueReg);
}
