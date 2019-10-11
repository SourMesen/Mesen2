#pragma once
#include "stdafx.h"

struct DmaChannelConfig
{
	bool DmaActive;

	bool InvertDirection;
	bool Decrement;
	bool FixedTransfer;
	bool HdmaIndirectAddressing;
	uint8_t TransferMode;

	uint16_t SrcAddress;
	uint8_t SrcBank;

	uint16_t TransferSize;
	uint8_t DestAddress;

	uint16_t HdmaTableAddress;
	uint8_t HdmaBank;
	uint8_t HdmaLineCounterAndRepeat;
	bool DoTransfer;
	bool HdmaFinished;

	bool UnusedFlag;
};