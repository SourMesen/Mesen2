#pragma once
#include "pch.h"

struct DmaChannelConfig
{
	uint16_t SrcAddress;
	uint16_t TransferSize;
	uint16_t HdmaTableAddress;
	uint8_t SrcBank;
	uint8_t DestAddress;

	bool DmaActive;

	bool InvertDirection;
	bool Decrement;
	bool FixedTransfer;
	bool HdmaIndirectAddressing;
	uint8_t TransferMode;

	uint8_t HdmaBank;
	uint8_t HdmaLineCounterAndRepeat;
	bool DoTransfer;
	bool HdmaFinished;

	bool UnusedControlFlag;

	uint8_t UnusedRegister;
};

struct SnesDmaControllerState
{
	DmaChannelConfig Channel[8];
	uint8_t HdmaChannels;
};