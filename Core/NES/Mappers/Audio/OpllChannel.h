//////////////////////////////////////////////////////////////////////////////////////////
// Adapted from Nintendulator's code, which itself is based on Mitsutaka Okazaki's code //
// https://www.qmtpro.com/~nes/nintendulator/
//////////////////////////////////////////////////////////////////////////////////////////

/* Modified for usage in VRC7 sound emulation

Copyright (C) Mitsutaka Okazaki 2004

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from
the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not
be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

*/

/***********************************************************************************

emu2413.c -- YM2413 emulator written by Mitsutaka Okazaki 2001

2003 01-24 : Modified by xodnizel to remove code not needed for the VRC7, among other things.

References:
fmopl.c	       -- 1999,2000 written by Tatsuyuki Satoh (MAME development).
fmopl.c(fixed) -- (C) 2002 Jarek Burczynski.
s_opl.c	       -- 2001 written by Mamiya (NEZplug development).
fmgen.cpp      -- 1999,2000 written by cisc.
fmpac.ill      -- 2000 created by NARUTO.
MSX-Datapack
YMU757 data sheet
YM2143 data sheet

**************************************************************************************/

#pragma once
#include "stdafx.h"
#include "OpllTables.h"
#include "Utilities/Serializer.h"

namespace Vrc7Opll 
{
	/* voice data */
	struct OpllPatch
	{
		uint32_t TL, FB, EG, ML, AR, DR, SL, RR, KR, KL, AM, PM, WF;
	};

	class OpllChannel : public ISerializable
	{
	private:
		/* Expand x which is s bits to d bits. */
		uint32_t ExpandBits(uint32_t x, uint32_t s, uint32_t d)
		{
			return x << (d-s);
		}

		/* Cut the lower b bit(s) off. */
		uint32_t GetHighBits(uint32_t c, uint32_t b)
		{
			return c >> b;
		}

		int32_t wave2_4pi(int32_t e)
		{
			return (e) >> (SLOT_AMP_BITS - PG_BITS - 1);
		}

		/* Convert Amp(0 to EG_HEIGHT) to Phase(0 to 8PI). */
		int32_t wave2_8pi(int32_t e)
		{
			return e >> (SLOT_AMP_BITS - PG_BITS - 2);
		}

		int32_t EG2DB(int32_t d)
		{
			return d*(int32_t)(EG_STEP / DB_STEP);
		}

		uint32_t SL2EG(int32_t d)
		{
			return d*(int32_t)(SL_STEP / EG_STEP);
		}

		uint32_t S2E(double x)
		{
			return (SL2EG((int32_t)(x / SL_STEP)) << (EG_DP_BITS - EG_BITS));
		}

		shared_ptr<OpllTables> _tables;
		OpllPatch patch;

		int32_t type;          /* 0 : modulator 1 : carrier */

									  /* OUTPUT */
		int32_t feedback;
		int32_t output[2];   /* Output value of slot */

									/* for Phase Generator (PG) */
		uint16_t *sintbl;    /* Wavetable */
		uint32_t phase;      /* Phase */
		uint32_t dphase;     /* Phase increment amount */
		uint32_t pgout;      /* output */

									/* for Envelope Generator (EG) */
		int32_t fnum;          /* F-Number */
		int32_t block;         /* Block */
		int32_t volume;        /* Current volume */
		int32_t sustine;       /* Sustine 1 = ON, 0 = OFF */
		uint32_t tll;	      /* Total Level + Key scale level*/
		uint32_t rks;        /* Key scale offset (Rks) */
		int32_t eg_mode;       /* Current state */
		uint32_t eg_phase;   /* Phase */
		uint32_t eg_dphase;  /* Phase increment amount */
		uint32_t egout;      /* output */

	protected:
		void Serialize(Serializer& s) override
		{
			s.Stream(type, feedback, output[0], output[1], phase, dphase, pgout, fnum, block, volume, sustine, tll, rks, eg_mode, eg_phase, eg_dphase, egout,
				patch.TL, patch.FB, patch.EG, patch.ML, patch.AR, patch.DR, patch.SL, patch.RR, patch.KR, patch.KL, patch.AM, patch.PM, patch.WF);

			if(!s.IsSaving()) {
				UpdateAll();
			}
		}

	public:
		OpllPatch* GetPatch()
		{
			return &patch;
		}

		int32_t GetType()
		{
			return type;
		}

		int32_t GetEgMode()
		{
			return eg_mode;
		}

		void SetTables(shared_ptr<OpllTables> tables)
		{
			_tables = tables;
		}

		void setSustine(int32_t sustine)
		{
			this->sustine = sustine;
		}

		void setVolume(int32_t volume)
		{
			this->volume = volume;
		}

		void setFnumber(int32_t fnum)
		{
			this->fnum = fnum;
		}

		void setBlock(int32_t block)
		{
			this->block = block;
		}

		void reset(int type)
		{
			this->type = type;
			sintbl = _tables->waveform[0];
			phase = 0;
			dphase = 0;
			output[0] = 0;
			output[1] = 0;
			feedback = 0;
			eg_mode = SETTLE;
			eg_phase = EG_DP_WIDTH;
			eg_dphase = 0;
			rks = 0;
			tll = 0;
			sustine = 0;
			fnum = 0;
			block = 0;
			volume = 0;
			pgout = 0;
			egout = 0;
		}

		/* Slot key on	*/
		void slotOn()
		{
			eg_mode = ATTACK;
			eg_phase = 0;
			phase = 0;
		}

		/* Slot key on without reseting the phase */
		void slotOn2()
		{
			eg_mode = ATTACK;
			eg_phase = 0;
		}

