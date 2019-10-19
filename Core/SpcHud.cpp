#include "stdafx.h"
#include "SpcHud.h"
#include "Console.h"
#include "SoundMixer.h"
#include "DebugHud.h"
#include "SpcFileData.h"

SpcHud::SpcHud(Console * console, SpcFileData* spcData)
{
	_mixer = console->GetSoundMixer().get();
	_hud = console->GetDebugHud().get();
	_spcData = spcData;
}

void SpcHud::Draw(uint32_t frame)
{
	_hud->DrawString(20, 20, "Game:", 0xBBBBBB, 0, 1, frame);
	_hud->DrawString(20, 30, "Track:", 0xBBBBBB, 0, 1, frame);
	_hud->DrawString(20, 40, "Artist:", 0xBBBBBB, 0, 1, frame);
	_hud->DrawString(20, 50, "Comment:", 0xBBBBBB, 0, 1, frame);
	_hud->DrawString(20, 60, "Dumper:", 0xBBBBBB, 0, 1, frame);

	_hud->DrawString(70, 20, _spcData->GameTitle, 0xFFFFFF, 0, 1, frame);
	_hud->DrawString(70, 30, _spcData->SongTitle, 0xFFFFFF, 0, 1, frame);
	_hud->DrawString(70, 40, _spcData->Artist, 0xFFFFFF, 0, 1, frame);
	_hud->DrawString(70, 50, _spcData->Comment, 0xFFFFFF, 0, 1, frame);
	_hud->DrawString(70, 60, _spcData->Dumper, 0xFFFFFF, 0, 1, frame);

	int16_t left, right;
	_mixer->GetLastSamples(left, right);
	_volumesL[_volPosition] = left / 128;
	_volumesR[_volPosition] = right / 128;
	_volPosition = (_volPosition + 1) & 0x7F;
	for(int i = 1; i < 128; i++) {
		_hud->DrawLine((i - 1)*2, 160 + _volumesL[(_volPosition + i - 1) & 0x7F], i*2, 160 + _volumesL[(_volPosition + i) & 0x7F], 0x30FFAAAA, 1, frame);
		_hud->DrawLine((i - 1)*2, 160 + _volumesR[(_volPosition + i - 1) & 0x7F], i*2, 160 + _volumesR[(_volPosition + i) & 0x7F], 0x30AAAAFF, 1, frame);
	}
}
