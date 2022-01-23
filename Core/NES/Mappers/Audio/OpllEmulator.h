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
#include "OpllChannel.h"
#include "Utilities/Serializer.h"

namespace Vrc7Opll
{
	class OpllEmulator : public ISerializable
	{
	private:
		//Updated based on NukeYKT's data: https://siliconpr0n.org/archive/doku.php?id=vendor:yamaha:opl2#vrc7_instrument_rom_dump
		const unsigned char default_inst[15][8] = {
			{0x03, 0x21, 0x05, 0x06, 0xE8, 0x81, 0x42, 0x27},
			{0x13, 0x41, 0x14, 0x0D, 0xD8, 0xF6, 0x23, 0x12},
			{0x11, 0x11, 0x08, 0x08, 0xFA, 0xB2, 0x20, 0x12},
			{0x31, 0x61, 0x0C, 0x07, 0xA8, 0x64, 0x61, 0x27},
			{0x32, 0x21, 0x1E, 0x06, 0xE1, 0x76, 0x01, 0x28},
			{0x02, 0x01, 0x06, 0x00, 0xA3, 0xE2, 0xF4, 0xF4},
			{0x21, 0x61, 0x1D, 0x07, 0x82, 0x81, 0x11, 0x07},
			{0x23, 0x21, 0x22, 0x17, 0xA2, 0x72, 0x01, 0x17},
			{0x35, 0x11, 0x25, 0x00, 0x40, 0x73, 0x72, 0x01},
			{0xB5, 0x01, 0x0F, 0x0F, 0xA8, 0xA5, 0x51, 0x02},
			{0x17, 0xC1, 0x24, 0x07, 0xF8, 0xF8, 0x22, 0x12},
			{0x71, 0x23, 0x11, 0x06, 0x65, 0x74, 0x18, 0x16},
			{0x01, 0x02, 0xD3, 0x05, 0xC9, 0x95, 0x03, 0x02},
			{0x61, 0x63, 0x0C, 0x00, 0x94, 0xC0, 0x33, 0xF6},
			{0x21, 0x72, 0x0D, 0x00, 0xC1, 0xD5, 0x56, 0x06}
		};

		uint32_t adr = 0;
		int32_t _out = 0;

		uint32_t realstep = 0;
		uint32_t oplltime = 0;
		uint32_t opllstep = 0;
		int32_t prev = 0, next = 0;

		/* Register */
		uint8_t LowFreq[6] = {};
		uint8_t HiFreq[6] = {};
		uint8_t InstVol[6] = {};

		uint8_t CustInst[8] = {};

		int32_t slot_on_flag[6 * 2] = {};

		/* Pitch Modulator */
		uint32_t pm_phase = 0;
		int32_t lfo_pm = 0;

		/* Amp Modulator */
		int32_t am_phase = 0;
		int32_t lfo_am = 0;

		/* Channel Data */
		int32_t patch_number[6] = {};
		int32_t key_status[6] = {};

		/* Slot */
		OpllChannel slot[6 * 2];

		uint32_t _mask = 0;
		shared_ptr<OpllTables> tables;

		OpllChannel* GetModulator(uint8_t i)
		{
			return &slot[i << 1];
		}

		OpllChannel* GetCarrier(uint8_t i)
		{
			return &slot[(i << 1) | 1];
		}

		int32_t OPLL_MASK_CH(int32_t x)
		{
			return 1 << x;
		}

		/* Cut the lower b bit(s) off. */
		uint32_t GetHighBits(uint32_t c, uint32_t b)
		{
			return c >> b;
		}
	protected:
		void Serialize(Serializer& s) override
		{
			ArrayInfo<uint8_t> lowFreq{ LowFreq, 6 };
			ArrayInfo<uint8_t> hiFreq{ HiFreq, 6 };
			ArrayInfo<uint8_t> instVol{ InstVol, 6 };
			ArrayInfo<uint8_t> custInst{ CustInst, 8 };
			ArrayInfo<int32_t> slotOnFlag{ slot_on_flag, 12 };
			ArrayInfo<int32_t> patchNumber{ patch_number, 6 };
			ArrayInfo<int32_t> keyStatus{ key_status, 6 };

			s.Stream(tables.get());
			for(int i = 0; i < 12; i++) {
				s.Stream(&slot[i]);
			}

			s.Stream(adr, _out, realstep, oplltime, opllstep, prev, next, pm_phase, lfo_pm, am_phase, lfo_am, _mask,
				lowFreq, hiFreq, instVol, custInst, slotOnFlag, patchNumber, keyStatus
			);
		}

	public:
		OpllEmulator()
		{
			tables.reset(new Vrc7Opll::OpllTables());
			tables->maketables(3579545, 49716);

			for(int i = 0; i < 12; i++) {
				slot[i].SetTables(tables);
			}

			_mask = 0;
			Reset(tables->clk, tables->rate);
		}

		virtual ~OpllEmulator()
		{

		}

