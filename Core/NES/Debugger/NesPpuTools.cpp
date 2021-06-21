#include "stdafx.h"
#include "NES/Debugger/NesPpuTools.h"
#include "Debugger/DebugTypes.h"
#include "Shared/SettingTypes.h"

NesPpuTools::NesPpuTools(Debugger* debugger, Emulator *emu) : PpuTools(debugger, emu)
{
}

void NesPpuTools::GetTilemap(GetTilemapOptions options, BaseState& baseState, uint8_t* vram, uint32_t* palette, uint32_t* outBuffer)
{
}

void NesPpuTools::GetSpritePreview(GetSpritePreviewOptions options, BaseState& baseState, uint8_t* vram, uint8_t* oamRam, uint32_t* palette, uint32_t* outBuffer)
{
}