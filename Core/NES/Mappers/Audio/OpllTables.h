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
#include "pch.h"
#include "Utilities/Serializer.h"

namespace Vrc7Opll {
	/* Size of Sintable ( 8 -- 18 can be used. 9 recommended.)*/
	const int32_t PG_BITS = 9;
	const int32_t PG_WIDTH = 1 << PG_BITS;

	/* Phase increment counter */
	const int32_t DP_BITS = 18;
	const int32_t DP_WIDTH = 1 << DP_BITS;
	const int32_t DP_BASE_BITS = DP_BITS - PG_BITS;

	/* Dynamic range (Accuracy of sin table) */
	const int32_t DB_BITS = 8;
	const double DB_STEP = (48.0 / (1 << DB_BITS));
	const int32_t DB_MUTE = 1 << DB_BITS;

	/* Dynamic range of envelope */
	const double EG_STEP = 0.375;
	const int32_t EG_BITS = 7;
	const int32_t EG_MUTE = (1 << EG_BITS);

	/* Dynamic range of total level */
	const double TL_STEP = 0.75;
	const int32_t TL_BITS = 6;
	const int32_t TL_MUTE = (1 << TL_BITS);

	/* Dynamic range of sustine level */
	const double SL_STEP = 3.0;
	const int32_t SL_BITS = 4;
	const int32_t SL_MUTE = (1 << SL_BITS);

	/* Bits for liner value */
	const int32_t DB2LIN_AMP_BITS = 11;
	const int32_t SLOT_AMP_BITS = DB2LIN_AMP_BITS;

	/* Bits for envelope phase incremental counter */
	const int32_t EG_DP_BITS = 22;
	const int32_t EG_DP_WIDTH = (1 << EG_DP_BITS);

	/* Bits for Pitch and Amp modulator */
	const int32_t PM_PG_BITS = 8;
	const int32_t PM_PG_WIDTH = (1 << PM_PG_BITS);
	const int32_t PM_DP_BITS = 16;
	const int32_t PM_DP_WIDTH = (1 << PM_DP_BITS);
	const int32_t AM_PG_BITS = 8;
	const int32_t AM_PG_WIDTH = (1 << AM_PG_BITS);
	const int32_t AM_DP_BITS = 16;
	const int32_t AM_DP_WIDTH = (1 << AM_DP_BITS);

	/* PM table is calcurated by PM_AMP * pow(2,PM_DEPTH*sin(x)/1200) */
	const int32_t PM_AMP_BITS = 8;
	const int32_t PM_AMP = (1 << PM_AMP_BITS);

	/* PM speed(Hz) and depth(cent) */
	const double PM_SPEED = 6.4;
	const double PM_DEPTH = 13.75;

	/* AM speed(Hz) and depth(dB) */
	const double AM_SPEED = 3.7;
	const double AM_DEPTH = 4.8;

	const double PI = 3.14159265358979323846;

	/* Definition of envelope mode */
	enum
	{
		SETTLE, ATTACK, DECAY, SUSHOLD, SUSTINE, RELEASE, FINISH
	};

	class OpllTables : public ISerializable
	{
	private:
		/* Convert Amp(0 to EG_HEIGHT) to Phase(0 to 4PI). */
		int32_t wave2_4pi(int32_t e) 
		{
			return (e) >> (SLOT_AMP_BITS - PG_BITS - 1);
		}

		/* Convert Amp(0 to EG_HEIGHT) to Phase(0 to 8PI). */
		int32_t wave2_8pi(int32_t e)
		{
			return e >> (SLOT_AMP_BITS - PG_BITS - 2);
		}

		uint32_t TL2EG(uint32_t d)
		{
			return d*(int32_t)(TL_STEP / EG_STEP);
		}

		/* Adjust envelope speed which depends on sampling rate. */
		uint32_t rate_adjust(double x)
		{
			return (uint32_t)((rate == 49716 ? x : ((double)(x)*clk / 72 / rate + 0.5)));		/* added 0.5 to round the value*/
		}

	protected:
		void Serialize(Serializer& s) override
		{
			SV(clk); SV(rate);
			if(!s.IsSaving()) {
				maketables(clk, rate);
			}
		}

	public:
		virtual ~OpllTables() = default;

		/* Input clock */
		uint32_t clk = 844451141;
		/* Sampling rate */
		uint32_t rate = 3354932;
		
