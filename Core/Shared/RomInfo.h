#pragma once
#include "pch.h"
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

	Sms,
	GameGear,
	Sg,
	ColecoVision,

	Gba,

	Ws,
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
