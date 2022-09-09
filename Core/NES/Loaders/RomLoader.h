#pragma once
#include "pch.h"

class VirtualFile;
struct RomData;

class RomLoader
{
public:
	static bool LoadFile(VirtualFile &romFile, RomData& romData, bool databaseEnabled);
};
