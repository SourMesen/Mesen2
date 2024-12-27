#include "pch.h"
#include "SNES/DSP/DspVoice.h"
#include "SNES/DSP/Dsp.h"
#include "SNES/DSP/DspTypes.h"
#include "SNES/DSP/DspInterpolation.h"
#include "SNES/Spc.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"

void DspVoice::Init(uint8_t voiceIndex, Spc* spc, Dsp* dsp, uint8_t* dspVoiceRegs, SnesConfig* cfg)
{
	_voiceIndex = voiceIndex;
	_voiceBit = 1 << voiceIndex;
	_spc = spc;
	_dsp = dsp;
	_regs = dspVoiceRegs;
	_cfg = cfg;
	_shared = &_dsp->GetState();
}

void DspVoice::DecodeBrrSample()
{
	uint8_t nextBrrData = _spc->DspReadRam(_brrAddress + _brrOffset + 1);

	//Sign-extend all four 4-bit samples encoded in these 2 bytes
	int16_t samples[4] = {
		(int16_t)((int16_t)((_shared->BrrData & 0xF0) << 8) >> 12),
		(int16_t)((int16_t)((_shared->BrrData & 0xF) << 12) >> 12),
		(int16_t)((int16_t)((nextBrrData & 0xF0) << 8) >> 12),
		(int16_t)((int16_t)((nextBrrData & 0xF) << 12) >> 12)
	};

	int shift = _shared->BrrHeader >> 4;
	int filter = _shared->BrrHeader & 0x0C;

	int32_t prev1 = _sampleBuffer[_bufferPos > 0 ? _bufferPos - 1 : 11] >> 1;
	int32_t prev2 = _sampleBuffer[_bufferPos > 1 ? _bufferPos - 2 : 10] >> 1;
	for(int i = 0; i < 4; i++) {
		//Values 0-12 work normally, 16-bit RD=(D<<shift)>>1
		int32_t s = ((int32_t)samples[i] << shift) >> 1;
		if(shift >= 0x0D) {
			//"Values 13-15 force RD to either 0x0000 or 0xF800 depending on the sign of the input D"
			s = s < 0 ? -0x800 : 0;
		}

		switch(filter) {
			//Filter 0 (Direct):       S(x) = RD
			case 0x00: break;

			//Filter 1 (15/16):        S(x) = RD + S(x-1) + ((-S(x-1))>>4)
			case 0x04: s += prev1 + (-prev1 >> 4); break;

			//Filter 2 (61/32-15/16):  S(x) = RD + (S(x-1)<<1) + ((-((S(x-1)<<1)+S(x-1)))>>5) - S(x-2) + (S(x-2)>>4)
			case 0x08: s += (prev1 << 1) + ((-((prev1 << 1) + prev1)) >> 5) - prev2 + (prev2 >> 4); break;

			//Filter 3 (115/64-13/16): S(x) = RD + (S(x-1)<<1) + ((-(S(x-1)+(S(x-1)<<2)+(S(x-1)<<3)))>>6) - S(x-2) + (((S(x-2)<<1) + S(x-2))>>4)
			case 0x0C: s += (prev1 << 1) + ((-(prev1 + (prev1 << 2) + (prev1 << 3))) >> 6) - prev2 + (((prev2 << 1) + prev2) >> 4); break;
		}

		//"The calculations above are preformed in some higher number of bits, clamped to
		//16 bits at the end and then clipped to 15 bits. This 15-bit value is the value
		//output and the value used as S(x-1) or S(x-2) as needed for future filter iterations."
		_sampleBuffer[_bufferPos + i] = Dsp::Clamp16(s) * 2;
		prev2 = prev1;
		prev1 = _sampleBuffer[_bufferPos + i] >> 1;
	}

	if(_bufferPos <= 4) {
		_bufferPos += 4;
	} else {
		_bufferPos = 0;
	}
}

