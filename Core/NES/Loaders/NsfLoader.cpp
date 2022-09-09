#include "pch.h"
#include "NES/Loaders/NsfLoader.h"
#include "NES/RomData.h"
#include "NES/MapperFactory.h"
#include "Shared/RomInfo.h"

void NsfLoader::Read(uint8_t *& data, uint8_t & dest)
{
	dest = data[0];
	data++;
}

void NsfLoader::Read(uint8_t *& data, uint16_t & dest)
{
	dest = data[0] | (data[1] << 8);
	data += 2;
}

void NsfLoader::Read(uint8_t *& data, char * dest, size_t len)
{
	memcpy(dest, data, len);
	data += len;
}

void NsfLoader::InitializeFromHeader(RomData &romData)
{
	NsfHeader &header = romData.Info.NsfInfo;

	romData.Info.Format = RomFormat::Nsf;
	romData.Info.MapperID = MapperFactory::NsfMapperID;

	if(header.LoadAddress < 0x6000 || header.TotalSongs == 0) {
		romData.Error = true;
	}

	if(header.Flags == 0x01) {
		romData.Info.System = GameSystem::NesPal;
	}

	if(header.PlaySpeedNtsc == 0) {
		header.PlaySpeedNtsc = 16639;
	}

	if(header.PlaySpeedPal == 0) {
		header.PlaySpeedPal = 19997;
	}

	if(header.StartingSong > header.TotalSongs || header.StartingSong == 0) {
		header.StartingSong = 1;
	}

	//Log window output
	Log("[NSF] Region: " + string(header.Flags == 0x00 ? "NTSC" : (header.Flags == 0x01 ? "PAL" : "NTSC & PAL")));
	if(header.PlaySpeedNtsc > 0) {
		Log("[NSF] Play speed (NTSC): " + std::to_string(1000000.0 / (double)header.PlaySpeedNtsc) + " Hz");
	}
	if(header.PlaySpeedPal > 0) {
		Log("[NSF] Play speed (PAL): " + std::to_string(1000000.0 / (double)header.PlaySpeedPal) + " Hz");
	}
	Log("[NSF] Title: " + string(header.SongName));
	Log("[NSF] Artist: " + string(header.ArtistName));
	Log("[NSF] Copyright: " + string(header.CopyrightHolder));
	Log("[NSF] Ripper: " + string(header.RipperName));

	stringstream ss;
	ss << "[NSF] Load Address: 0x" << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << header.LoadAddress;
	Log(ss.str());
	ss = stringstream();
	ss << "[NSF] Init Address: 0x" << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << header.InitAddress;
	Log(ss.str());
	ss = stringstream();
	ss << "[NSF] Play Address: 0x" << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << header.PlayAddress;
	Log(ss.str());

	vector<string> chips;
	if(header.SoundChips & 0x01) {
		chips.push_back("VRC6");
	}
	if(header.SoundChips & 0x02) {
		chips.push_back("VRC7");
	}
	if(header.SoundChips & 0x04) {
		chips.push_back("FDS");
	}
	if(header.SoundChips & 0x08) {
		chips.push_back("MMC5");
	}
	if(header.SoundChips & 0x10) {
		chips.push_back("Namco 163");
	}
	if(header.SoundChips & 0x20) {
		chips.push_back("Sunsoft 5B");
	}
	if(chips.empty()) {
		chips.push_back("<none>");
	}

	ss = stringstream();
	for(size_t i = 0; i < chips.size(); i++) {
		if(i > 0) {
			ss << ", ";
		}
		ss << chips[i];
	}

	Log("[NSF] Sound Chips: " + ss.str());
	Log("[NSF] ROM size: " + std::to_string(romData.PrgRom.size() / 1024) + " KB");
}

void NsfLoader::InitHeader(NsfHeader & header)
{
	for(int i = 0; i < 256; i++) {
		//Used by NSFE
		header.TrackLength[i] = -1;
		header.TrackFade[i] = -1;
	}
}

void NsfLoader::LoadRom(RomData& romData, vector<uint8_t>& romFile)
{
	NsfHeader &header = romData.Info.NsfInfo;

	InitHeader(header);

	uint8_t* data = romFile.data();
	Read(data, header.Header, 5);
	Read(data, header.Version);
	Read(data, header.TotalSongs);
	Read(data, header.StartingSong);
	Read(data, header.LoadAddress);
	Read(data, header.InitAddress);
	Read(data, header.PlayAddress);
	Read(data, header.SongName, 32);
	Read(data, header.ArtistName, 32);
	Read(data, header.CopyrightHolder, 32);
	Read(data, header.PlaySpeedNtsc);
	Read(data, (char*)header.BankSetup, 8);
	Read(data, header.PlaySpeedPal);
	Read(data, header.Flags);
	Read(data, header.SoundChips);
	Read(data, (char*)header.Padding, 4);

	//Ensure strings are null terminated
	header.SongName[31] = 0;
	header.ArtistName[31] = 0;
	header.CopyrightHolder[31] = 0;

	if(header.PlaySpeedNtsc == 16666 || header.PlaySpeedNtsc == 16667) {
		header.PlaySpeedNtsc = 16639;
	}
	if(header.PlaySpeedPal == 20000) {
		header.PlaySpeedPal = 19997;
	}

	//Pad start of file to make the first block start at a multiple of 4k
	romData.PrgRom.insert(romData.PrgRom.end(), header.LoadAddress % 4096, 0);

	romData.PrgRom.insert(romData.PrgRom.end(), data, data + romFile.size() - 0x80);

	//Pad out the last block to be a multiple of 4k
	if(romData.PrgRom.size() % 4096 != 0) {
		romData.PrgRom.insert(romData.PrgRom.end(), 4096 - (romData.PrgRom.size() % 4096), 0);
	}

	//NSF header size
	romData.Info.FilePrgOffset = 0x80;

	InitializeFromHeader(romData);
}
