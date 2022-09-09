#pragma once
#include "pch.h"
#include "SNES/DSP/DspTypes.h"
#include "Utilities/ISerializable.h"

class Spc;
class Dsp;
struct SnesConfig;

class DspVoice final : public ISerializable
{
private:
	Spc* _spc = nullptr;
	Dsp* _dsp = nullptr;
	uint8_t* _regs = nullptr;
	SnesConfig* _cfg = nullptr;
	DspState* _shared = nullptr;

	int32_t _envVolume = 0;
	int32_t _prevCalculatedEnv = 0;
	int32_t _interpolationPos = 0;
	EnvelopeMode _envMode = EnvelopeMode::Release;

	uint16_t _brrAddress = 0;
	uint16_t _brrOffset = 1;
	uint8_t _voiceIndex = 0;
	uint8_t _voiceBit = 0;
	uint8_t _keyOnDelay = 0;
	uint8_t _envOut = 0;
	uint8_t _bufferPos = 0;

	int16_t _sampleBuffer[12] = {};

	uint8_t ReadReg(DspVoiceRegs reg) { return _regs[(int)reg]; }
	void WriteReg(DspVoiceRegs reg, uint8_t value) { _regs[(int)reg] = value; }

	void DecodeBrrSample();
	void ProcessEnvelope();
	void UpdateOutput(bool right);

public:
	void Init(uint8_t voiceIndex, Spc* spc, Dsp* dsp, uint8_t* dspVoiceRegs, SnesConfig* cfg);

	void Step1();
	void Step2();
	void Step3();
	void Step3a();
	void Step3b();
	void Step3c();
	void Step4();
	void Step5();
	void Step6();
	void Step7();
	void Step8();
	void Step9();

	void Serialize(Serializer& s) override;
};
