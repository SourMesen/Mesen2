#pragma once
#include "pch.h"

class SpcFileData
{
public:
	string SongTitle;
	string GameTitle;
	string Dumper;
	string Artist;
	string Comment;

	uint16_t PC;
	uint8_t A;
	uint8_t X;
	uint8_t Y;
	uint8_t PS;
	uint8_t SP;

	uint8_t CpuRegs[4];
	uint8_t ControlReg;
	uint8_t RamRegs[2];
	uint8_t TimerOutput[3];
	uint8_t TimerTarget[3];

	uint8_t DspRegSelect;
	uint8_t DspRegs[128];
	uint8_t SpcRam[0x10000];

	bool HasExtraRam = false;
	uint8_t SpcExtraRam[0x40] = {};

	uint32_t TrackLength;
	uint32_t FadeLength;

	SpcFileData(uint8_t* spcData, uint32_t size)
	{
		SongTitle = string(spcData + 0x2E, spcData + 0x2E + 0x20);
		GameTitle = string(spcData + 0x4E, spcData + 0x4E + 0x20);
		Dumper = string(spcData + 0x6E, spcData + 0x6E + 0x10);
		Artist = string(spcData + 0xB1, spcData + 0xB1 + 0x20);
		Comment = string(spcData + 0x7E, spcData + 0x7E + 0x20);

		string strTrackLength = string(spcData + 0xA9, spcData + 0xA9 + 0x03);
		string strFadeLength = string(spcData + 0xAC, spcData + 0xAC + 0x05);
		bool isStringValue = true;
		for(char c : strTrackLength) {
			if(c != 0 && (c < '0' || c > '9')) {
				isStringValue = false;
			}
		}

		TrackLength = 0;
		FadeLength = 0;

		if(isStringValue) {
			try {
				if(strTrackLength.size() && strTrackLength[0] != 0) {
					TrackLength = std::stoi(strTrackLength);
				}
				if(strFadeLength.size() && strFadeLength[0] != 0) {
					FadeLength = std::stoi(strFadeLength);
				}
			} catch(std::exception&) {
			}
		}

		memcpy(SpcRam, spcData + 0x100, 0x10000);
		if(size >= 0x10200) {
			//Some SPC files don't have this data (0x10180 bytes instead of 0x10200 bytes)
			memcpy(SpcExtraRam, spcData + 0x101C0, 0x40);
			HasExtraRam = true;
		}

		memcpy(DspRegs, spcData + 0x10100, 128);

		PC = spcData[0x25] | (spcData[0x26] << 8);
		A = spcData[0x27];
		X = spcData[0x28];
		Y = spcData[0x29];
		PS = spcData[0x2A];
		SP = spcData[0x2B];

		ControlReg = spcData[0x100 + 0xF1];
		DspRegSelect = spcData[0x100 + 0xF2];

		CpuRegs[0] = spcData[0x100 + 0xF4];
		CpuRegs[1] = spcData[0x100 + 0xF5];
		CpuRegs[2] = spcData[0x100 + 0xF6];
		CpuRegs[3] = spcData[0x100 + 0xF7];

		RamRegs[0] = spcData[0x100 + 0xF8];
		RamRegs[1] = spcData[0x100 + 0xF9];

		TimerTarget[0] = spcData[0x100 + 0xFA];
		TimerTarget[1] = spcData[0x100 + 0xFB];
		TimerTarget[2] = spcData[0x100 + 0xFC];

		TimerOutput[0] = spcData[0x100 + 0xFD];
		TimerOutput[1] = spcData[0x100 + 0xFE];
		TimerOutput[2] = spcData[0x100 + 0xFF];
	}
};