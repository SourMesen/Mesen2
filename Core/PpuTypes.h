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

	bool HorizontalMirrorring;
	bool VerticalMirrorring;

	bool LargeTiles;
};