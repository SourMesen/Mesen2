#pragma once

#include "stdafx.h"
#include "BaseVideoFilter.h"

class NesDefaultVideoFilter : public BaseVideoFilter
{
private:
	uint32_t _calculatedPalette[512];
	VideoConfig _videoConfig = {};
	NesConfig _nesConfig = {};

	void InitLookupTable();
	void GenerateFullColorPalette(uint32_t* paletteBuffer);

protected:
	void DecodePpuBuffer(uint16_t* ppuOutputBuffer, uint32_t* outputBuffer, bool displayScanlines);
	void OnBeforeApplyFilter();

public:
	NesDefaultVideoFilter(shared_ptr<Emulator> emulator);

	
	void ApplyFilter(uint16_t* ppuOutputBuffer);
};