#include "pch.h"
#include "NES/Loaders/StudyBoxLoader.h"
#include "NES/RomData.h"
#include "NES/MapperFactory.h"
#include "Utilities/FolderUtilities.h"
#include "Utilities/HexUtilities.h"

uint32_t StudyBoxLoader::ReadInt(uint8_t*& data)
{
	uint32_t val = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
	data += 4;
	return val;
}

string StudyBoxLoader::ReadFourCC(uint8_t*& data)
{
	stringstream ss;
	for(int i = 0; i < 4; i++) {
		ss << (char)data[i];
	}
	data += 4;
	return ss.str();
}

vector<uint8_t> StudyBoxLoader::ReadArray(uint8_t*& data, uint32_t length)
{
	vector<uint8_t> out;
	out.resize(length);
	memcpy(out.data(), data, length);
	data += length;
	return out;
}

bool StudyBoxLoader::LoadStudyBoxTape(vector<uint8_t>& studyBoxFile, StudyBoxData& studyBoxData)
{
	uint8_t* data = studyBoxFile.data();
	uint8_t* end = data + studyBoxFile.size();

	if(end - data < 16) {
		//File too small to parse
		Log("[Study Box] File is too small to parse");
		return false;
	}

	if(ReadFourCC(data) != "STBX") {
		//Invalid file!
		Log("[Study Box] Invalid studybox file");
		return false;
	}

	uint32_t size = ReadInt(data);
	if(size != 4) {
		//Should be 4 bytes, with only a version field
		Log("[Study Box] Unexpected length value");
		return false;
	}

	uint32_t version = ReadInt(data);
	if(version != 0x100) {
		//Unsupported version
		Log("[Study Box] Unsupported version: " + std::to_string(version));
		return false;
	}

	uint32_t prevAudioOffset = 0;
	uint32_t prevLeadInOffset = 0;
	while(data < end - 4) {
		string cc = ReadFourCC(data);

		if(cc == "PAGE") {
			uint32_t pageSize = ReadInt(data);
			uint32_t leadInOffset = ReadInt(data);
			uint32_t audioOffset = ReadInt(data);

			if(audioOffset < leadInOffset) {
				//Invalid file, lead in must start before the track
				Log("[Study Box] Track lead in must start before the first bit of data");
				return false;
			}

			if(audioOffset < prevAudioOffset || leadInOffset < prevLeadInOffset) {
				//Invalid file, page chunks must be in the order found on the tape
				Log("[Study Box] PAGE chunks must be in the order found on the audio tape");
				return false;
			}

			prevAudioOffset = audioOffset;
			prevLeadInOffset = leadInOffset;

			if((end - data) >= pageSize - 8) {
				vector<uint8_t> pageData = ReadArray(data, pageSize - 8);
				studyBoxData.Pages.push_back({ leadInOffset, audioOffset, pageData });
			} else {
				//Invalid size value
				Log("[Study Box] Invalid size value for PAGE chunk");
				return false;
			}
		} else if(cc == "AUDI") {
			uint32_t audioSize = ReadInt(data);
			uint32_t fileType = ReadInt(data);
			if(fileType == 0) {
				if((end - data) >= audioSize - 4) {
					studyBoxData.AudioFile = ReadArray(data, audioSize - 4);
					//AUDI chunk should be the last in the file
					break;
				} else {
					Log("[Study Box] Invalid size value for AUDI chunk");
					return false;
				}
			} else {
				//Unsupported audio type
				Log("[Study Box] Unsupported audio type: " + std::to_string(fileType));
				return false;
			}
		} else {
			//Unsupported tag
			Log("[Study Box] Unsupported tag");
			return false;
		}
	}

	return studyBoxData.Pages.size() > 0;
}

void StudyBoxLoader::LoadRom(RomData& romData, vector<uint8_t>& romFile, string filepath)
{
	romData.Info.Hash.PrgCrc32 = romData.Info.Hash.Crc32;

	romData.Info.Format = RomFormat::StudyBox;
	romData.Info.MapperID = MapperFactory::StudyBoxMapperID;
	romData.Info.System = GameSystem::Famicom;

	romData.StudyBox.FileName = filepath;
	if(!LoadStudyBoxTape(romFile, romData.StudyBox)) {
		romData.Error = true;
	}
}
