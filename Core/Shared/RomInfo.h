#pragma once
#include "stdafx.h"
#include "Utilities/VirtualFile.h"

enum class RomFormat
{
	Unknown,
	
	Sfc,
	Spc,

	Gb,
	Gbs,

	iNes,
	Unif,
	Fds,
	VsSystem,
	VsDualSystem,
	Nsf,
	StudyBox,

	Pce,
	PceCdRom,
	PceHes,
};

struct DipSwitchInfo
{
	uint32_t DatabaseId = 0;
	uint32_t DipSwitchCount = 0;
};

struct RomInfo
{
	VirtualFile RomFile;
	VirtualFile PatchFile;
	RomFormat Format = RomFormat::Unknown;
	DipSwitchInfo DipSwitches = {};
};
