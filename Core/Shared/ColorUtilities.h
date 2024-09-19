#pragma once
#include "pch.h"

class ColorUtilities
{
public:
	static uint8_t Convert5BitTo8Bit(uint8_t color)
	{
		return (color << 3) + (color >> 2);
	}

	static uint8_t Convert4BitTo8Bit(uint8_t color)
	{
		return (color << 4) | color;
	}

	static uint32_t Rgb555ToArgb(uint16_t rgb555)
	{
		uint8_t b = Convert5BitTo8Bit((rgb555 >> 10) & 0x1F);
		uint8_t g = Convert5BitTo8Bit((rgb555 >> 5) & 0x1F);
		uint8_t r = Convert5BitTo8Bit(rgb555 & 0x1F);

		return 0xFF000000 | (r << 16) | (g << 8) | b;
	}

	static uint16_t Rgb222To555(uint8_t value)
	{
		return (
			((value & 0x30) << 9) | ((value & 0x30) << 7) | ((value & 0x20) << 5) |
			((value & 0x0C) << 6) | ((value & 0x0C) << 4) | ((value & 0x08) << 2) |
			((value & 0x03) << 3) | ((value & 0x03) << 1) | ((value & 0x02) >> 1)
		);
	}

	static uint16_t Rgb444To555(uint16_t value)
	{
		return (
			((value & 0xF00) << 3) | ((value & 0x800) >> 1) |
			((value & 0x0F0) << 2) | ((value & 0x080) >> 2) |
			((value & 0x00F) << 1) | ((value & 0x008) >> 3)
		);
	}

	static uint32_t Rgb222ToArgb(uint8_t rgb222)
	{
		return Rgb555ToArgb(Rgb222To555(rgb222));
	}

	static uint32_t Rgb444ToArgb(uint16_t rgb444)
	{
		return Rgb555ToArgb(Rgb444To555(rgb444));
	}

	static uint32_t Bgr444ToArgb(uint16_t bgr444)
	{
		uint8_t b = (bgr444 & 0x00F);
		uint8_t g = (bgr444 & 0x0F0) >> 4;
		uint8_t r = (bgr444 & 0xF00) >> 8;
		return 0xFF000000 | (r << 20) | (r << 16) | (g << 12) | (g << 8) | (b << 4) | (b << 0);
	}

	static uint16_t Rgb888To555(uint32_t rgb888)
	{
		uint8_t r = (rgb888 >> 19) & 0x1F;
		uint8_t g = (rgb888 >> 11) & 0x1F;
		uint8_t b = (rgb888 >> 3) & 0x1F;

		return (b << 10) | (g << 5) | r;
	}
};