		/* Setup */
		void Reset(uint32_t clk, uint32_t rate)
		{
			for(int32_t i = 0; i < 12; i++) {
				slot[i].reset(i % 2);
			}

			for(int32_t i = 0; i < 6; i++) {
				key_status[i] = 0;
				//setPatch (opll, i, 0);
			}

			for(int32_t i = 0; i < 0x40; i++) {
				WriteReg(i, 0);
			}

			realstep = (uint32_t)((1 << 31) / rate);
			opllstep = (uint32_t)((1 << 31) / (clk / 72));
		}

		/* Port/Register access */
		void WriteReg(uint32_t reg, uint32_t val)
		{
			int32_t i, v, ch;

			uint32_t data = val & 0xff;
			reg = reg & 0x3f;

			switch(reg) {
				case 0x00:
					CustInst[0] = (uint8_t)data;
					for(i = 0; i < 6; i++) {
						if(patch_number[i] == 0) {
							setInstrument(i, 0);
							GetModulator(i)->UpdatePg();
							GetModulator(i)->UpdateRks();
							GetModulator(i)->UpdateEg();
						}
					}
					break;

				case 0x01:
					CustInst[1] = (uint8_t)data;
					for(i = 0; i < 6; i++) {
						if(patch_number[i] == 0) {
							setInstrument(i, 0);
							GetCarrier(i)->UpdatePg();
							GetCarrier(i)->UpdateRks();
							GetCarrier(i)->UpdateEg();
						}
					}
					break;

				case 0x02:
					CustInst[2] = (uint8_t)data;
					for(i = 0; i < 6; i++) {
						if(patch_number[i] == 0) {
							setInstrument(i, 0);
							GetModulator(i)->UpdateTll();
						}
					}
					break;

				case 0x03:
					CustInst[3] = (uint8_t)data;
					for(i = 0; i < 6; i++) {
						if(patch_number[i] == 0) {
							setInstrument(i, 0);
							GetModulator(i)->UpdateWf();
							GetCarrier(i)->UpdateWf();
						}
					}
					break;

				case 0x04:
					CustInst[4] = (uint8_t)data;
					for(i = 0; i < 6; i++) {
						if(patch_number[i] == 0) {
							setInstrument(i, 0);
							GetModulator(i)->UpdateEg();
						}
					}
					break;

				case 0x05:
					CustInst[5] = (uint8_t)data;
					for(i = 0; i < 6; i++) {
						if(patch_number[i] == 0) {
							setInstrument(i, 0);
							GetCarrier(i)->UpdateEg();
						}
					}
					break;

				case 0x06:
					CustInst[6] = (uint8_t)data;
					for(i = 0; i < 6; i++) {
						if(patch_number[i] == 0) {
							setInstrument(i, 0);
							GetModulator(i)->UpdateEg();
						}
					}
					break;

				case 0x07:
					CustInst[7] = (uint8_t)data;
					for(i = 0; i < 6; i++) {
						if(patch_number[i] == 0) {
							setInstrument(i, 0);
							GetCarrier(i)->UpdateEg();
						}
					}
					break;

				case 0x10:
				case 0x11:
				case 0x12:
				case 0x13:
				case 0x14:
				case 0x15:
					ch = reg - 0x10;
					LowFreq[ch] = (uint8_t)data;
					setFnumber(ch, data + ((HiFreq[ch] & 1) << 8));
					GetModulator(ch)->UpdateAll();
					GetCarrier(ch)->UpdateAll();
					break;

				case 0x20:
				case 0x21:
				case 0x22:
				case 0x23:
				case 0x24:
				case 0x25:
					ch = reg - 0x20;
					HiFreq[ch] = (uint8_t)data;

					setFnumber(ch, ((data & 1) << 8) + LowFreq[ch]);
					setBlock(ch, (data >> 1) & 7);
					setSustine(ch, (data >> 5) & 1);
					if(data & 0x10) {
						keyOn(ch);
					} else {
						keyOff(ch);
					}
					GetModulator(ch)->UpdateAll();
					GetCarrier(ch)->UpdateAll();
					update_key_status();
					break;

				case 0x30:
				case 0x31:
				case 0x32:
				case 0x33:
				case 0x34:
				case 0x35:
					InstVol[reg - 0x30] = (uint8_t)data;
					i = (data >> 4) & 15;
					v = data & 15;
					setInstrument(reg - 0x30, i);
					setVolume(reg - 0x30, v << 2);
					GetModulator(reg - 0x30)->UpdateAll();
					GetCarrier(reg - 0x30)->UpdateAll();
					break;

				default:
					break;

			}
		}

