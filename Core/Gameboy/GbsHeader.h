#pragma once
#include "pch.h"

struct GbsHeader
{
	char Header[3];
	uint8_t Version;
	uint8_t TrackCount;
	uint8_t FirstTrack;
	uint8_t LoadAddress[2];
	uint8_t InitAddress[2];
	uint8_t PlayAddress[2];
	uint8_t StackPointer[2];
	uint8_t TimerModulo;
	uint8_t TimerControl;
	char Title[32];
	char Author[32];
	char Copyright[32];
};