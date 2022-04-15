#pragma once
#include "stdafx.h"
#include "Shared/MessageManager.h"
#include "Utilities/VirtualFile.h"
#include "Utilities/StringUtilities.h"
#include "Utilities/FolderUtilities.h"

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
};

class CdReader
{
public:
	static bool LoadCue(string path, DiscInfo& disc);
};