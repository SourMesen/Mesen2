#include "pch.h"
#include "SNES/DSP/Dsp.h"
#include "SNES/Spc.h"
#include "SNES/SnesConsole.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Utilities/Serializer.h"

//Quoted comments are from anomie's DSP document (with modifications by jwdonal)

Dsp::Dsp(Emulator* emu, SnesConsole* console, Spc* spc)
{
	_emu = emu;
	_spc = spc;

	memset(_state.Regs, 0, 0x80);
	console->InitializeRam(_state.ExternalRegs, 0x80);
	_emu->RegisterMemory(MemoryType::SpcDspRegisters, _state.ExternalRegs, 0x80);

	for(int i = 0; i < 8; i++) {
		_voices[i].Init(i, spc, this, _state.Regs + (i * 0x10), &_emu->GetSettings()->GetSnesConfig());
	}
	_state.NewKeyOn = ReadReg(DspGlobalRegs::KeyOn);
	_state.DirSampleTableAddress = ReadReg(DspGlobalRegs::DirSampleTableAddress);
	_state.EchoRingBufferAddress = ReadReg(DspGlobalRegs::EchoRingBufferAddress);

	Reset();
}

void Dsp::LoadSpcFileRegs(uint8_t* regs)
{
	for(uint8_t i = 0; i < 0x80; i++) {
		Write(i, regs[i]);
	}

	_state.NewKeyOn = ReadReg(DspGlobalRegs::KeyOn);
	_state.DirSampleTableAddress = ReadReg(DspGlobalRegs::DirSampleTableAddress);
	_state.EchoRingBufferAddress = ReadReg(DspGlobalRegs::EchoRingBufferAddress);
}

void Dsp::Reset()
{
	//"FLG will always act as if set to 0xE0 after power on or reset, even if the value read back indicates otherwise"
	_state.Regs[(int)DspGlobalRegs::Flags] = 0xE0;

	_state.Counter = 0;
	_state.EchoHistoryPos = 0;
	_state.EchoOffset = 0;
	_state.EveryOtherSample = 1;

	//"The noise generator operation is as follows: On reset, N = 0x4000."
	_state.NoiseLfsr = 0x4000;

	_state.Step = 0;
}

bool Dsp::CheckCounter(int32_t rate)
{
	static uint16_t const rates[32] =
	{
		UINT16_MAX,
		2048, 1536,
		1280, 1024,  768,
		640,  512,  384,
		320,  256,  192,
		160,  128,   96,
		80,   64,   48,
		40,   32,   24,
		20,   16,   12,
		10,    8,    6,
		5,    4,    3,
		2,
		1
	};

	static uint16_t const offsets[32] =
	{
		1, 0, 1040,
		536, 0, 1040,
		536, 0, 1040,
		536, 0, 1040,
		536, 0, 1040,
		536, 0, 1040,
		536, 0, 1040,
		536, 0, 1040,
		536, 0, 1040,
		536, 0, 1040,
		0,
		0
	};

	return (((uint16_t)_state.Counter + offsets[rate]) % rates[rate]) == 0;
}

void Dsp::UpdateCounter()
{
	if(_state.Counter == 0) {
		_state.Counter = 0x77FF;
	} else {
		_state.Counter--;
	}
}

void Dsp::Write(uint8_t reg, uint8_t value)
{
	_state.Regs[reg] = value;
	_state.ExternalRegs[reg] = value;
	switch(reg & 0x0F) {
		case (int)DspVoiceRegs::Envelope: _state.EnvRegBuffer = value; break;
		case (int)DspVoiceRegs::Out: _state.OutRegBuffer = value; break;

		case 0x0C:
			switch(reg) {
				case (int)DspGlobalRegs::KeyOn: _state.NewKeyOn = value; break;

				case (int)DspGlobalRegs::VoiceEnd:
					_state.VoiceEndBuffer = 0;
					WriteGlobalReg(DspGlobalRegs::VoiceEnd, 0);
					break;
			}
			break;
	}
}

