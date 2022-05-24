#pragma once
#include "stdafx.h"
#include "Utilities/VirtualFile.h"

enum class RomFormat
{
	Unknown,
	
	Sfc,
	SfcWithCopierHeader,
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
};

struct RomInfo
{
	VirtualFile RomFile;
	VirtualFile PatchFile;
	RomFormat Format = RomFormat::Unknown;
};