		/* Slot key off */
		void slotOff()
		{
			if(eg_mode == ATTACK) {
				eg_phase = ExpandBits(_tables->AR_ADJUST_TABLE[GetHighBits(eg_phase, EG_DP_BITS - EG_BITS)], EG_BITS, EG_DP_BITS);
			}
			eg_mode = RELEASE;
		}

		void setSlotVolume(int32_t volume)
		{
			this->volume = volume;
		}

		void calc_phase(int32_t lfo)
		{
			if(patch.PM) {
				phase += (dphase * lfo) >> PM_AMP_BITS;
			} else {
				phase += dphase;
			}

			phase &= (DP_WIDTH - 1);

			pgout = GetHighBits(phase, DP_BASE_BITS);
		}

		int32_t calc_slot_car(int32_t fm)
		{
			output[1] = output[0];

			if(egout >= (DB_MUTE - 1)) {
				output[0] = 0;
			} else {
				output[0] = _tables->DB2LIN_TABLE[sintbl[(pgout + wave2_8pi(fm))&(PG_WIDTH - 1)] + egout];
			}

			return (output[1] + output[0]) >> 1;
		}

		int32_t calc_slot_mod()
		{
			int32_t fm;

			output[1] = output[0];

			if(egout >= (DB_MUTE - 1)) {
				output[0] = 0;
			} else if(patch.FB != 0) {
				fm = wave2_4pi(feedback) >> (7 - patch.FB);
				output[0] = _tables->DB2LIN_TABLE[sintbl[(pgout + fm)&(PG_WIDTH - 1)] + egout];
			} else {
				output[0] = _tables->DB2LIN_TABLE[sintbl[pgout] + egout];
			}

			feedback = (output[1] + output[0]) >> 1;

			return feedback;
		}

		void UpdatePg()
		{
			dphase = _tables->dphaseTable[fnum][block][patch.ML];
		}

		void UpdateTll()
		{
			if(type == 0) {
				tll = _tables->tllTable[(fnum) >> 5][block][patch.TL][patch.KL];
			} else {
				tll = _tables->tllTable[(fnum) >> 5][block][volume][patch.KL];
			}
		}

		void UpdateRks()
		{
			rks = _tables->rksTable[(fnum) >> 8][block][patch.KR];
		}

		void UpdateWf()
		{
			sintbl = _tables->waveform[patch.WF];
		}

		void UpdateEg()
		{
			eg_dphase = calc_eg_dphase();
		}

		void UpdateAll()
		{
			UpdatePg();
			UpdateTll();
			UpdateRks();
			UpdateWf();
			UpdateEg();		/* EG should be updated last. */
		}
		
		uint32_t calc_eg_dphase()
		{
			switch(eg_mode) {
				case ATTACK:
					return _tables->dphaseARTable[patch.AR][rks];

				case DECAY:
					return _tables->dphaseDRTable[patch.DR][rks];

				case SUSHOLD:
					return 0;

				case SUSTINE:
					return _tables->dphaseDRTable[patch.RR][rks];

				case RELEASE:
					if(sustine) {
						return _tables->dphaseDRTable[5][rks];
					} else if(patch.EG) {
						return _tables->dphaseDRTable[patch.RR][rks];
					} else {
						return _tables->dphaseDRTable[7][rks];
					}

				case FINISH:
					return 0;

				default:
					return 0;
			}
		}

		void calc_envelope(int32_t lfo)
		{
			static uint32_t SL[16] = {
				S2E(0.0), S2E(3.0), S2E(6.0), S2E(9.0), S2E(12.0), S2E(15.0), S2E(18.0), S2E(21.0),
				S2E(24.0), S2E(27.0), S2E(30.0), S2E(33.0), S2E(36.0), S2E(39.0), S2E(42.0), S2E(48.0)
			};

			uint32_t egout;

			switch(eg_mode) {

				case ATTACK:
					egout = _tables->AR_ADJUST_TABLE[GetHighBits(eg_phase, EG_DP_BITS - EG_BITS)];
					eg_phase += eg_dphase;
					if((EG_DP_WIDTH & eg_phase) || (patch.AR == 15)) {
						egout = 0;
						eg_phase = 0;
						eg_mode = DECAY;

						UpdateEg();
					}
					break;

				case DECAY:
					egout = GetHighBits(eg_phase, EG_DP_BITS - EG_BITS);
					eg_phase += eg_dphase;
					if(eg_phase >= SL[patch.SL]) {
						if(patch.EG) {
							eg_phase = SL[patch.SL];
							eg_mode = SUSHOLD;
							UpdateEg();
						} else {
							eg_phase = SL[patch.SL];
							eg_mode = SUSTINE;
							UpdateEg();
						}
					}
					break;

				case SUSHOLD:
					egout = GetHighBits(eg_phase, EG_DP_BITS - EG_BITS);
					if(patch.EG == 0) {
						eg_mode = SUSTINE;
						UpdateEg();
					}
					break;

				case SUSTINE:
				case RELEASE:
					egout = GetHighBits(eg_phase, EG_DP_BITS - EG_BITS);
					eg_phase += eg_dphase;
					if(egout >= (1 << EG_BITS)) {
						eg_mode = FINISH;
						egout = (1 << EG_BITS) - 1;
					}
					break;

				case FINISH:
					egout = (1 << EG_BITS) - 1;
					break;

				default:
					egout = (1 << EG_BITS) - 1;
					break;
			}

			if(patch.AM) {
				egout = EG2DB(egout + tll) + lfo;
			} else {
				egout = EG2DB(egout + tll);
			}

			if(egout >= DB_MUTE) {
				egout = DB_MUTE - 1;
			}

			this->egout = egout;
		}
	};
}
