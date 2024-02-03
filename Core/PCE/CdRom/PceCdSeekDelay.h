#include "pch.h"

//Adaptation of https://github.com/pce-devel/PCECD_seek/blob/master/seektime.c by Dave Shadoff (MIT)

class PceCdSeekDelay
{
private:
	struct SectorGroup
	{
		uint32_t SectorPerRevolution;
		uint32_t FirstSector;
		uint32_t LastSector;
		double RotationMs;
	};

	static constexpr uint32_t NUM_SECTOR_GROUPS = 14;

	static constexpr SectorGroup _sectorList[NUM_SECTOR_GROUPS] = {
		{ 10, 0,      12572,  133.47 },
		{ 11, 12573,  30244,  146.82 },
		{ 12, 30245,  49523,  160.17 },
		{ 13, 49524,  70408,  173.51 },
		{ 14, 70409,  92900,  186.86 },
		{ 15,	92901,  116998, 200.21 },
		{ 16, 116999, 142703, 213.56 },
		{ 17, 142704, 170014, 226.90 },
		{ 18, 170015, 198932, 240.25 },
		{ 19, 198933, 229456, 253.60 },
		{ 20, 229457, 261587, 266.95 },
		{ 21, 261588, 295324, 280.29 },
		{ 22, 295325, 330668, 293.64 },
		{ 23, 330669, 333012, 306.99 }
	};

	static uint32_t FindGroup(uint32_t lba)
	{
		for(uint32_t i = 0; i < NUM_SECTOR_GROUPS; i++) {
			if(lba >= _sectorList[i].FirstSector && lba <= _sectorList[i].LastSector) {
				return i;
			}
		}
		return 0;
	}

public:
	static uint32_t GetSeekTimeMs(uint32_t startLba, uint32_t targetLba)
	{
		uint32_t src = FindGroup(startLba);
		uint32_t dst = FindGroup(targetLba);

		uint32_t lbaGap = std::abs((int)targetLba - (int)startLba);
		double trackGap = 0;
		if(dst == src) {
			trackGap = (lbaGap / _sectorList[dst].SectorPerRevolution);
		} else if(dst > src) {
			trackGap = (_sectorList[src].LastSector - startLba) / _sectorList[src].SectorPerRevolution;
			trackGap += (targetLba - _sectorList[dst].FirstSector) / _sectorList[dst].SectorPerRevolution;
			trackGap += (1606.48 * (dst - src - 1));
		} else {
			trackGap = (startLba - _sectorList[src].FirstSector) / _sectorList[src].SectorPerRevolution;
			trackGap += (_sectorList[dst].LastSector - targetLba) / _sectorList[dst].SectorPerRevolution;
			trackGap += (1606.48 * (src - dst - 1));
		}

		if(lbaGap < 2) {
			return 9 * 1000 / 60;
		} else if(lbaGap < 5) {
			return (9 * 1000 / 60) + (_sectorList[dst].RotationMs / 2);
		} else if(trackGap <= 80) {
			return (18 * 1000 / 60) + (_sectorList[dst].RotationMs / 2);
		} else if(trackGap <= 160) {
			return (22 * 1000 / 60) + (_sectorList[dst].RotationMs / 2);
		} else if(trackGap <= 644) {
			return (22 * 1000 / 60) + (_sectorList[dst].RotationMs / 2) + ((trackGap - 161) * 16.66 / 80);
		} else {
			return (48 * 1000 / 60) + ((trackGap - 644) * 16.66 / 195);
		}
	}
};