int32_t Dsp::CalculateFir(int index, int ch)
{
	/*
		"The FIR formula is:
		The value is clipped when mixing samples x-1 to x-7:
		FIR = (int16)(S(x-7) * FFC0 >> 6 // oldest sample
						+ S(x-6) * FFC1 >> 6
						+ S(x-5) * FFC2 >> 6
						+ S(x-4) * FFC3 >> 6
						+ S(x-3) * FFC4 >> 6
						+ S(x-2) * FFC5 >> 6
						+ S(x-1) * FFC6 >> 6);
		
		We have overflow detection when adding the most recent sample only:
		FIR = clamp16(FIR + S(x-0) * FFC7 >> 6); // newest sample
		
		Finally, mask of the LSbit to get the final 16-bit result:
		FIR = FIR & ~1;"
	*/
	return (
		_state.EchoHistory[(_state.EchoHistoryPos + index + 1) & 0x07][ch] *
		(int8_t)_state.Regs[(int)DspGlobalRegs::EchoFilterCoeff0 + (index << 4)]
	) >> 6;
}

void Dsp::EchoStep22()
{
	_state.EchoHistoryPos = (_state.EchoHistoryPos + 1) & 0x07;

	//"Apply ESA using the previously loaded value along with the previously
	//calculated echo offset to calculate new echo pointer."
	_state.EchoPointer = (_state.EchoRingBufferAddress << 8) + _state.EchoOffset;

	//"Load left channel sample from the echo buffer."
	int16_t s = _spc->DspReadRam(_state.EchoPointer) | (_spc->DspReadRam(_state.EchoPointer + 1) << 8);
	_state.EchoHistory[_state.EchoHistoryPos][0] = s >> 1;

	//"Load FFC0."
	_state.EchoIn[0] = CalculateFir(0, 0);
	_state.EchoIn[1] = CalculateFir(0, 1);
}

void Dsp::EchoStep23()
{
	//"Load right channel sample from the echo buffer."
	int16_t s = _spc->DspReadRam(_state.EchoPointer + 2) | (_spc->DspReadRam(_state.EchoPointer + 3) << 8);
	_state.EchoHistory[_state.EchoHistoryPos][1] = s >> 1;

	//"Load FFC1 and FFC2."
	_state.EchoIn[0] += CalculateFir(1, 0) + CalculateFir(2, 0);
	_state.EchoIn[1] += CalculateFir(1, 1) + CalculateFir(2, 1);
}

void Dsp::EchoStep24()
{
	//"Load FFC3, FFC4, and FFC5."
	_state.EchoIn[0] += CalculateFir(3, 0) + CalculateFir(4, 0) + CalculateFir(5, 0);
	_state.EchoIn[1] += CalculateFir(3, 1) + CalculateFir(4, 1) + CalculateFir(5, 1);
}

void Dsp::EchoStep25()
{
	//"Load FFC6 and FFC7."
	int32_t left = (int16_t)(_state.EchoIn[0] + CalculateFir(6, 0));
	int32_t right = (int16_t)(_state.EchoIn[1] + CalculateFir(6, 1));

	//"We have overflow detection when adding the most recent sample only :
	// FIR = clamp16(FIR + S(x - 0) * FFC7 >> 6); // newest sample
	// Finally, mask of the LSbit to get the final 16-bit result:
	// FIR = FIR & ~1;"
	_state.EchoIn[0] = Dsp::Clamp16(left + (int16_t)CalculateFir(7, 0)) & ~0x01;
	_state.EchoIn[1] = Dsp::Clamp16(right + (int16_t)CalculateFir(7, 1)) & ~0x01;
}

void Dsp::EchoStep26()
{
	//"Load and apply MVOLL. Load and apply EVOLL."
	_state.OutSamples[0] = Dsp::Clamp16(
		((_state.OutSamples[0] * (int8_t)ReadReg(DspGlobalRegs::MasterVolLeft)) >> 7) +
		((_state.EchoIn[0] * (int8_t)ReadReg(DspGlobalRegs::EchoVolLeft)) >> 7)
	);

	//"Load and apply EFB."
	int32_t leftEcho = _state.EchoOut[0] + (int16_t)((_state.EchoIn[0] * (int8_t)ReadReg(DspGlobalRegs::EchoFeedbackVol)) >> 7);
	int32_t rightEcho = _state.EchoOut[1] + (int16_t)((_state.EchoIn[1] * (int8_t)ReadReg(DspGlobalRegs::EchoFeedbackVol)) >> 7);
	_state.EchoOut[0] = Dsp::Clamp16(leftEcho) & ~0x01;
	_state.EchoOut[1] = Dsp::Clamp16(rightEcho) & ~0x01;
}

void Dsp::EchoStep27()
{
	//"Load and apply MVOLR. Load and apply EVOLR."
	_state.OutSamples[1] = Dsp::Clamp16(
		((_state.OutSamples[1] * (int8_t)ReadReg(DspGlobalRegs::MasterVolRight)) >> 7) +
		((_state.EchoIn[1] * (int8_t)ReadReg(DspGlobalRegs::EchoVolRight)) >> 7)
	);
}

