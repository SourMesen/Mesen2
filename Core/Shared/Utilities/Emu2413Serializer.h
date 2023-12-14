#include "pch.h"
#include "Shared/Utilities/emu2413.h"
#include "Utilities/Serializer.h"

class Emu2413Serializer
{
public:
	static void Serialize(OPLL* opll, Serializer& s)
	{
		SV(opll->clk);
		SV(opll->rate);

		SV(opll->chip_type);

		SV(opll->adr);

		SV(opll->inp_step);
		SV(opll->out_step);
		SV(opll->out_time);

		SVArray(opll->reg, 0x40);
		SV(opll->test_flag);
		SV(opll->slot_key_status);
		SV(opll->rhythm_mode);

		SV(opll->eg_counter);

		SV(opll->pm_phase);
		SV(opll->am_phase);

		SV(opll->lfo_am);

		SV(opll->noise);
		SV(opll->short_noise);

		SVArray(opll->patch_number, 9);

		for(int i = 0; i < 18; i++) {
			SVI(opll->slot[i].number);
			SVI(opll->slot[i].type);

			SVI(opll->slot[i].output[0]);
			SVI(opll->slot[i].output[1]);

			SVI(opll->slot[i].pg_phase);
			SVI(opll->slot[i].pg_out);
			SVI(opll->slot[i].pg_keep);
			SVI(opll->slot[i].blk_fnum);
			SVI(opll->slot[i].fnum);
			SVI(opll->slot[i].blk);

			SVI(opll->slot[i].eg_state);
			SVI(opll->slot[i].volume);
			SVI(opll->slot[i].key_flag);
			SVI(opll->slot[i].sus_flag);
			SVI(opll->slot[i].tll);
			SVI(opll->slot[i].rks);
			SVI(opll->slot[i].eg_rate_h);
			SVI(opll->slot[i].eg_rate_l);
			SVI(opll->slot[i].eg_shift);
			SVI(opll->slot[i].eg_out);

			SVI(opll->slot[i].update_requests);
		}

		SV(opll->mask);

		SVArray(opll->ch_out, 14);
		SVArray(opll->mix_out, 2);

		//custom patches
		for(int i = 0; i < 2; i++) {
			SVI(opll->patch[i].TL);
			SVI(opll->patch[i].FB);
			SVI(opll->patch[i].EG);
			SVI(opll->patch[i].ML);
			SVI(opll->patch[i].AR);
			SVI(opll->patch[i].DR);
			SVI(opll->patch[i].SL);
			SVI(opll->patch[i].RR);
			SVI(opll->patch[i].KR);
			SVI(opll->patch[i].KL);
			SVI(opll->patch[i].AM);
			SVI(opll->patch[i].PM);
			SVI(opll->patch[i].WS);
		}

		if(!s.IsSaving()) {
			OPLL_forceRefresh(opll);
		}
	}
};