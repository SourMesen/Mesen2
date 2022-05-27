#pragma once
#include "stdafx.h"
#include "Core/Debugger/DebugTypes.h"
#include "Core/MemoryType.h"

class DebugUtilities
{
public:
	static constexpr MemoryType GetCpuMemoryType(CpuType type)
	{
		switch(type) {
			case CpuType::Snes: return MemoryType::SnesMemory;
			case CpuType::Spc: return MemoryType::SpcMemory;
			case CpuType::NecDsp: return MemoryType::NecDspMemory;
			case CpuType::Sa1: return MemoryType::Sa1Memory;
			case CpuType::Gsu: return MemoryType::GsuMemory;
			case CpuType::Cx4: return MemoryType::Cx4Memory;
			case CpuType::Gameboy: return MemoryType::GameboyMemory;
			case CpuType::Nes: return MemoryType::NesMemory;
			case CpuType::Pce: return MemoryType::PceMemory;
		}

		throw std::runtime_error("Invalid CPU type");
	}

	static constexpr int GetProgramCounterSize(CpuType type)
	{
		switch(type) {
			case CpuType::Snes: return 6;
			case CpuType::Spc: return 4;
			case CpuType::NecDsp: return 6;
			case CpuType::Sa1: return 6;
			case CpuType::Gsu: return 6;
			case CpuType::Cx4: return 6;
			case CpuType::Gameboy: return 4;
			case CpuType::Nes: return 4;
			case CpuType::Pce: return 4;
		}

		throw std::runtime_error("Invalid CPU type");
	}

	static constexpr CpuType ToCpuType(MemoryType type)
	{
		switch(type) {
			case MemoryType::SnesMemory:
			case MemoryType::SnesCgRam:
			case MemoryType::SnesPrgRom:
			case MemoryType::SnesSaveRam:
			case MemoryType::SnesSpriteRam:
			case MemoryType::SnesVideoRam:
			case MemoryType::SnesWorkRam:
			case MemoryType::BsxMemoryPack:
			case MemoryType::BsxPsRam:
			case MemoryType::Register:
				return CpuType::Snes;

			case MemoryType::SpcMemory:
			case MemoryType::SpcRam:
			case MemoryType::SpcRom:
				return CpuType::Spc;

			case MemoryType::GsuMemory:
			case MemoryType::GsuWorkRam:
				return CpuType::Gsu;

			case MemoryType::Sa1InternalRam:
			case MemoryType::Sa1Memory:
				return CpuType::Sa1;

			case MemoryType::NecDspMemory:
			case MemoryType::DspDataRam:
			case MemoryType::DspDataRom:
			case MemoryType::DspProgramRom:
				return CpuType::NecDsp;

			case MemoryType::Cx4DataRam:
			case MemoryType::Cx4Memory:
				return CpuType::Cx4;
				
			case MemoryType::GbPrgRom:
			case MemoryType::GbWorkRam:
			case MemoryType::GbCartRam:
			case MemoryType::GbHighRam:
			case MemoryType::GbBootRom:
			case MemoryType::GbVideoRam:
			case MemoryType::GbSpriteRam:
			case MemoryType::GameboyMemory:
				return CpuType::Gameboy;

			case MemoryType::NesChrRam:
			case MemoryType::NesChrRom:
			case MemoryType::NesInternalRam:
			case MemoryType::NesMemory:
			case MemoryType::NesNametableRam:
			case MemoryType::NesPaletteRam:
			case MemoryType::NesPpuMemory:
			case MemoryType::NesPrgRom:
			case MemoryType::NesSaveRam:
			case MemoryType::NesSpriteRam:
			case MemoryType::NesSecondarySpriteRam:
			case MemoryType::NesWorkRam:
				return CpuType::Nes;

			case MemoryType::PceMemory:
			case MemoryType::PcePrgRom:
			case MemoryType::PceWorkRam:
			case MemoryType::PceSaveRam:
			case MemoryType::PceCdromRam:
			case MemoryType::PceCardRam:
			case MemoryType::PceAdpcmRam:
			case MemoryType::PceArcadeCardRam:
			case MemoryType::PceVideoRam:
			case MemoryType::PceVideoRamVdc2:
			case MemoryType::PcePaletteRam:
			case MemoryType::PceSpriteRam:
			case MemoryType::PceSpriteRamVdc2:
				return CpuType::Pce;

			default:
				throw std::runtime_error("Invalid CPU type");
		}
	}

	static constexpr bool IsRelativeMemory(MemoryType memType)
	{
		return memType <= GetLastCpuMemoryType();
	}

	static constexpr MemoryType GetLastCpuMemoryType()
	{
		//TODO refactor to "IsRelativeMemory"?
		return MemoryType::PceMemory;
	}

	static constexpr bool IsPpuMemory(MemoryType memType)
	{
		switch(memType) {
			case MemoryType::SnesVideoRam:
			case MemoryType::SnesSpriteRam:
			case MemoryType::SnesCgRam:
			case MemoryType::GbVideoRam:
			case MemoryType::GbSpriteRam:
			
			case MemoryType::NesChrRam:
			case MemoryType::NesChrRom:
			case MemoryType::NesSpriteRam:
			case MemoryType::NesPaletteRam:
			case MemoryType::NesNametableRam:
			case MemoryType::NesSecondarySpriteRam:
			case MemoryType::NesPpuMemory:
				return true;

			case MemoryType::PceVideoRam:
			case MemoryType::PceVideoRamVdc2:
			case MemoryType::PcePaletteRam:
			case MemoryType::PceSpriteRam:
			case MemoryType::PceSpriteRamVdc2:
				return true;

			default: 
				return false;
		}
	}

	static constexpr bool IsVolatileRam(MemoryType memType)
	{
		switch(memType) {
			case MemoryType::SnesPrgRom:
			case MemoryType::GbPrgRom:
			case MemoryType::GbBootRom:
			case MemoryType::NesPrgRom:
			case MemoryType::NesChrRom:
			case MemoryType::PcePrgRom:

			case MemoryType::NesSaveRam:
			case MemoryType::GbCartRam:
			case MemoryType::SnesSaveRam:
			case MemoryType::PceSaveRam:
				return false;

			default:
				return true;
		}
	}

	static constexpr CpuType GetLastCpuType()
	{
		return CpuType::Pce;
	}
};