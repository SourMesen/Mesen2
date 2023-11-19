#pragma once
#include "pch.h"
#include "Utilities/VirtualFile.h"
#include "Shared/MessageManager.h"

enum class TrackFormat
{
	Audio,
	Mode1_2352,
	Mode1_2048
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

	string ToString()
	{
		return GetValueString(Minutes) + ":" + GetValueString(Seconds) + ":" + GetValueString(Frames);
	}

	static DiscPosition FromLba(uint32_t lba)
	{
		DiscPosition pos;
		pos.Minutes = lba / 75 / 60;
		pos.Seconds = lba / 75 % 60;
		pos.Frames = lba % 75;
		return pos;
	}

private:
	string GetValueString(uint32_t val)
	{
		if(val < 10) {
			return "0" + std::to_string(val);
		} else {
			return std::to_string(val);
		}
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

	uint32_t GetSectorSize()
	{
		switch(Format) {
			default:
			case TrackFormat::Audio: return 2352;
			case TrackFormat::Mode1_2352: return 2352;
			case TrackFormat::Mode1_2048: return 2048;
		}
	}
};

struct DiscInfo
{
	static constexpr int SectorSize = 2352;

	vector<VirtualFile> Files;
	vector<TrackInfo> Tracks;
	uint32_t DiscSize;
	uint32_t DiscSectorCount;
	DiscPosition EndPosition;

	int32_t GetTrack(uint32_t sector)
	{
		for(size_t i = 0; i < Tracks.size(); i++) {
			if(sector >= Tracks[i].FirstSector && sector <= Tracks[i].LastSector) {
				return (int32_t)i;
			}
		}
		return -1;
	}

	int32_t GetTrackFirstSector(int32_t track)
	{
		if(track < Tracks.size()) {
			return Tracks[track].FirstSector;
		} else if(track > 0 && track == Tracks.size()) {
			//Tenshi no Uta 2 intro sets the end of the audio playback to track 0x35, but the last track is 0x34
			//The expected behavior is probably that audio should end at the of track 0x34
			//Without this code, the end gets set to sector 0, which immediately triggers an IRQ and restarts the
			//intro sequence early, making it impossible to start playing the game.
			return Tracks[track - 1].LastSector;
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

	template<typename T>
	void ReadDataSector(uint32_t sector, T& outData)
	{
		constexpr int Mode1_2352_SectorHeaderSize = 16;

		int32_t track = GetTrack(sector);
		if(track < 0) {
			//TODO support reading pregap when it's available
			LogDebug("Invalid sector/track (or inside pregap)");
			outData.insert(outData.end(), 2048, 0);
		} else {
			TrackInfo& trk = Tracks[track];
			uint32_t sectorSize = trk.GetSectorSize();
			uint32_t sectorHeaderSize = trk.Format == TrackFormat::Mode1_2352 ? Mode1_2352_SectorHeaderSize : 0;
			uint32_t byteOffset = trk.FileOffset + (sector - trk.FirstSector) * sectorSize;
			if(!Files[trk.FileIndex].ReadChunk(outData, byteOffset + sectorHeaderSize, 2048)) {
				LogDebug("Invalid read offsets");
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
		uint32_t startByte = Tracks[track].FileOffset + (sector - Tracks[track].FirstSector) * DiscInfo::SectorSize;
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