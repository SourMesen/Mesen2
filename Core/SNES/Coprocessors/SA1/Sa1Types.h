#pragma once
#include "pch.h"
#include "SNES/SnesCpuTypes.h"

enum class Sa1MathOp
{
	Mul = 0,
	Div = 1,
	Sum = 2
};

enum class Sa1DmaSrcDevice
{
	PrgRom = 0,
	BwRam = 1,
	InternalRam = 2,
	Reserved = 3
};

enum class Sa1DmaDestDevice
{
	InternalRam = 0,
	BwRam = 1
};

struct Sa1State
{
	uint16_t Sa1ResetVector;
	uint16_t Sa1IrqVector;
	uint16_t Sa1NmiVector;

	bool Sa1IrqRequested;
	bool Sa1IrqEnabled; //Enables IRQs from S-CPU to SA-1 (SA-1 receives interrupt)

	bool Sa1NmiRequested;
	bool Sa1NmiEnabled; //Enables NMIs from S-CPU to SA-1 (SA-1 receives NMI)
	bool Sa1Wait;
	bool Sa1Reset;

	bool DmaIrqEnabled;
	bool TimerIrqEnabled;

	uint8_t Sa1MessageReceived; //Sent by the S-CPU
	uint8_t CpuMessageReceived; //Sent by the SA-1

	uint16_t CpuIrqVector; //For the S-CPU
	uint16_t CpuNmiVector; //For the S-CPU
	bool UseCpuIrqVector;
	bool UseCpuNmiVector;

	bool CpuIrqRequested;
	bool CpuIrqEnabled; //Enables IRQs from SA-1 to S-CPU

	bool CharConvIrqFlag;
	bool CharConvIrqEnabled;
	bool CharConvDmaActive;
	uint8_t CharConvBpp;
	uint8_t CharConvFormat;
	uint8_t CharConvWidth;
	uint8_t CharConvCounter;

	uint8_t CpuBwBank;
	bool CpuBwWriteEnabled;

	uint8_t Sa1BwBank;
	uint8_t Sa1BwMode;
	bool Sa1BwWriteEnabled;
	uint8_t BwWriteProtectedArea;
	bool BwRam2BppMode;

	uint8_t BitmapRegister1[8];
	uint8_t BitmapRegister2[8];

	uint8_t CpuIRamWriteProtect;
	uint8_t Sa1IRamWriteProtect;

	uint32_t DmaSrcAddr;
	uint32_t DmaDestAddr;
	uint16_t DmaSize;
	bool DmaEnabled;
	bool DmaPriority;
	bool DmaCharConv;
	bool DmaCharConvAuto;
	Sa1DmaDestDevice DmaDestDevice;
	Sa1DmaSrcDevice DmaSrcDevice;
	bool DmaRunning;
	bool DmaIrqFlag;

	bool HorizontalTimerEnabled;
	bool VerticalTimerEnabled;
	bool UseLinearTimer;

	uint16_t HTimer;
	uint16_t VTimer;
	uint32_t LinearTimerValue;

	Sa1MathOp MathOp;
	uint16_t MultiplicandDividend;
	uint16_t MultiplierDivisor;
	uint64_t MathOpResult;
	uint8_t MathOverflow;

	bool VarLenAutoInc;
	uint8_t VarLenBitCount;
	uint32_t VarLenAddress;
	uint8_t VarLenCurrentBit;

	uint8_t Banks[4];
};
