#include "stdafx.h"

#include "CRC32.h"

void CRC32::AddData(const uint8_t* pData, const std::streamoff length)
{
	uint8_t* pCur = (uint8_t*)pData;
	for(std::streamoff remaining = length; remaining--; ++pCur) {
		_crc = (_crc >> 8) ^ kCrc32Table[(_crc ^ *pCur) & 0xff];
	}
}

uint32_t CRC32::GetCRC(uint8_t *buffer, std::streamoff length)
{
	CRC32 crc;
	crc.AddData(buffer, length);
	return ~(crc._crc);
}

uint32_t CRC32::GetCRC(string filename)
{
	uint32_t crc = 0;

	ifstream file(filename, std::ios::in | std::ios::binary);

	if(file) {
		file.seekg(0, std::ios::end);
		std::streamoff fileSize = file.tellg();
		file.seekg(0, std::ios::beg);
		uint8_t* buffer = new uint8_t[(uint32_t)fileSize];

		file.read((char*)buffer, fileSize);
		file.close();

		crc = GetCRC(buffer, fileSize);

		delete[] buffer;
	}
	return ~crc;
}