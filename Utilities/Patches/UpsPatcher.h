#pragma once

#include "pch.h"

class UpsPatcher
{
private:
	static int64_t ReadBase128Number(std::istream &file);

public:
	static bool PatchBuffer(std::istream &upsFile, vector<uint8_t> &input, vector<uint8_t> &output);
	static bool PatchBuffer(string upsFilepath, vector<uint8_t> &input, vector<uint8_t> &output);
};