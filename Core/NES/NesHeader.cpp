#include "pch.h"
#include "NES/NesHeader.h"
#include "NES/RomData.h"
#include "Shared/MessageManager.h"

uint16_t NesHeader::GetMapperID()
{
	switch(GetRomHeaderVersion()) {
		case RomHeaderVersion::Nes2_0:
			return ((Byte8 & 0x0F) << 8) | (Byte7 & 0xF0) | (Byte6 >> 4);
		default:
		case RomHeaderVersion::iNes:
			return (Byte7 & 0xF0) | (Byte6 >> 4);
		case RomHeaderVersion::OldiNes:
			return (Byte6 >> 4);
	}
}

bool NesHeader::HasBattery()
{
	return (Byte6 & 0x02) == 0x02;
}

bool NesHeader::HasTrainer()
{
	return (Byte6 & 0x04) == 0x04;
}

GameSystem NesHeader::GetNesGameSystem()
{
	if(GetRomHeaderVersion() == RomHeaderVersion::Nes2_0) {
		switch(Byte12 & 0x03) {
			case 0: return GameSystem::NesNtsc;
			case 1: return GameSystem::NesPal;
			case 2: return GameSystem::NesNtsc; //Game works with both NTSC/PAL, pick NTSC by default
			case 3: return GameSystem::Dendy;
		}
	} else if(GetRomHeaderVersion() == RomHeaderVersion::iNes) {
		return (Byte9 & 0x01) ? GameSystem::NesPal : GameSystem::Unknown;
	}
	return GameSystem::Unknown;
}

GameSystem NesHeader::GetGameSystem()
{
	if(GetRomHeaderVersion() == RomHeaderVersion::Nes2_0) {
		switch(Byte7 & 0x03) {
			case 0: return GetNesGameSystem();
			case 1: return GameSystem::VsSystem;
			case 2: return GameSystem::Playchoice;
			case 3:
				switch(Byte13 & 0x0F) {
					case 0x0: return GetNesGameSystem();
					case 0x1: return GameSystem::VsSystem;
					case 0x2: return GameSystem::Playchoice;
					case 0x4: return GetNesGameSystem(); //EPSM
					case 0xC: return GameSystem::FamicomNetworkSystem;
					default:
						MessageManager::Log("[iNes] Unsupported console type detected (using NES NTSC instead)");
						return GameSystem::NesNtsc;

				}
				break;
		}
	} else if(GetRomHeaderVersion() == RomHeaderVersion::iNes) {
		if(Byte7 & 0x01) {
			return GameSystem::VsSystem;
		} else if(Byte7 & 0x02) {
			return GameSystem::Playchoice;
		}
	}
	return GetNesGameSystem();
}

bool NesHeader::HasEpsm()
{
	if(GetRomHeaderVersion() == RomHeaderVersion::Nes2_0) {
		return (Byte7 & 0x03) == 0x03 && (Byte13 & 0xF) == 0x04;
	}
	return false;
}

RomHeaderVersion NesHeader::GetRomHeaderVersion()
{
	if((Byte7 & 0x0C) == 0x08) {
		return RomHeaderVersion::Nes2_0;
	} else if((Byte7 & 0x0C) == 0x00) {
		return RomHeaderVersion::iNes;
	} else {
		return RomHeaderVersion::OldiNes;
	}
}

uint32_t NesHeader::GetSizeValue(uint32_t exponent, uint32_t multiplier)
{
	if(exponent > 60) {
		//Restrict max size to avoid overflow in a 64-bit value
		exponent = 60;
		MessageManager::Log("[iNes] Unsupported size value.");
	}

	multiplier = multiplier * 2 + 1;
	
	uint64_t size = multiplier * (uint64_t)1 << exponent;
	if(size >= ((uint64_t)1 << 32)) {
		MessageManager::Log("[iNes] Unsupported size value.");
	}
	return (uint32_t)size;
}

uint32_t NesHeader::GetPrgSize()
{
	if(GetRomHeaderVersion() == RomHeaderVersion::Nes2_0) {
		if((Byte9 & 0x0F) == 0x0F) {
			return (uint32_t)GetSizeValue(PrgCount >> 2, PrgCount & 0x03);
		} else {
			return (((Byte9 & 0x0F) << 8) | PrgCount) * 0x4000;
		}
	} else {
		if(PrgCount == 0) {
			return 256 * 0x4000; //0 is a special value and means 256
		} else {
			return PrgCount * 0x4000;
		}
	}
}

