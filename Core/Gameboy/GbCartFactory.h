#pragma once
#include "pch.h"
#include "Gameboy/Carts/GbCart.h"
#include "Gameboy/Carts/GbMbc1.h"
#include "Gameboy/Carts/GbMbc2.h"
#include "Gameboy/Carts/GbMbc3.h"
#include "Gameboy/Carts/GbMbc5.h"
#include "Gameboy/Carts/GbMbc6.h"
#include "Gameboy/Carts/GbHuc1.h"

class GbCartFactory
{
public:
	static GbCart* CreateCart(Emulator* emu, uint8_t cartType)
	{
		switch(cartType) {
			case 0x00:
				return new GbCart();

			case 0x01: case 0x02: case 0x03:
				return new GbMbc1();

			case 0x05: case 0x06:
				return new GbMbc2();

			case 0x0B: case 0x0C: case 0x0D:
				//MMM01
				break;

			case 0x0F: case 0x10:
				return new GbMbc3(emu, true);

			case 0x11: case 0x12: case 0x13:
				return new GbMbc3(emu, false);

			case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1E:
				return new GbMbc5();

			case 0x20:
				return new GbMbc6();

			case 0x22:
				//MBC7
				break;

			case 0xFE:
				//HuC3
				break;

			case 0xFF:
				return new GbHuc1();
		};

		return nullptr;
	}
};
