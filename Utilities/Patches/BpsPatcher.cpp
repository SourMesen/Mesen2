#include "pch.h"
#include <assert.h>
#include <cstring>
#include "BpsPatcher.h"
#include "CRC32.h"

int64_t BpsPatcher::ReadBase128Number(std::istream &file)
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

bool BpsPatcher::PatchBuffer(string bpsFilepath, vector<uint8_t> &input, vector<uint8_t> &output)
{
	ifstream bpsFile(bpsFilepath, std::ios::in | std::ios::binary);
	if(bpsFile) {
		return PatchBuffer(bpsFile, input, output);
	}
	return false;
}

bool BpsPatcher::PatchBuffer(std::istream &bpsFile, vector<uint8_t> &input, vector<uint8_t> &output)
{
	bpsFile.seekg(0, std::ios::end);
	size_t fileSize = (size_t)bpsFile.tellg();
	bpsFile.seekg(0, std::ios::beg);

	char header[4];
	bpsFile.read((char*)&header, 4);
	if(memcmp((char*)&header, "BPS1", 4) != 0) {
		//Invalid BPS file
		return false;
	}

	int64_t inputFileSize = ReadBase128Number(bpsFile);
	int64_t outputFileSize = ReadBase128Number(bpsFile);
	if(inputFileSize == -1 || outputFileSize == -1) {
		//Invalid file
		return false;
	}

	int64_t metadataSize = ReadBase128Number(bpsFile);
	bpsFile.seekg(metadataSize, std::ios::cur);

	output.resize((size_t)outputFileSize);

	uint32_t outputOffset = 0;
	uint32_t inputRelativeOffset = 0;
	uint32_t outputRelativeOffset = 0;
	while((size_t)bpsFile.tellg() < fileSize - 12) {
		int64_t data = ReadBase128Number(bpsFile);
		if(data == -1) {
			//Invalid file
			return false;
		}

		uint8_t command = data & 0x03;
		uint64_t length = (data >> 2) + 1;
		switch(command) {
			case 0:
				//SourceRead
				while(length--) {
					output[outputOffset] = input[outputOffset];
					outputOffset++;
				}
				break;

			case 1:
				//TargetRead
				while(length--) {
					uint8_t value = 0;
					bpsFile.read((char*)&value, 1);

					output[outputOffset++] = value;
				}
				break;

			case 2: {
				//SourceCopy
				int32_t data = (int32_t)ReadBase128Number(bpsFile);
				inputRelativeOffset += (data & 1 ? -1 : +1) * (data >> 1);
				while(length--) {
					output[outputOffset++] = input[inputRelativeOffset++];
				}
				break;
			}

			case 3: {
				//TargetCopy
				int32_t data = (int32_t)ReadBase128Number(bpsFile);
				outputRelativeOffset += (data & 1 ? -1 : +1) * (data >> 1);
				while(length--) {
					output[outputOffset++] = output[outputRelativeOffset++];
				}
				break;
			}
		}			
	}

	uint8_t inputChecksum[4];
	uint8_t outputChecksum[4];
	bpsFile.read((char*)inputChecksum, 4);
	bpsFile.read((char*)outputChecksum, 4);
	uint32_t patchInputCrc = inputChecksum[0] | (inputChecksum[1] << 8) | (inputChecksum[2] << 16) | (inputChecksum[3] << 24);
	uint32_t patchOutputCrc = outputChecksum[0] | (outputChecksum[1] << 8) | (outputChecksum[2] << 16) | (outputChecksum[3] << 24);
	uint32_t inputCrc = CRC32::GetCRC(input.data(), input.size());
	uint32_t outputCrc = CRC32::GetCRC(output.data(), output.size());

	if(patchInputCrc != inputCrc || patchOutputCrc != outputCrc) {
		return false;
	}
	return true;
}
