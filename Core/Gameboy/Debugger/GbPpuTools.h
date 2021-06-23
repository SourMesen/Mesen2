#pragma once
#include "stdafx.h"
#include "Debugger/PpuTools.h"

class Debugger;
class Emulator;

class GbPpuTools : public PpuTools
{
public:
	GbPpuTools(Debugger* debugger, Emulator *emu);

	void GetTilemap(GetTilemapOptions options, BaseState& state, uint8_t* vram, uint32_t* palette, uint32_t *outBuffer) override;
	FrameInfo GetTilemapSize(GetTilemapOptions options, BaseState& state) override;

	void GetSpritePreview(GetSpritePreviewOptions options, BaseState& state, uint8_t* vram, uint8_t* oamRam, uint32_t* palette, uint32_t *outBuffer) override;
	FrameInfo GetSpritePreviewSize(GetSpritePreviewOptions options, BaseState& state) override;
};