		/* WaveTable for each envelope amp */
		uint16_t fullsintable[PG_WIDTH];
		uint16_t halfsintable[PG_WIDTH];

		/* LFO Table */
		int32_t pmtable[PM_PG_WIDTH];
		int32_t amtable[AM_PG_WIDTH];

		/* dB to Liner table */
		int16_t DB2LIN_TABLE[(DB_MUTE + DB_MUTE) * 2];

		/* Liner to Log curve conversion table (for Attack rate). */
		uint16_t AR_ADJUST_TABLE[1 << EG_BITS];

		/* Phase incr table for Attack */
		uint32_t dphaseARTable[16][16];
		/* Phase incr table for Decay and Release */
		uint32_t dphaseDRTable[16][16];

		/* KSL + TL Table */
		uint32_t tllTable[16][8][1 << TL_BITS][4];
		int32_t rksTable[2][8][2];

		/* Phase incr table for PG */
		uint32_t dphaseTable[512][8][16];

		uint16_t *waveform[2] = { fullsintable, halfsintable };

		/* Phase delta for LFO */
		uint32_t pm_dphase;
		uint32_t am_dphase;

		/* Table for AR to LogCurve. */
		void makeAdjustTable()
		{
			int32_t i;

			AR_ADJUST_TABLE[0] = (1 << EG_BITS);
			for(i = 1; i < 128; i++)
				AR_ADJUST_TABLE[i] = (uint16_t)((double)(1 << EG_BITS) - 1 - (1 << EG_BITS) * log((double)i) / log(128.0));
		}


		/* Table for dB(0 -- (1<<DB_BITS)-1) to Liner(0 -- DB2LIN_AMP_WIDTH) */
		void makeDB2LinTable()
		{
			int32_t i;

			for(i = 0; i < DB_MUTE + DB_MUTE; i++) {
				DB2LIN_TABLE[i] = (int16_t)((double)((1 << DB2LIN_AMP_BITS) - 1) * pow(10.0, -(double)i * DB_STEP / 20));
				if(i >= DB_MUTE) DB2LIN_TABLE[i] = 0;
				DB2LIN_TABLE[i + DB_MUTE + DB_MUTE] = (int16_t)(-DB2LIN_TABLE[i]);
			}
		}

		/* Liner(+0.0 - +1.0) to dB((1<<DB_BITS) - 1 -- 0) */
		int32_t lin2db(double d)
		{
			if(d == 0)
				return (DB_MUTE - 1);
			else
				return Min(-(int32_t)(20.0 * log10(d) / DB_STEP), DB_MUTE - 1);	/* 0 -- 127 */
		}


		/* Sin Table */
		void makeSinTable(void)
		{
			int32_t i;

			for(i = 0; i < PG_WIDTH / 4; i++) {
				fullsintable[i] = (uint16_t)lin2db(sin(2.0 * PI * i / PG_WIDTH));
			}

			for(i = 0; i < PG_WIDTH / 4; i++) {
				fullsintable[PG_WIDTH / 2 - 1 - i] = fullsintable[i];
			}

			for(i = 0; i < PG_WIDTH / 2; i++) {
				fullsintable[PG_WIDTH / 2 + i] = (uint16_t)(DB_MUTE + DB_MUTE + fullsintable[i]);
			}

			for(i = 0; i < PG_WIDTH / 2; i++)
				halfsintable[i] = fullsintable[i];
			for(i = PG_WIDTH / 2; i < PG_WIDTH; i++)
				halfsintable[i] = fullsintable[0];
		}

		/* Table for Pitch Modulator */
		void makePmTable(void)
		{
			int32_t i;

			for(i = 0; i < PM_PG_WIDTH; i++)
				pmtable[i] = (int32_t)((double)PM_AMP * pow(2.0, (double)PM_DEPTH * sin(2.0 * PI * i / PM_PG_WIDTH) / 1200));
		}

		/* Table for Amp Modulator */
		void makeAmTable(void)
		{
			int32_t i;

			for(i = 0; i < AM_PG_WIDTH; i++)
				amtable[i] = (int32_t)((double)AM_DEPTH / 2 / DB_STEP * (1.0 + sin(2.0 * PI * i / PM_PG_WIDTH)));
		}