void DspVoice::ProcessEnvelope()
{
	int32_t env = _envVolume;
	if(_envMode == EnvelopeMode::Release) {
		//"When the envelope is in the Release state, this overrides all settings
		//of these registers. In this case, the counter rate R = 31 (i.e. adjust
		//every sample), and the adjustment is E -= 8."
		env -= 8;
		_envVolume = env < 0 ? 0 : env;
	} else {
		int32_t rate = 0;
		uint8_t sustain;
		//"Load VxGAIN or VxADSR2 register depending on ADSR1.7."
		if(_shared->Adsr1 & 0x80) {
			//"The most complex method ("ADSR") is used when VxADSR1 bit 7 is 1."
			uint8_t adsr2 = ReadReg(DspVoiceRegs::Adsr2);
			sustain = adsr2;
			switch(_envMode) {
				case EnvelopeMode::Attack:
					//"Attack: If aaaa == %1111, R=31 and E+=1024. Otherwise, pretend VxGAIN = %110aaaa1."
					if((_shared->Adsr1 & 0x0F) == 0x0F) {
						rate = 31;
						env += 1024;
					} else {
						rate = ((_shared->Adsr1 & 0x0F) << 1) | 0x01;
						env += 32;
					}
					break;

				case EnvelopeMode::Decay:
					//"Pretend VxGAIN = %1011ddd0."
					env -= ((env - 1) >> 8) + 1;
					rate = ((_shared->Adsr1 >> 3) & 0x0E) | 0x10;
					break;

				case EnvelopeMode::Sustain:
					//"Pretend VxGAIN = %101rrrrr."
					env -= ((env - 1) >> 8) + 1;
					rate = adsr2 & 0x1F;
					break;
			}
		} else {
			uint8_t gain = ReadReg(DspVoiceRegs::Gain);
			sustain = gain;
			if(gain & 0x80) {
				//"The second method ("Gain", usually with one of the 4 names below) is
				//available when VxADSR1 bit 7 is clear, but VxGAIN bit 7 is set. In
				//this case, we have 4 options, chosen based on the 'm' bits."
				rate = gain & 0x1F;
				switch(gain & 0x60) {
					//00 = Linear Decrease. R=g, E-=32
					case 0x00: env -= 32; break;

					//01 = Exp Decrease.    R=g, E-=((E-1)>>8)+1
					case 0x20: env -= ((env - 1) >> 8) + 1; break;

					//10 = Linear Increase. R=g, E+=32
					case 0x40: env += 32; break;

					//11 = Bent Increase.   R=g, E+=(E<0x600)?32:8
					case 0x60: env += ((uint16_t)_prevCalculatedEnv < 0x600 ? 32 : 8); break;
				}
			} else {
				//"The simplest method of envelope control ("Direct Gain") is available
				//when VxADSR1 bit 7 and VxGAIN bit 7 are both clear.In this case, the
				//volume envelope is simply E=%GGGGGGG0000, and R does not matter."
				env = (uint16_t)gain << 4;
				rate = 31;
			}
		}

		//"If the mode is Decay and the Sustain Level is matched, change to the Sustain state."
		//"When the upper 3 bits of E equal the Sustain Level (see above), enter the Sustain state."
		//"Note: the "lll" bits are the Sustain Level only when bit 'e' is set. If 'e' is clear, the top 3 bits of VxGAIN are used instead.
		if(_envMode == EnvelopeMode::Decay && (env >> 8) == (sustain >> 5)) {
			_envMode = EnvelopeMode::Sustain;
		}

		//"Save the new value, *pre-clamp*, to determine the
		//increment for GAIN Bent Increase mode's next sample."
		_prevCalculatedEnv = env;

		//Clamp value
		if(env < 0 || env > 0x7FF) {
			//"In all cases, clip E to 0 or 0x7FF rather than wrapping."
			env = env < 0 ? 0 : 0x7FF;
			if(_envMode == EnvelopeMode::Attack) {
				//"If the mode is Attack and the new value is greater than 0x7FF, change to the Decay state.
				//CRITICAL NOTE: Negative values also trigger this."
				_envMode = EnvelopeMode::Decay;
			}
		}

		if(_dsp->CheckCounter(rate)) {
			//"If the counter specifies the envelope is to be updated, the
			//envelope is set to the new value, clamped to 11 bits."
			_envVolume = env;
		}
	}
}

