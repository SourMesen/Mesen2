#pragma once
#include "stdafx.h"

class HexUtilities
{
private:
	const static vector<string> _hexCache;

public:
	static string ToHex(uint8_t value);
	static string ToHex(uint16_t value);
	static string ToHex(uint32_t value, bool fullSize = false);
	static string ToHex(int32_t value, bool fullSize = false);
	static string ToHex(vector<uint8_t> &data);

	static int FromHex(string hex);
};