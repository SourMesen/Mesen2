#pragma once
#include "pch.h"

class HexUtilities
{
private:
	const static vector<string> _hexCache;

public:
	static string ToHex(uint8_t value);
	static const char* ToHexChar(uint8_t value);
	static string ToHex(uint16_t value);
	static string ToHex(uint32_t value, bool fullSize = false);
	static string ToHex(int32_t value, bool fullSize = false);
	static string ToHex20(uint32_t value);
	static string ToHex24(int32_t value);
	static string ToHex32(uint32_t value);
	static string ToHex(uint64_t value);
	static string ToHex(vector<uint8_t> &data, char delimiter = 0);

	static int FromHex(string hex);
};