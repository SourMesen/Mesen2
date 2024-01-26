#pragma once
#include "pch.h"
#include "Gameboy/Carts/GbCart.h"
#include "Gameboy/Carts/GbMbc1.h"
#include "Gameboy/Carts/GbMbc2.h"
#include "Gameboy/Carts/GbMbc3.h"
#include "Gameboy/Carts/GbMbc5.h"
#include "Gameboy/Carts/GbMbc6.h"
#include "Gameboy/Carts/GbMbc7.h"
#include "Gameboy/Carts/GbHuc1.h"

class GbCartFactory
{
public:
	static GbCart* CreateCart(Emulator* emu, GameboyHeader& header, vector<uint8_t>& prgRom)
	{
		switch(header.CartType) {
			case 0x00:
				return new GbCart();

			case 0x01: case 0x02: case 0x03: {
				//When the boot rom logo appears at both of these offsets, this is usually a multicart collection
				//which uses a MBC1 chip with slightly different wiring.
				bool isMbc1m = prgRom.size() > 0x40134 && memcmp(&prgRom[0x104], &prgRom[0x40104], 0x30) == 0;
				return new GbMbc1(isMbc1m);
			}

			case 0x05: case 0x06:
				return new GbMbc2();

			case 0x0B: case 0x0C: case 0x0D:
				//MMM01
				break;

			case 0x0F: case 0x10:
			case 0x11: case 0x12: case 0x13: {
				bool isMbc30 = header.GetCartRamSize() >= 0x10000;
				bool hasRtc = header.CartType <= 0x10;
				return new GbMbc3(emu, hasRtc, isMbc30);
			}
							
			case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1E:
				return new GbMbc5();

			case 0x20:
				return new GbMbc6();

			case 0x22:
				return new GbMbc7();

			case 0xFE:
				//HuC3
				break;

			case 0xFF:
				return new GbHuc1();
		};

		return nullptr;
	}
};
