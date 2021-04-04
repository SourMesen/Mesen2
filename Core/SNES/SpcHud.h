#pragma once
#include "stdafx.h"

class Emulator;
class SoundMixer;
class DebugHud;
class SpcFileData;

class SpcHud
{
private:
	DebugHud* _hud;
	SoundMixer* _mixer;

	int8_t _volumesL[128] = {};
	int8_t _volumesR[128] = {};
	uint8_t _volPosition = 0;

	SpcFileData* _spcData;

public:
	SpcHud(Emulator* emu, SpcFileData* spcData);

	void Draw(uint32_t frame);
};