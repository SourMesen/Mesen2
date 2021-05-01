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
	Nsf,
	StudyBox
};

struct RomInfo
{
	VirtualFile RomFile;
	VirtualFile PatchFile;
	RomFormat Format;
};
