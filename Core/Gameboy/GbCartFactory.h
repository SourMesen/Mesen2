#pragma once
#include "pch.h"
#include "Gameboy/Carts/GbCart.h"
#include "Gameboy/Carts/GbMbc1.h"
#include "Gameboy/Carts/GbMbc2.h"
#include "Gameboy/Carts/GbMbc3.h"
#include "Gameboy/Carts/GbMbc5.h"
#include "Gameboy/Carts/GbMbc6.h"
#include "Gameboy/Carts/GbMbc7.h"
#include "Gameboy/Carts/GbMmm01.h"
#include "Gameboy/Carts/GbM161.h"
#include "Gameboy/Carts/GbHuc1.h"
#include "Gameboy/Carts/GbWisdomTree.h"
#include "Gameboy/GbxFooter.h"

class GbCartFactory
{
private:
	static GbCart* LoadFromGbxFooter(Emulator* emu, GbxFooter& footer, vector<uint8_t>& prgRom)
	{
		string mapperId = footer.GetMapperId();

		MessageManager::Log("[GBX] Mapper: " + mapperId);
		MessageManager::Log(string("[GBX] ROM Size: ") + std::to_string(footer.GetRomSize() / 1024) + " KB");
		MessageManager::Log(string("[GBX] RAM Size: ") + std::to_string(footer.GetRamSize() / 1024) + " KB");
		MessageManager::Log("[GBX] Battery: " + string(footer.HasBattery() ? "Yes" : "No"));
		MessageManager::Log("[GBX] RTC: " + string(footer.HasRtc() ? "Yes" : "No"));
		MessageManager::Log("[GBX] Rumble: " + string(footer.HasRumble() ? "Yes" : "No"));

		if(mapperId == "ROM") {
			return new GbCart();
		} else if(mapperId == "MBC1") {
			return new GbMbc1(false);
		} else if(mapperId == "MBC2") {
			return new GbMbc2();
		} else if(mapperId == "MBC3") {
			bool isMbc30 = footer.GetRamSize() >= 0x10000;
			return new GbMbc3(emu, footer.HasRtc(), isMbc30);
		} else if(mapperId == "MBC5") {
			return new GbMbc5(footer.HasRumble());
		} else if(mapperId == "MBC6") {
			return new GbMbc6();
		} else if(mapperId == "MBC7") {
			return new GbMbc7();
		} else if(mapperId == "MB1M") {
			return new GbMbc1(true);
		} else if(mapperId == "HUC1") {
			return new GbHuc1();
		} else if(mapperId == "WISD") {
			return new GbWisdomTree();
		} else if(mapperId == "MMM1") {
			return new GbMmm01();
		} else if(mapperId == "M161") {
			return new GbM161();
		}

		return nullptr;
	}

	static GbCart* LoadFromGbHeader(Emulator* emu, GameboyHeader& header, vector<uint8_t>& prgRom)
	{
		GbCart* cart = ProcessExceptions(header);
		if(cart) {
			return cart;
		}

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
				return new GbMmm01();

			case 0x0F: case 0x10:
			case 0x11: case 0x12: case 0x13:
			{
				bool isMbc30 = header.GetCartRamSize() >= 0x10000;
				bool hasRtc = header.CartType <= 0x10;
				return new GbMbc3(emu, hasRtc, isMbc30);
			}

			case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1E: {
				bool hasRumble = header.CartType >= 0x1C;
				return new GbMbc5(hasRumble);
			}

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

	static GbCart* ProcessExceptions(GameboyHeader& header)
	{
		if(header.CartType == 0x10 && header.GetCartName() == "TETRIS SET") {
			MessageManager::Log("Auto-detected cart type: M161");
			return new GbM161();
		}
		return nullptr;
	}

public:
	static GbCart* CreateCart(Emulator* emu, GameboyHeader& header, GbxFooter& gbxFooter, vector<uint8_t>& prgRom)
	{
		if(gbxFooter.IsValid()) {
			return LoadFromGbxFooter(emu, gbxFooter, prgRom);
		} else {
			return LoadFromGbHeader(emu, header, prgRom);
		}
	}

};
