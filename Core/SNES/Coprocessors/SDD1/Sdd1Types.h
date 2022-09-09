#pragma once
#include "pch.h"

struct Sdd1State
{
	uint8_t AllowDmaProcessing;
	uint8_t ProcessNextDma;
	uint8_t SelectedBanks[4];

	uint32_t DmaAddress[8];
	uint16_t DmaLength[8];
	bool NeedInit;
};