#pragma once

enum class MemoryType
{
	SnesMemory,
	SpcMemory,
	Sa1Memory,
	NecDspMemory,
	GsuMemory,
	Cx4Memory,
	GameboyMemory,
	NesMemory,
	NesPpuMemory,

	SnesPrgRom,
	SnesWorkRam,
	SnesSaveRam,
	SnesVideoRam,
	SnesSpriteRam,
	SnesCgRam,
	SpcRam,
	SpcRom,
	DspProgramRom,
	DspDataRom,
	DspDataRam,
	Sa1InternalRam,
	GsuWorkRam,
	Cx4DataRam,
	BsxPsRam,
	BsxMemoryPack,

	GbPrgRom,
	GbWorkRam,
	GbCartRam,
	GbHighRam,
	GbBootRom,
	GbVideoRam,
	GbSpriteRam,

	NesPrgRom,
	NesInternalRam,
	NesWorkRam,
	NesSaveRam,
	NesNametableRam,
	NesSpriteRam,
	NesSecondarySpriteRam,
	NesPaletteRam,
	NesChrRam,
	NesChrRom,

	Register
};