#pragma once
#include "pch.h"

struct DspState
{
	uint8_t ExternalRegs[128];
	uint8_t Regs[128];

	int32_t NoiseLfsr = 0x4000;
	uint16_t Counter = 0;
	uint8_t Step = 0;
	uint8_t OutRegBuffer = 0;
	uint8_t EnvRegBuffer = 0;
	uint8_t VoiceEndBuffer = 0;

	int32_t VoiceOutput = 0;
	int32_t OutSamples[2] = {};

	//Latched values
	int32_t Pitch = 0;
	uint16_t SampleAddress = 0;
	uint16_t BrrNextAddress = 0;
	uint8_t DirSampleTableAddress = 0;
	uint8_t NoiseOn = 0;
	uint8_t PitchModulationOn = 0;
	uint8_t KeyOn = 0;
	uint8_t NewKeyOn = 0;
	uint8_t KeyOff = 0;
	uint8_t EveryOtherSample = 1;
	uint8_t SourceNumber = 0;
	uint8_t BrrHeader = 0;
	uint8_t BrrData = 0;
	uint8_t Looped = 0;
	uint8_t Adsr1 = 0;

	//Echo values
	int32_t EchoIn[2] = {};
	int32_t EchoOut[2] = {};
	int16_t EchoHistory[8][2] = {};
	uint16_t EchoPointer = 0;
	uint16_t EchoLength = 0;
	uint16_t EchoOffset = 0;
	uint8_t EchoHistoryPos = 0;
	uint8_t EchoRingBufferAddress = 0;
	uint8_t EchoOn = 0;
	bool EchoEnabled = false;
};

enum class DspGlobalRegs
{
	MasterVolLeft = 0x0C, //MVOLL
	MasterVolRight = 0x1C, //MVOLR
	EchoVolLeft = 0x2C, //EVOLL
	EchoVolRight = 0x3C, //EVOLR
	KeyOn = 0x4C, //KON
	KeyOff = 0x5C, //KOFF
	Flags = 0x6C, //FLG
	VoiceEnd = 0x7C, //ENDX

	EchoFeedbackVol = 0x0D, //EFB
	PitchModulationOn = 0x2D, //PMON
	NoiseOn = 0x3D, //NON
	EchoOn = 0x4D, //EON

	DirSampleTableAddress = 0x5D, //DIR
	EchoRingBufferAddress = 0x6D, //ESA
	EchoDelay = 0x7D, //EDL

	EchoFilterCoeff0 = 0x0F,
	EchoFilterCoeff1 = 0x1F,
	EchoFilterCoeff2 = 0x2F,
	EchoFilterCoeff3 = 0x3F,
	EchoFilterCoeff4 = 0x4F,
	EchoFilterCoeff5 = 0x5F,
	EchoFilterCoeff6 = 0x6F,
	EchoFilterCoeff7 = 0x7F
};

enum class DspVoiceRegs
{
	VolLeft, //VOLL
	VolRight, //VOLR
	PitchLow, //PITCHL
	PitchHigh, //PITCHH
	SourceNumber, //SRCN
	Adsr1,
	Adsr2,
	Gain,
	Envelope, //ENVX
	Out //OUTX
};

enum class EnvelopeMode
{
	Release,
	Attack,
	Decay,
	Sustain
};