void Dsp::EchoStep28()
{
	//"Load FLG bit 5 (ECENx) for application to the left channel."
	_state.EchoEnabled = (ReadReg(DspGlobalRegs::Flags) & 0x20) == 0;
}

void Dsp::EchoStep29()
{
	//"Load EDL - if the current echo offset is 0, apply EDL."
	if(_state.EchoOffset == 0) {
		//"The size of the buffer is simply D<<11 bytes (D<<9 16-bit stereo samples)"
		_state.EchoLength = (ReadReg(DspGlobalRegs::EchoDelay) & 0x0F) << 11;
	}

	//"Increment the echo offset, and set to 0 if it exceeds the buffer length."
	_state.EchoOffset += 4;
	if(_state.EchoOffset >= _state.EchoLength) {
		_state.EchoOffset = 0;
	}

	//"Write left channel sample to the echo buffer, if allowed by ECENx."
	if(_state.EchoEnabled) {
		_spc->DspWriteRam(_state.EchoPointer, _state.EchoOut[0]);
		_spc->DspWriteRam(_state.EchoPointer + 1, _state.EchoOut[0] >> 8);
	}
	_state.EchoOut[0] = 0;

	//"Load ESA for future use."
	_state.EchoRingBufferAddress = ReadReg(DspGlobalRegs::EchoRingBufferAddress);

	//"Load FLG bit 5 (ECENx) again for application to the right channel."
	_state.EchoEnabled = (ReadReg(DspGlobalRegs::Flags) & 0x20) == 0;
}

void Dsp::EchoStep30()
{
	//"Write right channel sample to the echo buffer, if allowed by ECENx."
	if(_state.EchoEnabled) {
		_spc->DspWriteRam(_state.EchoPointer + 2, _state.EchoOut[1]);
		_spc->DspWriteRam(_state.EchoPointer + 3, _state.EchoOut[1] >> 8);
	}
	_state.EchoOut[1] = 0;
}

