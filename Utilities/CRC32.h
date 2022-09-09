#pragma once
#include "pch.h"

class CRC32
{
private:
	static uint32_t crc32_16bytes(const void* data, size_t length, uint32_t previousCrc32);

public:
	static uint32_t GetCRC(uint8_t* buffer, std::streamoff length);
	static uint32_t GetCRC(vector<uint8_t>& data);
	static uint32_t GetCRC(string filename);
};