#pragma once
#include "stdafx.h"
#include "NES/NesPpu.h"

class NsfPpu final : public NesPpu<NsfPpu>
{
public:
	NsfPpu(NesConsole* console) : NesPpu(console)
	{
	}

	void UpdateTimings(ConsoleRegion region, bool overclockAllowed)
	{
		NesPpu<NsfPpu>::UpdateTimings(region, false);
	}

	__forceinline void StoreSpriteAbsoluteAddress()
	{
	}

	__forceinline void StoreTileAbsoluteAddress()
	{
	}

	__forceinline void ProcessScanline()
	{
	}

	__forceinline void DrawPixel()
	{
	}

	void* OnBeforeSendFrame()
	{
		return nullptr;
	}

	void Run(uint64_t runTo) override
	{
		do {
			//Always need to run at least once, check condition at the end of the loop (slightly faster)
			if(_cycle < 340) {
				//Process cycles 1 to 340
				_cycle++;
			} else {
				//Process cycle 0
				ProcessScanlineFirstCycle();
			}
			_masterClock += _masterClockDivider;
		} while(_masterClock + _masterClockDivider <= runTo);
	}
};
