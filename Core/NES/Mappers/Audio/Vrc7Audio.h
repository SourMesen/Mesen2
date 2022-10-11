#pragma once 
#include "pch.h"
#include "NES/APU/BaseExpansionAudio.h"
#include "NES/APU/NesApu.h"
#include "NES/NesConsole.h"
#include "NES/Mappers/Audio/emu2413.h"
#include "Utilities/Serializer.h"

class Vrc7Audio : public BaseExpansionAudio
{
private:
	static constexpr int OpllSampleRate = 49716;
	static constexpr int OpllClockRate = Vrc7Audio::OpllSampleRate * 72;

	OPLL* _opll = nullptr;
	uint8_t _currentReg = 0;
	int16_t _previousOutput = 0;
	double _clockTimer = 0;
	bool _muted = false;

protected:
	void ClockAudio() override
	{
		if(_clockTimer == 0) {
			_clockTimer = ((double)_console->GetMasterClockRate()) / Vrc7Audio::OpllSampleRate;
		}

		_clockTimer--;
		if(_clockTimer <= 0) {
			int16_t output = OPLL_calc(_opll);
			_console->GetApu()->AddExpansionAudioDelta(AudioChannel::VRC7, _muted ? 0 : (output - _previousOutput));
			_previousOutput = output;
			_clockTimer = ((double)_console->GetMasterClockRate()) / Vrc7Audio::OpllSampleRate;
		}
	}

	void Serialize(Serializer& s) override
	{
		BaseExpansionAudio::Serialize(s);

		SV(_currentReg); SV(_previousOutput); SV(_clockTimer); SV(_muted);

		SV(_opll->clk);
		SV(_opll->rate);

		SV(_opll->chip_type);

		SV(_opll->adr);

		SV(_opll->inp_step);
		SV(_opll->out_step);
		SV(_opll->out_time);

		SVArray(_opll->reg, 0x40);
		SV(_opll->test_flag);
		SV(_opll->slot_key_status);
		SV(_opll->rhythm_mode);

		SV(_opll->eg_counter);

		SV(_opll->pm_phase);
		SV(_opll->am_phase);

		SV(_opll->lfo_am);

		SV(_opll->noise);
		SV(_opll->short_noise);
		
		SVArray(_opll->patch_number, 9);
		
		for(int i = 0; i < 12; i++) {
			SVI(_opll->slot[i].number);
			SVI(_opll->slot[i].type);

			SVI(_opll->slot[i].output[0]);
			SVI(_opll->slot[i].output[1]);

			SVI(_opll->slot[i].pg_phase);
			SVI(_opll->slot[i].pg_out);
			SVI(_opll->slot[i].pg_keep);
			SVI(_opll->slot[i].blk_fnum);
			SVI(_opll->slot[i].fnum);
			SVI(_opll->slot[i].blk);

			SVI(_opll->slot[i].eg_state);
			SVI(_opll->slot[i].volume);
			SVI(_opll->slot[i].key_flag);
			SVI(_opll->slot[i].sus_flag);
			SVI(_opll->slot[i].tll);
			SVI(_opll->slot[i].rks);
			SVI(_opll->slot[i].eg_rate_h);
			SVI(_opll->slot[i].eg_rate_l);
			SVI(_opll->slot[i].eg_shift);
			SVI(_opll->slot[i].eg_out);

			SVI(_opll->slot[i].update_requests);
		}

		SV(_opll->mask);

		SVArray(_opll->ch_out, 14);
		SVArray(_opll->mix_out, 2);

		//custom patches
		for(int i = 0; i < 2; i++) {
			SVI(_opll->patch[i].TL);
			SVI(_opll->patch[i].FB);
			SVI(_opll->patch[i].EG);
			SVI(_opll->patch[i].ML);
			SVI(_opll->patch[i].AR);
			SVI(_opll->patch[i].DR);
			SVI(_opll->patch[i].SL);
			SVI(_opll->patch[i].RR);
			SVI(_opll->patch[i].KR);
			SVI(_opll->patch[i].KL);
			SVI(_opll->patch[i].AM);
			SVI(_opll->patch[i].PM);
			SVI(_opll->patch[i].WS);
		}

		if(!s.IsSaving()) {
			OPLL_forceRefresh(_opll);
		}
	}

public:
	Vrc7Audio(NesConsole* console) : BaseExpansionAudio(console)
	{
		_previousOutput = 0;
		_currentReg = 0;
		_muted = false;
		_clockTimer = 0;
		
		_opll = OPLL_new(Vrc7Audio::OpllClockRate, Vrc7Audio::OpllSampleRate);

		//Set OPLL emulator to VRC7 mode
		OPLL_setChipType(_opll, 1);
		OPLL_resetPatch(_opll, 1);
	}

	~Vrc7Audio()
	{
		OPLL_delete(_opll);
	}

	void Reset()
	{
		OPLL_reset(_opll);
	}

	void SetMuteAudio(bool muted)
	{
		_muted = muted;
	}

	void WriteReg(uint16_t addr, uint8_t value)
	{
		if(_muted) {
			//"Writes to $9010 and $9030 are disregarded while this bit is set."
			return;
		}

		switch(addr & 0xF030) {
			case 0x9010:
				_currentReg = value;
				break;
			case 0x9030:
				OPLL_writeReg(_opll, _currentReg, value);
				break;
		}
	}
};