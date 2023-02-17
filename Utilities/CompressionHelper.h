#pragma once
#include "pch.h"
#include "miniz.h"

class CompressionHelper
{
public:
	static void Compress(string data, int compressionLevel, vector<uint8_t>& output)
	{
		unsigned long compressedSize = compressBound((unsigned long)data.size());
		uint8_t* compressedData = new uint8_t[compressedSize];
		compress2(compressedData, &compressedSize, (unsigned char*)data.c_str(), (unsigned long)data.size(), compressionLevel);

		uint32_t size = (uint32_t)compressedSize;
		uint32_t originalSize = (uint32_t)data.size();
		output.insert(output.end(), (char*)&originalSize, (char*)&originalSize + sizeof(uint32_t));
		output.insert(output.end(), (char*)&size, (char*)&size + sizeof(uint32_t));
		output.insert(output.end(), (char*)compressedData, (char*)compressedData + compressedSize);
		delete[] compressedData;
	}

	static bool Decompress(vector<uint8_t>& input, vector<uint8_t>& output)
	{
		uint32_t decompressedSize;
		uint32_t compressedSize;

		memcpy(&decompressedSize, input.data(), sizeof(uint32_t));
		memcpy(&compressedSize, input.data() + sizeof(uint32_t), sizeof(uint32_t));

		if(decompressedSize >= 1024 * 1024 * 10 || compressedSize >= 1024 * 1024 * 10) {
			//Limit to 10mb the data's size
			return false;
		}

		output.resize(decompressedSize, 0);

		unsigned long decompSize = decompressedSize;
		if(uncompress(output.data(), &decompSize, input.data() + sizeof(uint32_t)*2, (unsigned long)input.size() - sizeof(uint32_t) * 2) != MZ_OK) {
			return false;
		}

		return true;
	}
};