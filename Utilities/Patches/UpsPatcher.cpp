#include "pch.h"
#include <assert.h>
#include <cstring>
#include "UpsPatcher.h"
#include "CRC32.h"

int64_t UpsPatcher::ReadBase128Number(std::istream &file)
{
	int64_t result = 0;
	int shift = 0;
	uint8_t buffer;
	while(true) {
		file.read((char*)&buffer, 1);
		if(file.eof()) {
			return -1;
		}
		result += (buffer & 0x7F) << shift;
		shift += 7;
		if(buffer & 0x80) {
			break;
		}
		result += (int64_t)1 << shift;
	}

	return result;
}

bool UpsPatcher::PatchBuffer(string upsFilepath, vector<uint8_t> &input, vector<uint8_t> &output)
{
	ifstream upsFile(upsFilepath, std::ios::in | std::ios::binary);
	if(upsFile) {
		return PatchBuffer(upsFile, input, output);
	}
	return false;
}

bool UpsPatcher::PatchBuffer(std::istream &upsFile, vector<uint8_t> &input, vector<uint8_t> &output)
{
	upsFile.seekg(0, std::ios::end);
	size_t fileSize = (size_t)upsFile.tellg();
	upsFile.seekg(0, std::ios::beg);

	char header[4];
	upsFile.read((char*)&header, 4);
	if(memcmp((char*)&header, "UPS1", 4) != 0) {
		//Invalid UPS file
		return false;
	}

	int64_t inputFileSize = ReadBase128Number(upsFile);
	int64_t outputFileSize = ReadBase128Number(upsFile);
	if(inputFileSize == -1 || outputFileSize == -1) {
		//Invalid file
		return false;
	}

	output.resize((size_t)outputFileSize);
	std::copy(input.begin(), input.end(), output.begin());

	uint32_t pos = 0;
	while((size_t)upsFile.tellg() < fileSize - 12) {
		int32_t offset = (int32_t)ReadBase128Number(upsFile);
		if(offset == -1) {
			//Invalid file
			return false;
		}

		pos += offset;

		while(true) {
			uint8_t xorValue = 0;
			upsFile.read((char*)&xorValue, 1);
			if((size_t)upsFile.tellg() > fileSize - 12) {
				//Invalid file
				return false;
			}

			output[pos] ^= xorValue;
			pos++;

			if(!xorValue) {
				break;
			}
		}
	}

	uint8_t inputChecksum[4];
	uint8_t outputChecksum[4];
	upsFile.read((char*)inputChecksum, 4);
	upsFile.read((char*)outputChecksum, 4);
	uint32_t patchInputCrc = inputChecksum[0] | (inputChecksum[1] << 8) | (inputChecksum[2] << 16) | (inputChecksum[3] << 24);
	uint32_t patchOutputCrc = outputChecksum[0] | (outputChecksum[1] << 8) | (outputChecksum[2] << 16) | (outputChecksum[3] << 24);
	uint32_t inputCrc = CRC32::GetCRC(input.data(), input.size());
	uint32_t outputCrc = CRC32::GetCRC(output.data(), output.size());

	if(patchInputCrc != inputCrc || patchOutputCrc != outputCrc) {
		return false;
	}
	return true;
}
