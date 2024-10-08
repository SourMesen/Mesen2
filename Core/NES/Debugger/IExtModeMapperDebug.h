#pragma once
#include "pch.h"

struct NtExtConfig
{
	uint16_t SourceOffset;
	bool AttrExtMode;
	bool BgExtMode;
	bool FillMode;
};

struct ExtModeConfig
{
	NtExtConfig Nametables[5];
	bool SpriteExtMode;
	uint8_t ChrSource;
	uint8_t WindowBank;
	uint8_t WindowScrollX;
	uint8_t WindowScrollY;
	uint8_t BgExtBank;
	uint8_t SpriteExtBank;
	uint8_t SpriteExtData[64];
	uint8_t ExtRam[0x2000];
};

class IExtModeMapperDebug
{
public:
	virtual ExtModeConfig GetExModeConfig()
	{
		return {};
	}

	virtual bool HasExtendedAttributes(ExtModeConfig& extCfg, uint8_t ntIndex)
	{
		return false;
	}

	virtual bool HasExtendedBackground(ExtModeConfig& extCfg, uint8_t ntIndex)
	{
		return false;
	}

	virtual bool HasExtendedSprites(ExtModeConfig& extCfg)
	{
		return false;
	}

	virtual uint8_t GetExAttributePalette(ExtModeConfig& extCfg, uint8_t ntIndex, uint16_t ntAddr)
	{
		return 0;
	}

	virtual uint8_t GetExBackgroundChrData(ExtModeConfig& extCfg, uint8_t ntIndex, uint16_t ntAddr, uint16_t chrAddr)
	{
		return 0;
	}

	virtual uint8_t GetExSpriteChrData(ExtModeConfig& extCfg, uint8_t spriteIndex, uint16_t chrAddr)
	{
		return 0;
	}
};
