#include "pch.h"
#include <assert.h>
#include <cstring>
#include <sstream>
#include "IpsPatcher.h"

class IpsRecord
{
public:
	uint32_t Address = 0;
	uint16_t Length = 0;
	vector<uint8_t> Replacement;

	//For RLE records (when length == 0)
	uint16_t RepeatCount = 0;
	uint8_t Value = 0;

	bool ReadRecord(std::istream &ipsFile)
	{
		uint8_t buffer[3];

		ipsFile.read((char*)buffer, 3);
		if(memcmp(buffer, "EOF", 3) == 0) {
			//EOF reached
			return false;
		} else {
			Address = buffer[2] | (buffer[1] << 8) | (buffer[0] << 16);

			ipsFile.read((char*)buffer, 2);
			Length = buffer[1] | (buffer[0] << 8);

			if(Length == 0) {
				//RLE record
				ipsFile.read((char*)buffer, 3);
				RepeatCount = buffer[1] | (buffer[0] << 8);
				Value = buffer[2];
			} else {
				Replacement.resize(Length);
				ipsFile.read((char*)Replacement.data(), Length);
			}
			return true;
		}
	}

	void WriteRecord(vector<uint8_t> &output)
	{
		output.push_back((Address >> 16) & 0xFF);
		output.push_back((Address >> 8) & 0xFF);
		output.push_back(Address & 0xFF);

		output.push_back((Length >> 8) & 0xFF);
		output.push_back(Length & 0xFF);

		if(Length == 0) {
			output.push_back((RepeatCount >> 8) & 0xFF);
			output.push_back(RepeatCount & 0xFF);
			output.push_back(Value);
		} else {
			output.insert(output.end(), Replacement.data(), Replacement.data() + Replacement.size());
		}
	}
};

bool IpsPatcher::PatchBuffer(string ipsFilepath, vector<uint8_t> &input, vector<uint8_t> &output)
{
	ifstream ipsFile(ipsFilepath, std::ios::in | std::ios::binary);
	if(ipsFile) {
		return PatchBuffer(ipsFile, input, output);
	}
	return false;
}

bool IpsPatcher::PatchBuffer(vector<uint8_t> &ipsData, vector<uint8_t> &input, vector<uint8_t> &output)
{
	std::stringstream ss;
	ss.write((char*)ipsData.data(), ipsData.size());
	return PatchBuffer(ss, input, output);
}

bool IpsPatcher::PatchBuffer(std::istream &ipsFile, vector<uint8_t> &input, vector<uint8_t> &output)
{
	char header[5];
	ipsFile.read((char*)&header, 5);
	if(memcmp((char*)&header, "PATCH", 5) != 0) {
		//Invalid ips file
		return false;
	}

	vector<IpsRecord> records;
	int32_t truncateOffset = -1;
	size_t maxOutputSize = input.size();
	while(!ipsFile.eof()) {
		IpsRecord record;
		if(record.ReadRecord(ipsFile)) {
			if(record.Address + record.Length + record.RepeatCount > maxOutputSize) {
				maxOutputSize = record.Address + record.Length + record.RepeatCount;
			}
			records.push_back(record);
		} else {
			//EOF, try to read truncate offset record if it exists
			uint8_t buffer[3];
			ipsFile.read((char*)buffer, 3);
			if(!ipsFile.eof()) {
				truncateOffset = buffer[2] | (buffer[1] << 8) | (buffer[0] << 16);
			}
			break;
		}
	}

	output.resize(maxOutputSize);
	std::copy(input.begin(), input.end(), output.begin());

	for(IpsRecord record : records) {
		if(record.Length == 0) {
			std::fill(&output[record.Address], &output[record.Address]+record.RepeatCount, record.Value);
		} else {
			std::copy(record.Replacement.begin(), record.Replacement.end(), output.begin()+record.Address);
		}
	}

	if(truncateOffset != -1 && (int32_t)output.size() > truncateOffset) {
		output.resize(truncateOffset);
	}

	return true;
}

vector<uint8_t> IpsPatcher::CreatePatch(vector<uint8_t> originalData, vector<uint8_t> newData)
{
	vector<uint8_t> patchFile;
	if(originalData.size() != newData.size()) {
		return patchFile;
	}

	uint8_t header[5] = { 'P', 'A', 'T', 'C', 'H' };
	patchFile.insert(patchFile.end(), header, header + sizeof(header));

	size_t i = 0, len = originalData.size();
	while(i < len) {
		while(i < len && originalData[i] == newData[i]) {
			i++;
		}
		if(i < len) {
			IpsRecord patchRecord;
			uint8_t rleByte = newData[i];
			uint8_t rleCount = 0;
			bool createRleRecord = false;
			patchRecord.Address = (uint32_t)i;
			patchRecord.Length = 0;
			while(i < len && patchRecord.Length < 65535 && originalData[i] != newData[i]) {
				if(newData[i] == rleByte) {
					rleCount++;
				} else if(createRleRecord) {
					break;
				} else {
					rleByte = newData[i];
					rleCount = 1;
				}

				patchRecord.Length++;
				i++;

				if((patchRecord.Length == rleCount && rleCount > 3) || rleCount > 13) {
					//Making a RLE entry would probably save space, so write the current entry and create a RLE entry after it
					if(patchRecord.Length == rleCount) {
						//Same character since the start of this entry, make the RLE entry now
						createRleRecord = true;
					} else {
						patchRecord.Length -= rleCount;
						i -= rleCount;
						break;
					}
				}
			}
			if(createRleRecord) {
				patchRecord.Length = 0;
				patchRecord.RepeatCount = rleCount;
				patchRecord.Value = rleByte;
			} else {
				patchRecord.Replacement = vector<uint8_t>(newData.data() + patchRecord.Address, newData.data() + patchRecord.Address + patchRecord.Length);
			}
			patchRecord.WriteRecord(patchFile);
		}
	}

	uint8_t endOfFile[3] = { 'E', 'O', 'F' };
	patchFile.insert(patchFile.end(), endOfFile, endOfFile + sizeof(endOfFile));

	return patchFile;
}