#pragma once
#include "stdafx.h"

struct PpuState
{
	uint16_t Cycle;
	uint16_t Scanline;
	uint32_t FrameCount;
};

struct LayerConfig
{
	uint16_t TilemapAddress;
	uint16_t ChrAddress;

	uint16_t HScroll;
	uint16_t VScroll;

	bool HorizontalMirroring;
	bool VerticalMirroring;

	bool LargeTiles;
};

struct WindowConfig
{
	bool ActiveLayers[6];
	bool InvertedLayers[6];
	uint8_t Left;
	uint8_t Right;

	template<uint8_t layerIndex>
	bool PixelNeedsMasking(int x)
	{
		if(InvertedLayers[layerIndex]) {
			if(Left > Right) {
				return true;
			} else {
				return x < Left || x > Right;
			}
		} else {
			if(Left > Right) {
				return false;
			} else {
				return x >= Left && x <= Right;
			}
		}
	}
};

enum class  WindowMaskLogic
{
	Or = 0,
	And = 1,
	Xor = 2,
	Xnor = 3
};