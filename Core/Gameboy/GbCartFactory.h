#pragma once
#include "pch.h"
#include "Gameboy/Carts/GbCart.h"
#include "Gameboy/Carts/GbMbc1.h"
#include "Gameboy/Carts/GbMbc2.h"
#include "Gameboy/Carts/GbMbc3.h"
#include "Gameboy/Carts/GbMbc5.h"

class GbCartFactory
{
public:
	static GbCart* CreateCart(Emulator* emu, uint8_t cartType)
	{
		switch(cartType) {
			case 0:
				return new GbCart();

			case 1: case 2: case 3:
				return new GbMbc1();

			case 5: case 6:
				return new GbMbc2();

			case 15: case 16:
				return new GbMbc3(emu, true);

			case 17: case 18: case 19:
				return new GbMbc3(emu, false);

			case 25: case 26: case 27: case 28: case 29: case 30:
				return new GbMbc5();
		};

		return nullptr;
	}
};