void DspVoice::UpdateOutput(bool right)
{
	//"Load and apply VxVOL[L/R] register."
	int32_t voiceOut = ((int32_t)_shared->VoiceOutput * (int8_t)ReadReg((DspVoiceRegs)((int)DspVoiceRegs::VolLeft + (int)right))) >> 7;

	voiceOut = voiceOut * (int32_t)_cfg->ChannelVolumes[_voiceIndex] / 100;

	_shared->OutSamples[(int)right] = Dsp::Clamp16(_shared->OutSamples[(int)right] + voiceOut);

	if(_shared->EchoOn & _voiceBit) {
		_shared->EchoOut[(int)right] = Dsp::Clamp16(_shared->EchoOut[(int)right] + voiceOut);
	}
}

void DspVoice::Step1()
{
	//"Load VxSRCN register"
	_shared->SampleAddress = (_shared->DirSampleTableAddress * 0x100) + (_shared->SourceNumber * 4);
	_shared->SourceNumber = ReadReg(DspVoiceRegs::SourceNumber);
}

void DspVoice::Step2()
{
	//"Load the sample pointer (using previously loaded DIR and VxSRCN)"
	uint16_t addr = _shared->SampleAddress;
	if(_keyOnDelay == 0) {
		//"the second word points to the 'restart' point for when the BRR end block is reached."
		addr += 2;
	}

	_shared->BrrNextAddress = _spc->DspReadRam(addr) | (_spc->DspReadRam(addr + 1) << 8);

	//"Load VxADSR1 register."
	_shared->Adsr1 = ReadReg(DspVoiceRegs::Adsr1);

	//"Load VxPITCHL register."
	_shared->Pitch = ReadReg(DspVoiceRegs::PitchLow);
}

void DspVoice::Step3()
{
	Step3a();
	Step3b();
	Step3c();
}

void DspVoice::Step3a()
{
	//"This 14-bit number adjusts the pitch of the sounds output for this voice"
	//Ignore top 2 bits
	_shared->Pitch |= (ReadReg(DspVoiceRegs::PitchHigh) & 0x3F) << 8;
}

void DspVoice::Step3b()
{
	//"Load the BRR header byte (every time), and the first of the two BRR bytes that will be decoded."
	_shared->BrrHeader = _spc->DspReadRam(_brrAddress);
	_shared->BrrData = _spc->DspReadRam(_brrAddress + _brrOffset);
}

void DspVoice::Step3c()
{
	if(_shared->PitchModulationOn & _voiceBit) {
		//"When the bit is set, the VxPITCH value will be adjusted by the
		//output of the PREVIOUS voice (i.e x - 1). The exact formula seems to be :
		//P = VxPITCH + (((OutX[x - 1] >> 5) * VxPITCH) >> 10)"
		_shared->Pitch += ((_shared->VoiceOutput >> 5) * _shared->Pitch) >> 10;
	}

	if(_keyOnDelay) {
		if(_keyOnDelay == 5) {
			//Key was just enabled, prepare next BRR sample decoding
			_brrAddress = _shared->BrrNextAddress;
			_brrOffset = 1;
			_bufferPos = 0;
			_shared->BrrHeader = 0;
		}

		_envVolume = 0;
		_prevCalculatedEnv = 0;

		_keyOnDelay--;
		_interpolationPos = (_keyOnDelay & 0x03) ? 0x4000 : 0;

		_shared->Pitch = 0;
	}

	int32_t output = 0;
	switch(_cfg->InterpolationType) {
		case DspInterpolationType::Gauss: output = DspInterpolation::Gauss(_interpolationPos, _sampleBuffer, _bufferPos); break;
		case DspInterpolationType::Cubic: output = DspInterpolation::Cubic(_interpolationPos, _sampleBuffer, _bufferPos); break;
		case DspInterpolationType::Sinc: output = DspInterpolation::Sinc(_interpolationPos, _sampleBuffer, _bufferPos); break;
		case DspInterpolationType::None: output = _sampleBuffer[((_interpolationPos >> 12) + _bufferPos) % 12]; break;
	}

	//"If applicable, replace the current sample with the noise sample."
	if(_shared->NoiseOn & _voiceBit) {
		//"And the output noise sample at any point is N (after which is volume adjustment then the left - shift to 'restore' the low bit)"
		output = (int16_t)(_shared->NoiseLfsr * 2);
	}

	//"Apply the volume envelope. - This is the value used for modulating the next voice's pitch, if applicable."
	_shared->VoiceOutput = ((output * _envVolume) >> 11) & ~0x01;
	_envOut = (uint8_t)(_envVolume >> 4);

	//"Check FLG bit 7 (NOT previously loaded).
	//Check BRR header 'e' and 'l' bits to determine if the voice ends."
	if((_dsp->ReadReg(DspGlobalRegs::Flags) & 0x80) || (_shared->BrrHeader & 0x03) == 0x01) {
		//"When a BRR end-without-loop block is reached, the state is set to Release."
		_envMode = EnvelopeMode::Release;
		_envVolume = 0;
	}

	if(_shared->EveryOtherSample) {
		//"Handle KOFF and KON using previously loaded values. If KON, ENDX.x will be cleared in step S7."
		if(_shared->KeyOff & _voiceBit) {
			//When the voice is keyed off [...], the state is set to Release."
			_envMode = EnvelopeMode::Release;
		}

		if(_shared->KeyOn & _voiceBit) {
			_keyOnDelay = 5;

			//"When the voice is keyed on, the state is set to Attack."
			_envMode = EnvelopeMode::Attack;
		}
	}

	if(!_keyOnDelay) {
		//"Update the volume envelope, using previously loaded values."
		ProcessEnvelope();
	}
}

