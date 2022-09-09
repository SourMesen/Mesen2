#pragma once
#include "pch.h"
#include "NES/Loaders/BaseLoader.h"

struct StudyBoxData;
struct RomData;

class StudyBoxLoader : public BaseLoader
{
private:
	uint32_t ReadInt(uint8_t*& data);
	string ReadFourCC(uint8_t*& data);
	vector<uint8_t> ReadArray(uint8_t*& data, uint32_t length);

	bool LoadStudyBoxTape(vector<uint8_t>& studyBoxFile, StudyBoxData& studyBoxData);

public:
	using BaseLoader::BaseLoader;

	void LoadRom(RomData& romData, vector<uint8_t>& romFile, string filepath);
};