uint32_t NesHeader::GetChrSize()
{
	if(GetRomHeaderVersion() == RomHeaderVersion::Nes2_0) {
		if((Byte9 & 0xF0) == 0xF0) {
			return (uint32_t)GetSizeValue(ChrCount >> 2, ChrCount & 0x03);
		} else {
			return (((Byte9 & 0xF0) << 4) | ChrCount) * 0x2000;
		}
	} else {
		return ChrCount * 0x2000;
	}
}

int32_t NesHeader::GetWorkRamSize()
{
	if(GetRomHeaderVersion() == RomHeaderVersion::Nes2_0) {
		uint8_t value = Byte10 & 0x0F;
		return value == 0 ? 0 : 128 * (uint32_t)std::pow(2, value - 1);
	} else {
		return -1;
	}
}

int32_t NesHeader::GetSaveRamSize()
{
	if(GetRomHeaderVersion() == RomHeaderVersion::Nes2_0) {
		uint8_t value = (Byte10 & 0xF0) >> 4;
		return value == 0 ? 0 : 128 * (uint32_t)std::pow(2, value - 1);
	} else {
		return -1;
	}
}

int32_t NesHeader::GetChrRamSize()
{
	if(GetRomHeaderVersion() == RomHeaderVersion::Nes2_0) {
		uint8_t value = Byte11 & 0x0F;
		return value == 0 ? 0 : 128 * (uint32_t)std::pow(2, value - 1);
	} else {
		return -1;
	}
}

int32_t NesHeader::GetSaveChrRamSize()
{
	if(GetRomHeaderVersion() == RomHeaderVersion::Nes2_0) {
		uint8_t value = (Byte11 & 0xF0) >> 4;
		return value == 0 ? 0 : 128 * (uint32_t)std::pow(2, value - 1);
	} else {
		return -1;
	}
}

uint8_t NesHeader::GetSubMapper()
{
	if(GetRomHeaderVersion() == RomHeaderVersion::Nes2_0) {
		return (Byte8 & 0xF0) >> 4;
	} else {
		return 0;
	}
}

MirroringType NesHeader::GetMirroringType()
{
	if(Byte6 & 0x08) {
		return MirroringType::FourScreens;
	} else {
		return Byte6 & 0x01 ? MirroringType::Vertical : MirroringType::Horizontal;
	}
}

GameInputType NesHeader::GetInputType()
{
	if(GetRomHeaderVersion() == RomHeaderVersion::Nes2_0) {
		if(Byte15 < (uint8_t)GameInputType::LastEntry) {
			return (GameInputType)Byte15;
		}

		MessageManager::Log("[iNes] Unknown controller type.");
		return GameInputType::Unspecified;
	} else {
		return GameInputType::Unspecified;
	}
}

VsSystemType NesHeader::GetVsSystemType()
{
	if(GetRomHeaderVersion() == RomHeaderVersion::Nes2_0) {
		if((Byte13 >> 4) <= 0x06) {
			return (VsSystemType)(Byte13 >> 4);
		}
		MessageManager::Log("[iNes] Unknown VS System Type specified.");
	}
	return VsSystemType::Default;
}

PpuModel NesHeader::GetVsSystemPpuModel()
{
	if(GetRomHeaderVersion() == RomHeaderVersion::Nes2_0) {
		switch(Byte13 & 0x0F) {
			case 0: return PpuModel::Ppu2C03;
			case 1:
				MessageManager::Log("[iNes] Unsupported VS System Palette specified (2C03G).");
				return PpuModel::Ppu2C03;

			case 2: return PpuModel::Ppu2C04A;
			case 3: return PpuModel::Ppu2C04B;
			case 4: return PpuModel::Ppu2C04C;
			case 5: return PpuModel::Ppu2C04D;
			case 6: return PpuModel::Ppu2C03;
			case 7: return PpuModel::Ppu2C03;
			case 8: return PpuModel::Ppu2C05A;
			case 9: return PpuModel::Ppu2C05B;
			case 10: return PpuModel::Ppu2C05C;
			case 11: return PpuModel::Ppu2C05D;
			case 12: return PpuModel::Ppu2C05E;

			default:
				MessageManager::Log("[iNes] Unknown VS System Palette specified.");
				break;
		}
	}
	return PpuModel::Ppu2C03;
}