void DspVoice::Step4()
{
	_shared->Looped = 0;
	if(_interpolationPos >= 0x4000) {
		//"If a new group of BRR samples is required, load the second BRR byte and decode the group of 4 BRR samples."
		DecodeBrrSample();

		if(_brrOffset >= 7) {
			//"If necessary, adjust the BRR pointer to the next block"
			if(_shared->BrrHeader & 0x01) {
				//"Flag the loop address for loading next step S2and set ENDX.x in step S7."
				_brrAddress = _shared->BrrNextAddress;
				_shared->Looped = _voiceBit;
			} else {
				_brrAddress += 9;
			}
			_brrOffset = 1;
		} else {
			_brrOffset += 2;
		}
	}

	//"Increment interpolation sample position as specified by pitch values."
	_interpolationPos = (_interpolationPos & 0x3FFF) + _shared->Pitch;
	if(_interpolationPos > 0x7FFF) {
		_interpolationPos = 0x7FFF;
	}

	UpdateOutput(false);
}

void DspVoice::Step5()
{
	UpdateOutput(true);

	//"The new ENDX.x value is prepared, and can be overwritten. Reads will not see it yet."
	uint8_t voiceEnd = _dsp->ReadReg(DspGlobalRegs::VoiceEnd) | _shared->Looped;
	if(_keyOnDelay == 5) {
		//Clear bit for this voice when key on starts
		voiceEnd &= ~_voiceBit;
	}
	_shared->VoiceEndBuffer = voiceEnd;
}

void DspVoice::Step6()
{
	//"The new VxOUTX value is prepared, and can be overwritten. Reads will not see it yet."
	_shared->OutRegBuffer = (uint8_t)(_shared->VoiceOutput >> 8);
}

void DspVoice::Step7()
{
	//"The new ENDX.x value may now be read."
	_dsp->WriteGlobalReg(DspGlobalRegs::VoiceEnd, _shared->VoiceEndBuffer);

	//"The new VxENVX value is prepared, and can be overwritten. Reads will not see it yet."
	_shared->EnvRegBuffer = _envOut;
}

void DspVoice::Step8()
{
	//"The new VxOUTX value may now be read."
	_dsp->WriteVoiceReg(_voiceIndex, DspVoiceRegs::Out, _shared->OutRegBuffer);
}

void DspVoice::Step9()
{
	//"The new VxENVX value may now be read."
	_dsp->WriteVoiceReg(_voiceIndex, DspVoiceRegs::Envelope, _shared->EnvRegBuffer);
}

void DspVoice::Serialize(Serializer& s)
{
	SV(_envVolume);
	SV(_prevCalculatedEnv);
	SV(_interpolationPos);
	SV(_envMode);

	SV(_brrAddress);
	SV(_brrOffset);
	SV(_voiceBit);
	SV(_keyOnDelay);
	SV(_envOut);
	SV(_bufferPos);

	SVArray(_sampleBuffer, 12);
}
