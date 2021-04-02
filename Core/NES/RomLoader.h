#pragma once
#include "stdafx.h"
#include "RomData.h"
#include "BaseLoader.h"

class ArchiveReader;
class VirtualFile;

class RomLoader : public BaseLoader
{
private:
	static constexpr int MaxFilesToCheck = 100;

	RomData _romData;
	string _filename;

	static string FindMatchingRomInFile(string filePath, HashInfo hashInfo, int &iterationCount);
	
public:
	using BaseLoader::BaseLoader;
	
	bool LoadFile(VirtualFile &romFile);

	RomData GetRomData();
	static string FindMatchingRom(vector<string> romFiles, string romFilename, HashInfo hashInfo, bool useFastSearch);
};