		/* Phase increment counter table */
		void makeDphaseTable(void)
		{
			uint32_t fnum, block, ML;
			uint32_t mltable[16] =
			{ 1, 1 * 2, 2 * 2, 3 * 2, 4 * 2, 5 * 2, 6 * 2, 7 * 2, 8 * 2, 9 * 2, 10 * 2, 10 * 2, 12 * 2, 12 * 2, 15 * 2, 15 * 2 };

			for(fnum = 0; fnum < 512; fnum++)
				for(block = 0; block < 8; block++)
					for(ML = 0; ML < 16; ML++)
						dphaseTable[fnum][block][ML] = rate_adjust(((fnum * mltable[ML]) << block) >> (20 - DP_BITS));
		}

		void makeTllTable(void)
		{
			static double kltable[16] = {
				0.00, 18.00, 24.00, 27.75, 30.00, 32.25, 33.75, 35.25,
				36.00, 37.50, 38.25, 39.00, 39.75, 40.50, 41.25, 42.00
			};

			int32_t tmp;
			int32_t fnum, block, TL, KL;

			for(fnum = 0; fnum < 16; fnum++)
				for(block = 0; block < 8; block++)
					for(TL = 0; TL < 64; TL++)
						for(KL = 0; KL < 4; KL++) {
							if(KL == 0) {
								tllTable[fnum][block][TL][KL] = TL2EG(TL);
							} else {
								tmp = (int32_t)(kltable[fnum] - 6.000 * (7 - block));
								if(tmp <= 0)
									tllTable[fnum][block][TL][KL] = TL2EG(TL);
								else
									tllTable[fnum][block][TL][KL] = (uint32_t)((tmp >> (3 - KL)) / EG_STEP) + TL2EG(TL);
							}
						}
		}

		/* Rate Table for Attack */
		void makeDphaseARTable(void)
		{
			int32_t AR, Rks, RM, RL;

			for(AR = 0; AR < 16; AR++)
				for(Rks = 0; Rks < 16; Rks++) {
					RM = AR + (Rks >> 2);
					RL = Rks & 3;
					if(RM > 15)
						RM = 15;
					switch(AR) {
						case 0:
							dphaseARTable[AR][Rks] = 0;
							break;
						case 15:
							dphaseARTable[AR][Rks] = 0;/*EG_DP_WIDTH;*/
							break;
						default:
							dphaseARTable[AR][Rks] = rate_adjust((3 * (RL + 4) << (RM + 1)));
							break;
					}
				}
		}

		/* Rate Table for Decay and Release */
		void makeDphaseDRTable(void)
		{
			int32_t DR, Rks, RM, RL;

			for(DR = 0; DR < 16; DR++)
				for(Rks = 0; Rks < 16; Rks++) {
					RM = DR + (Rks >> 2);
					RL = Rks & 3;
					if(RM > 15)
						RM = 15;
					switch(DR) {
						case 0:
							dphaseDRTable[DR][Rks] = 0;
							break;
						default:
							dphaseDRTable[DR][Rks] = rate_adjust((RL + 4) << (RM - 1));
							break;
					}
				}
		}

		void makeRksTable(void)
		{

			int32_t fnum8, block, KR;

			for(fnum8 = 0; fnum8 < 2; fnum8++)
				for(block = 0; block < 8; block++)
					for(KR = 0; KR < 2; KR++) {
						if(KR != 0)
							rksTable[fnum8][block][KR] = (block << 1) + fnum8;
						else
							rksTable[fnum8][block][KR] = block >> 1;
					}
		}

		int32_t Min(int32_t i, int32_t j)
		{
			if(i < j)
				return i;
			else
				return j;
		}

		void internal_refresh()
		{
			makeDphaseTable();
			makeDphaseARTable();
			makeDphaseDRTable();
			pm_dphase = (uint32_t)rate_adjust(PM_SPEED * PM_DP_WIDTH / (clk / 72));
			am_dphase = (uint32_t)rate_adjust(AM_SPEED * AM_DP_WIDTH / (clk / 72));
		}

		void maketables(uint32_t c, uint32_t r)
		{
			clk = c;
			makePmTable();
			makeAmTable();
			makeDB2LinTable();
			makeAdjustTable();
			makeTllTable();
			makeRksTable();
			makeSinTable();
			//makeDefaultPatch ();

			rate = r;
			internal_refresh();
		}
	};
}