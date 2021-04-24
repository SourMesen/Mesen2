#include "stdafx.h"
#include <algorithm>
#include "NES/Loaders/FdsLoader.h"
#include "NES/RomData.h"
#include "NES/MapperFactory.h"
#include "NES/GameDatabase.h"
#include "Shared/MessageManager.h"
#include "Shared/EmuSettings.h"
#include "Utilities/FolderUtilities.h"
#include "Utilities/CRC32.h"
#include "Utilities/sha1.h"

void FdsLoader::AddGaps(vector<uint8_t>& diskSide, uint8_t * readBuffer)
{
	//Start image with 28300 bits of gap
	diskSide.insert(diskSide.end(), 28300 / 8, 0);

	for(size_t j = 0; j < FdsDiskSideCapacity;) {
		uint8_t blockType = readBuffer[j];
		uint32_t blockLength = 1;
		switch(blockType) {
			case 1: blockLength = 56; break; //Disk header
			case 2: blockLength = 2; break; //File count
			case 3: blockLength = 16; break; //File header
			case 4: blockLength = 1 + readBuffer[j - 3] + readBuffer[j - 2] * 0x100; break;
			default: return; //End parsing when we encounter an invalid block type (This is what Nestopia apppears to do)
		}

		if(blockType == 0) {
			diskSide.push_back(blockType);
		} else {
			diskSide.push_back(0x80);
			diskSide.insert(diskSide.end(), &readBuffer[j], &readBuffer[j] + blockLength);

			//Fake CRC value
			diskSide.push_back(0x4D);
			diskSide.push_back(0x62);

			//Insert 976 bits of gap after a block
			diskSide.insert(diskSide.end(), 976 / 8, 0);
		}

		j += blockLength;
	}
}

vector<uint8_t> FdsLoader::LoadBios()
{
	//For FDS, the PRG ROM is the FDS BIOS (8k)
	vector<uint8_t> biosData;

	ifstream biosFile(FolderUtilities::CombinePath(FolderUtilities::GetFirmwareFolder(), "FdsBios.bin"), ios::in | ios::binary);
	if(biosFile) {
		return vector<uint8_t>(std::istreambuf_iterator<char>(biosFile), {});
	} else {
		biosFile.open(FolderUtilities::CombinePath(FolderUtilities::GetFirmwareFolder(), "disksys.rom"), ios::in | ios::binary);
		if(biosFile) {
			return vector<uint8_t>(std::istreambuf_iterator<char>(biosFile), {});
		}
	}
	return {};
}

vector<uint8_t> FdsLoader::RebuildFdsFile(vector<vector<uint8_t>> diskData, bool needHeader)
{
	vector<uint8_t> output;
	output.reserve(diskData.size() * FdsDiskSideCapacity + 16);

	if(needHeader) {
		uint8_t header[16] = { 'F', 'D', 'S', '\x1a', (uint8_t)diskData.size(), '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0' };
		output.insert(output.end(), header, header + sizeof(header));
	}

	for(vector<uint8_t> &diskSide : diskData) {
		bool inGap = true;
		size_t i = 0, len = diskSide.size();
		size_t gapNeeded = FdsDiskSideCapacity;
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
				output.insert(output.end(), &diskSide[i], &diskSide[i] + blockLength);
				gapNeeded -= blockLength;
				i += blockLength;
				i += 2; //Skip CRC after block

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
		numberOfSides = (uint8_t)(romFile.size() / 65500);
	}

	for(uint32_t i = 0; i < numberOfSides; i++) {
		diskData.push_back(vector<uint8_t>());
		vector<uint8_t> &fdsDiskImage = diskData.back();

		diskHeaders.push_back(vector<uint8_t>(romFile.data() + fileOffset + 1, romFile.data() + fileOffset + 57));

		AddGaps(fdsDiskImage, &romFile[fileOffset]);
		fileOffset += FdsDiskSideCapacity;

		//Ensure the image is 65500 bytes
		if(fdsDiskImage.size() < FdsDiskSideCapacity) {
			fdsDiskImage.resize(FdsDiskSideCapacity);
		}
	}
}

void FdsLoader::LoadRom(RomData& romData, vector<uint8_t>& romFile)
{
	romData.Info.Hash.PrgCrc32 = CRC32::GetCRC(romFile.data(), romFile.size());

	romData.Info.Format = RomFormat::Fds;
	romData.Info.MapperID = MapperFactory::FdsMapperID;
	romData.Info.Mirroring = MirroringType::Vertical;
	romData.PrgRom = LoadBios();
	romData.Info.System = GameSystem::FDS;

	if(romData.PrgRom.size() != 0x2000) {
		romData.Error = true;
		romData.BiosMissing = true;
	}
}