void Dsp::Exec()
{
	uint8_t step = _state.Step;
	_state.Step = (_state.Step + 1) & 0x1F;

	switch(step) {
		case  0: _voices[0].Step5(); _voices[1].Step2(); break;
		case  1: _voices[0].Step6(); _voices[1].Step3(); break;
		case  2: _voices[0].Step7(); _voices[1].Step4(); _voices[3].Step1(); break;
		case  3: _voices[0].Step8(); _voices[1].Step5(); _voices[2].Step2(); break;
		case  4: _voices[0].Step9(); _voices[1].Step6(); _voices[2].Step3(); break;
		case  5: _voices[1].Step7(); _voices[2].Step4(); _voices[4].Step1(); break;
		case  6: _voices[1].Step8(); _voices[2].Step5(); _voices[3].Step2(); break;
		case  7: _voices[1].Step9(); _voices[2].Step6(); _voices[3].Step3(); break;
		case  8: _voices[2].Step7(); _voices[3].Step4(); _voices[5].Step1(); break;
		case  9: _voices[2].Step8(); _voices[3].Step5(); _voices[4].Step2(); break;
		case 10: _voices[2].Step9(); _voices[3].Step6(); _voices[4].Step3(); break;
		case 11: _voices[3].Step7(); _voices[4].Step4(); _voices[6].Step1(); break;
		case 12: _voices[3].Step8(); _voices[4].Step5(); _voices[5].Step2(); break;
		case 13: _voices[3].Step9(); _voices[4].Step6(); _voices[5].Step3(); break;
		case 14: _voices[4].Step7(); _voices[5].Step4(); _voices[7].Step1(); break;
		case 15: _voices[4].Step8(); _voices[5].Step5(); _voices[6].Step2(); break;
		case 16: _voices[4].Step9(); _voices[5].Step6(); _voices[6].Step3(); break;
		case 17: _voices[0].Step1(); _voices[5].Step7(); _voices[6].Step4(); break;
		case 18: _voices[5].Step8(); _voices[6].Step5(); _voices[7].Step2(); break;
		case 19: _voices[5].Step9(); _voices[6].Step6(); _voices[7].Step3(); break;
		case 20: _voices[1].Step1(); _voices[6].Step7(); _voices[7].Step4(); break;
		case 21: _voices[6].Step8(); _voices[7].Step5(); _voices[0].Step2(); break;
		case 22: _voices[0].Step3a(); _voices[6].Step9(); _voices[7].Step6(); EchoStep22(); break;
		case 23: _voices[7].Step7(); EchoStep23(); break;
		case 24: _voices[7].Step8(); EchoStep24(); break;
		case 25: _voices[0].Step3b(); _voices[7].Step9(); EchoStep25(); break;

		case 26:
			//"Output the left sample to the DAC."
			EchoStep26();
			break;

		case 27:
			//Pitch modulation is not supported on voice 0
			_state.PitchModulationOn = ReadReg(DspGlobalRegs::PitchModulationOn) & 0xFE;

			//"Output the right sample to the DAC."
			EchoStep27();

			if(ReadReg(DspGlobalRegs::Flags) & 0x40) {
				//Global mute/silence flag
				_dspOutput[_outSampleCount] = 0;
				_dspOutput[_outSampleCount + 1] = 0;
			} else {
				_dspOutput[_outSampleCount] = (int16_t)_state.OutSamples[0];
				_dspOutput[_outSampleCount + 1] = (int16_t)_state.OutSamples[1];
			}

			_state.OutSamples[0] = 0;
			_state.OutSamples[1] = 0;

			_outSampleCount += 2;
			break;

		case 28:
			_state.DirSampleTableAddress = ReadReg(DspGlobalRegs::DirSampleTableAddress);
			_state.NoiseOn = ReadReg(DspGlobalRegs::NoiseOn);
			_state.EchoOn = ReadReg(DspGlobalRegs::EchoOn);

			EchoStep28();
			break;

		case 29:
			_state.EveryOtherSample ^= 1;
			if(_state.EveryOtherSample) {
				//"Clear internal KON bits for any channels keyed on in the previous 2 samples."
				//"These two steps (KON and KOFF related) are performed every other sample."
				_state.NewKeyOn &= ~_state.KeyOn;
			}

			EchoStep29();
			break;

		case 30:
			if(_state.EveryOtherSample) {
				//"Load KOFF and internal KON."
				//"These two steps (KON and KOFF related) are performed every other sample."
				_state.KeyOn = _state.NewKeyOn;
				_state.KeyOff = ReadReg(DspGlobalRegs::KeyOff);
			}

			//"Update global counter."
			UpdateCounter();

			//"Load FLG bits 0-4 and update noise sample if necessary."
			if(CheckCounter(ReadReg(DspGlobalRegs::Flags) & 0x1F)) {
				//"Each update, N=(N>>1)|(((N<<14)^(N<<13))&0x4000)."
				int newBit = ((_state.NoiseLfsr << 14) ^ (_state.NoiseLfsr << 13)) & 0x4000;
				_state.NoiseLfsr = newBit ^ (_state.NoiseLfsr >> 1);
			}

			_voices[0].Step3c();

			EchoStep30();
			break;

		case 31: _voices[0].Step4(); _voices[2].Step1(); break;
	}
}

void Dsp::Serialize(Serializer& s)
{
	SVArray(_state.Regs, 128);
	SVArray(_state.ExternalRegs, 128);

	for(int i = 0; i < 8; i++) {
		SVI(_voices[i]);
	}

	SV(_state.NoiseLfsr);
	SV(_state.Counter);
	SV(_state.Step);
	SV(_state.OutRegBuffer);
	SV(_state.EnvRegBuffer);
	SV(_state.VoiceEndBuffer);

	SV(_state.VoiceOutput);
	SVArray(_state.OutSamples, 2);

	SV(_state.Pitch);
	SV(_state.SampleAddress);
	SV(_state.BrrNextAddress);
	SV(_state.DirSampleTableAddress);
	SV(_state.NoiseOn);
	SV(_state.PitchModulationOn);
	SV(_state.KeyOn);
	SV(_state.NewKeyOn);
	SV(_state.KeyOff);
	SV(_state.EveryOtherSample);
	SV(_state.SourceNumber);
	SV(_state.BrrHeader);
	SV(_state.BrrData);
	SV(_state.Looped);
	SV(_state.Adsr1);

	SVArray(_state.EchoIn, 2);
	SVArray(_state.EchoOut, 2);
	
	int16_t* echoHistory = &_state.EchoHistory[0][0];
	SVArray(echoHistory, 8*2);

	SV(_state.EchoPointer);
	SV(_state.EchoLength);
	SV(_state.EchoOffset);
	SV(_state.EchoHistoryPos);
	SV(_state.EchoRingBufferAddress);
	SV(_state.EchoOn);
	SV(_state.EchoEnabled);
}
