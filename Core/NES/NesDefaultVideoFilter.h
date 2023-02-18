#pragma once

#include "pch.h"
#include "Shared/Video/BaseVideoFilter.h"
#include "NES/NesTypes.h"

class NesConsole;

class NesDefaultVideoFilter : public BaseVideoFilter
{
private:
	uint32_t _calculatedPalette[512] = {};
	VideoConfig _videoConfig = {};
	NesConfig _nesConfig = {};
	PpuModel _ppuModel = PpuModel::Ppu2C02;

	void InitLookupTable();

protected:
	void DecodePpuBuffer(uint16_t* ppuOutputBuffer, uint32_t* outputBuffer);
	void OnBeforeApplyFilter() override;

public:
	NesDefaultVideoFilter(Emulator* emu);

	static void ApplyPalBorder(uint16_t* ppuOutputBuffer);

	static void GenerateFullColorPalette(uint32_t paletteBuffer[512]);
	static void GetFullPalette(uint32_t palette[512], NesConfig& nesCfg, PpuModel model);

	static uint32_t GetDefaultPixelBrightness(uint16_t colorIndex, PpuModel model);

	void ApplyFilter(uint16_t* ppuOutputBuffer) override;
};