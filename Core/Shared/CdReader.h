#pragma once
#include "stdafx.h"
#include "Utilities/VirtualFile.h"
#include "Shared/MessageManager.h"

enum class TrackFormat
{
	Audio,
	Mode1_2352
};

struct DiscPosition
{
	uint32_t Minutes;
	uint32_t Seconds;
	uint32_t Frames;

	uint32_t ToLba()
	{
		return ((Minutes * 60) + Seconds) * 75 + Frames;
	}

	static DiscPosition FromLba(uint32_t lba)
	{
		DiscPosition pos;
		pos.Minutes = lba / 75 / 60;
		pos.Seconds = lba / 75 % 60;
		pos.Frames = lba % 75;
		return pos;
	}
};

struct TrackInfo
{
	uint32_t Size;
	uint32_t SectorCount;

	bool HasLeadIn;
	DiscPosition LeadInPosition;
	DiscPosition StartPosition;
	DiscPosition EndPosition;

	TrackFormat Format;
	uint32_t FileIndex;
	uint32_t FileOffset;
	
	uint32_t FirstSector;
	uint32_t LastSector;
};

struct DiscInfo
{
	vector<VirtualFile> Files;
	vector<TrackInfo> Tracks;
	uint32_t DiscSize;
	uint32_t DiscSectorCount;
	DiscPosition EndPosition;

	int32_t GetTrack(uint32_t sector)
	{
		for(size_t i = 0; i < Tracks.size(); i++) {
			if(Tracks[i].LastSector > sector) {
				return (int32_t)i;
			}
		}
		return -1;
	}

	int32_t GetTrackFirstSector(int32_t track)
	{
		if(track < Tracks.size()) {
			return Tracks[track].FirstSector;
		}
		return -1;
	}

	int32_t GetTrackLastSector(int32_t track)
	{
		if(track < Tracks.size()) {
			return Tracks[track].LastSector;
		}
		return -1;
	}

	void ReadDataSector(uint32_t sector, deque<uint8_t>& outData)
	{
		int32_t track = GetTrack(sector);
		if(track < 0) {
			LogDebug("Invalid sector/track");
			for(int j = 0; j < 2048; j++) {
				outData.push_back(0);
			}
		} else {
			uint32_t fileIndex = Tracks[track].FileIndex;
			uint32_t byteOffset = Tracks[track].FileOffset + (sector - Tracks[track].FirstSector) * 2352;
			for(int j = 0; j < 2048; j++) {
				outData.push_back(Files[fileIndex].ReadByte(byteOffset + 16 + j));
			}
		}
	}

	int16_t ReadAudioSample(uint32_t sector, uint32_t sample, uint32_t byteOffset)
	{
		int32_t track = GetTrack(sector);
		if(track < 0) {
			LogDebug("Invalid sector/track");
			return 0;
		}

		uint32_t fileIndex = Tracks[track].FileIndex;
		uint32_t startByte = Tracks[track].FileOffset + (sector - Tracks[track].FirstSector) * 2352;
		return (int16_t)(Files[fileIndex].ReadByte(startByte + sample * 4 + byteOffset) | (Files[fileIndex].ReadByte(startByte + sample * 4 + 1 + byteOffset) << 8));
	}

	int16_t ReadLeftSample(uint32_t sector, uint32_t sample)
	{
		return ReadAudioSample(sector, sample, 0);
	}

	int16_t ReadRightSample(uint32_t sector, uint32_t sample)
	{
		return ReadAudioSample(sector, sample, 2);
	}
};

class CdReader
{
public:
	static bool LoadCue(VirtualFile& file, DiscInfo& disc);

	static uint8_t ToBcd(uint8_t value)
	{
		uint8_t div = value / 10;
		uint8_t rem = value % 10;
		return (div << 4) | rem;
	}

	static uint8_t FromBcd(uint8_t value)
	{
		return ((value >> 4) & 0x0F) * 10 + (value & 0x0F);
	}
};