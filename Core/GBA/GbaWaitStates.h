#pragma once
#include "pch.h"
#include "GBA/GbaTypes.h"

class GbaWaitStates
{
private:
	uint8_t* _waitStatesLut = nullptr;

public:
	GbaWaitStates()
	{
		_waitStatesLut = new uint8_t[0x400];
	}

	~GbaWaitStates()
	{
		delete[] _waitStatesLut;
	}

	void GenerateWaitStateLut(GbaMemoryManagerState& state)
	{
		for(GbaAccessModeVal mode = 0; mode < 4; mode++) {
			for(int i = 0; i <= 0xFF; i++) {
				uint8_t waitStates = 1;
				switch(i) {
					case 0x02:
						//External work ram
						waitStates = (mode & GbaAccessMode::Word) ? 6 : 3;
						break;

					case 0x05:
					case 0x06:
						//VRAM/Palette
						waitStates = (mode & GbaAccessMode::Word) ? 2 : 1;
						break;

					case 0x08:
					case 0x09:
						waitStates = state.PrgWaitStates0[mode & GbaAccessMode::Sequential] + ((mode & GbaAccessMode::Word) ? state.PrgWaitStates0[1] : 0);
						break;

					case 0x0A:
					case 0x0B:
						waitStates = state.PrgWaitStates1[mode & GbaAccessMode::Sequential] + ((mode & GbaAccessMode::Word) ? state.PrgWaitStates1[1] : 0);
						break;

					case 0x0C:
					case 0x0D:
						waitStates = state.PrgWaitStates2[mode & GbaAccessMode::Sequential] + ((mode & GbaAccessMode::Word) ? state.PrgWaitStates2[1] : 0);
						break;

					case 0x0E:
					case 0x0F:
						//SRAM
						waitStates = state.SramWaitStates;
						break;
				}

				_waitStatesLut[(i << 2) | mode] = waitStates;
			}
		}
	}

	__forceinline uint8_t GetPrefetchWaitStates(GbaAccessModeVal mode, uint32_t addr)
	{
		return _waitStatesLut[((addr >> 22) & 0x3FC) | (mode & (GbaAccessMode::Word | GbaAccessMode::Sequential))];
	}

	__forceinline uint8_t GetWaitStates(GbaAccessModeVal mode, uint32_t addr)
	{
		return _waitStatesLut[((addr >> 22) & 0x3FC) | (mode & (GbaAccessMode::Word | ((addr & 0x1FFFF) ? GbaAccessMode::Sequential : 0)))];
	}
};