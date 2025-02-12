#include "pch.h"
#include <algorithm>
#include "NES/Loaders/FdsLoader.h"
#include "NES/RomData.h"
#include "NES/MapperFactory.h"
#include "NES/GameDatabase.h"
#include "Shared/MessageManager.h"
#include "Shared/RomInfo.h"
#include "Shared/EmuSettings.h"
#include "Utilities/FolderUtilities.h"
#include "Utilities/CRC32.h"
#include "Utilities/sha1.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/StringUtilities.h"

FdsLoader::FdsLoader(bool useQdFormat) : BaseLoader()
{
	_useQdFormat = useQdFormat;
}

int FdsLoader::GetSideCapacity()
{
	return _useQdFormat ? QdDiskSideCapacity : FdsDiskSideCapacity;
}

void FdsLoader::AddGaps(vector<uint8_t>& diskSide, uint8_t* readBuffer, uint32_t bufferSize)
{
	//Start image with 28300 bits of gap
	diskSide.insert(diskSide.end(), 28300 / 8, 0);

	auto read = [&](uint32_t i) -> uint8_t {
		if(i >= 0 && i < bufferSize) {
			return readBuffer[i];
		}
		return 0;
	};

	for(int j = 0; j < GetSideCapacity();) {
		uint8_t blockType = read(j);
		uint32_t blockLength = 1;
		switch(blockType) {
			case 1: blockLength = 56; break; //Disk header
			case 2: blockLength = 2; break; //File count
			
			case 3:
				//File header
				//Log("ID: $" + HexUtilities::ToHex(read(j + 1)) + " - $" + HexUtilities::ToHex(read(j + 2)) + " - " + StringUtilities::GetString(readBuffer + j + 3, 8));
				//Log("Size: $" + HexUtilities::ToHex(read(j + 0x0D) | (read(j+0x0E) << 8)));
				//Log("Load address: $" + HexUtilities::ToHex(read(j + 0x0B) | (read(j + 0x0C) << 8)));
				blockLength = 16;
				break;

			case 4:
				if(_useQdFormat) {
					blockLength = 1 + (read(j - 5) | (read(j - 4) << 8));
				} else {
					blockLength = 1 + (read(j - 3) | (read(j - 2) << 8));
				}
				break;

			default: return; //End parsing when we encounter an invalid block type (This is what Nestopia apppears to do)
		}

		if(_useQdFormat) {
			//Use CRC values from QD file
			blockLength += 2;
		}

		if(j + blockLength >= bufferSize) {
			return;
		}

		diskSide.push_back(0x80);
		diskSide.insert(diskSide.end(), &readBuffer[j], &readBuffer[j] + blockLength);

		if(!_useQdFormat) {
			//Fake CRC value
			diskSide.push_back(0x4D);
			diskSide.push_back(0x62);
		}

		//Insert 976 bits of gap after a block
		diskSide.insert(diskSide.end(), 976 / 8, 0);

		j += blockLength;
	}
}

vector<uint8_t> FdsLoader::RebuildFdsFile(vector<vector<uint8_t>> diskData, bool needHeader)
{
	vector<uint8_t> output;
	output.reserve(diskData.size() * GetSideCapacity() + 16);

	if(needHeader) {
		uint8_t header[16] = { 'F', 'D', 'S', '\x1a', (uint8_t)diskData.size(), '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0' };
		output.insert(output.end(), header, header + sizeof(header));
	}

	for(vector<uint8_t> &diskSide : diskData) {
		bool inGap = true;
		size_t i = 0, len = diskSide.size();
		size_t gapNeeded = GetSideCapacity();
		uint32_t fileSize = 0;
		while(i < len) {
			if(inGap) {
				if(diskSide[i] == 0x80) {
					inGap = false;
				}
				i++;
			} else {
				uint32_t blockLength = 1;
				switch(diskSide[i]) {
					case 1: blockLength = 56; break; //Disk header
					case 2: blockLength = 2; break; //File count
					case 3: blockLength = 16; fileSize = diskSide[i + 13] + diskSide[i + 14] * 0x100;  break; //File header
					case 4: blockLength = 1 + fileSize; break;
				}

				if(_useQdFormat) {
					blockLength += 2;
				}

				output.insert(output.end(), &diskSide[i], &diskSide[i] + blockLength);
				gapNeeded -= blockLength;
				i += blockLength;

				if(!_useQdFormat) {
					i += 2; //Skip CRC after block
				}

				inGap = true;
			}
		}
		output.insert(output.end(), gapNeeded, 0);
	}

	return output;
}

void FdsLoader::LoadDiskData(vector<uint8_t>& romFile, vector<vector<uint8_t>>& diskData, vector<vector<uint8_t>>& diskHeaders)
{
	uint8_t numberOfSides = 0;
	size_t fileOffset = 0;
	bool hasHeader = memcmp(romFile.data(), "FDS\x1a", 4) == 0;
	if(hasHeader) {
		numberOfSides = romFile[4];
		fileOffset = 16;
	} else {
		numberOfSides = (uint8_t)(romFile.size() / GetSideCapacity());
	}

	for(uint32_t i = 0; i < numberOfSides; i++) {
		diskData.push_back(vector<uint8_t>());
		vector<uint8_t> &fdsDiskImage = diskData.back();

		diskHeaders.push_back(vector<uint8_t>(romFile.data() + fileOffset + 1, romFile.data() + fileOffset + 57));

		AddGaps(fdsDiskImage, &romFile[fileOffset], (uint32_t)(romFile.size() - fileOffset));
		fileOffset += GetSideCapacity();

		//Ensure the image is at least the expected size of a disk
		if(fdsDiskImage.size() < GetSideCapacity()) {
			fdsDiskImage.resize(GetSideCapacity());
		}
	}
}

void FdsLoader::LoadRom(RomData& romData, vector<uint8_t>& romFile)
{
	romData.Info.Hash.PrgCrc32 = CRC32::GetCRC(romFile.data(), romFile.size());

	romData.Info.Format = RomFormat::Fds;
	romData.Info.MapperID = MapperFactory::FdsMapperID;
	romData.Info.Mirroring = MirroringType::Vertical;
	romData.Info.System = GameSystem::FDS;
}