		void setInstrument(unsigned int i, unsigned int inst)
		{
			const uint8_t *src;
			OpllPatch *modp, *carp;

			patch_number[i] = inst;

			if(inst) {
				src = default_inst[inst - 1];
			} else {
				src = CustInst;
			}

			modp = GetModulator(i)->GetPatch();
			carp = GetCarrier(i)->GetPatch();

			modp->AM = (src[0] >> 7) & 1;
			modp->PM = (src[0] >> 6) & 1;
			modp->EG = (src[0] >> 5) & 1;
			modp->KR = (src[0] >> 4) & 1;
			modp->ML = (src[0] & 0xF);

			carp->AM = (src[1] >> 7) & 1;
			carp->PM = (src[1] >> 6) & 1;
			carp->EG = (src[1] >> 5) & 1;
			carp->KR = (src[1] >> 4) & 1;
			carp->ML = (src[1] & 0xF);

			modp->KL = (src[2] >> 6) & 3;
			modp->TL = (src[2] & 0x3F);

			carp->KL = (src[3] >> 6) & 3;
			carp->WF = (src[3] >> 4) & 1;

			modp->WF = (src[3] >> 3) & 1;

			modp->FB = (src[3]) & 7;

			modp->AR = (src[4] >> 4) & 0xF;
			modp->DR = (src[4] & 0xF);

			carp->AR = (src[5] >> 4) & 0xF;
			carp->DR = (src[5] & 0xF);

			modp->SL = (src[6] >> 4) & 0xF;
			modp->RR = (src[6] & 0xF);

			carp->SL = (src[7] >> 4) & 0xF;
			carp->RR = (src[7] & 0xF);
		}

		/* Misc */
		void ForceRefresh()
		{
			for(int i = 0; i < 12; i++) {
				slot[i].UpdatePg();
				slot[i].UpdateRks();
				slot[i].UpdateTll();
				slot[i].UpdateWf();
				slot[i].UpdateEg();
			}
		}

		/* Channel Mask */
		uint32_t SetMask(uint32_t mask)
		{
			uint32_t ret = mask;
			this->_mask = mask;
			return ret;
		}

		uint32_t ToggleMask(uint32_t mask)
		{
			uint32_t ret = mask;
			this->_mask ^= mask;
			return ret;
		}

		/* Channel key on */
		void keyOn(int32_t i)
		{
			if(!slot_on_flag[i * 2]) {
				GetModulator(i)->slotOn();
			}
			if(!slot_on_flag[i * 2 + 1]) {
				GetCarrier(i)->slotOn();
			}
			key_status[i] = 1;
		}

		/* Channel key off */
		void keyOff(int32_t i)
		{
			if(slot_on_flag[i * 2 + 1]) {
				GetCarrier(i)->slotOff();
			}
			key_status[i] = 0;
		}

		/* Set sustine parameter */
		void setSustine(int32_t c, int32_t sustine)
		{
			GetCarrier(c)->setSustine(sustine);
			if(GetModulator(c)->GetType()) {
				GetModulator(c)->setSustine(sustine);
			}
		}

		/* Volume : 6bit ( Volume register << 2 ) */
		void setVolume(int32_t c, int32_t volume)
		{
			GetCarrier(c)->setVolume(volume);
		}

		/* Set F-Number ( fnum : 9bit ) */
		void setFnumber(int32_t c, int32_t fnum)
		{
			GetCarrier(c)->setFnumber(fnum);
			GetModulator(c)->setFnumber(fnum);
		}

		/* Set Block data (block : 3bit ) */
		void setBlock(int32_t c, int32_t block)
		{
			GetCarrier(c)->setBlock(block);
			GetModulator(c)->setBlock(block);
		}

		void update_key_status()
		{
			for(int ch = 0; ch < 6; ch++) {
				slot_on_flag[ch * 2] = slot_on_flag[ch * 2 + 1] = (HiFreq[ch]) & 0x10;
			}
		}

		void update_ampm()
		{
			pm_phase = (pm_phase + tables->pm_dphase) & (PM_DP_WIDTH - 1);
			am_phase = (am_phase + tables->am_dphase) & (AM_DP_WIDTH - 1);
			lfo_am = tables->amtable[GetHighBits(am_phase, AM_DP_BITS - AM_PG_BITS)];
			lfo_pm = tables->pmtable[GetHighBits(pm_phase, PM_DP_BITS - PM_PG_BITS)];
		}

		int32_t calc()
		{
			int32_t inst = 0, out = 0;
			int32_t i;

			update_ampm();

			for(i = 0; i < 12; i++) {
				slot[i].calc_phase(lfo_pm);
				slot[i].calc_envelope(lfo_am);
			}

			for(i = 0; i < 6; i++) {
				if(!(_mask & OPLL_MASK_CH(i)) && (GetCarrier(i)->GetEgMode() != FINISH)) {
					inst += GetCarrier(i)->calc_slot_car(GetModulator(i)->calc_slot_mod());
				}
			}

			out = inst;
			return (int32_t)out;
		}

		int16_t GetOutput()
		{
			while(realstep > oplltime) {
				oplltime += opllstep;
				prev = next;
				next = calc();
			}

			oplltime -= realstep;
			_out = (int16_t)(((double)next * (opllstep - oplltime) + (double)prev * oplltime) / opllstep);

			return (int16_t)_out;
		}
	};
}