#pragma once
#include "pch.h"

struct AudioTrackInfo
{
	string GameTitle;
	string SongTitle;
	string Artist;
	string Comment;

	double Position;
	double Length;
	double FadeLength;

	uint32_t TrackNumber;
	uint32_t TrackCount;
};

enum class AudioPlayerAction
{
	NextTrack,
	PrevTrack,
	SelectTrack,
};

struct AudioPlayerActionParams
{
	AudioPlayerAction Action;
	uint32_t TrackNumber;
};