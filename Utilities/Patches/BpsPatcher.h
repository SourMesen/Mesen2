#pragma once

#include "pch.h"

class BpsPatcher
{
private:
	static int64_t ReadBase128Number(std::istream &file);

public:
	static bool PatchBuffer(std::istream &bpsFile, vector<uint8_t> &input, vector<uint8_t> &output);
	static bool PatchBuffer(string bpsFilepath, vector<uint8_t> &input, vector<uint8_t> &output);
};