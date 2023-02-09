#pragma once
#include "pch.h"
#include "Utilities/VirtualFile.h"

struct HesFileData
{
	uint32_t DataSize = 0;
	uint32_t LoadAddress = 0;
	uint16_t RequestAddress = 0;
	uint8_t FirstSong = 0;
	uint8_t CurrentTrack = 0;
	uint8_t InitialMpr[8] = {};

	vector<uint8_t> RomData;

	bool LoadFile(VirtualFile& hesFile)
	{
		vector<uint8_t> fileData;
		if(!hesFile.ReadFile(fileData) || fileData.size() < 0x20) {
			return false;
		}

		if(memcmp(fileData.data(), "HESM", 4) != 0) {
			return false;
		}

		FirstSong = fileData[5];
		CurrentTrack = FirstSong;
		RequestAddress = fileData[6] | (fileData[7] << 8);
		for(int i = 0; i < 8; i++) {
			InitialMpr[i] = fileData[8 + i];
		}

		DataSize = fileData[0x14] | (fileData[0x15] << 8) | (fileData[0x16] << 16) | (fileData[0x17] << 24);
		LoadAddress = fileData[0x18] | (fileData[0x19] << 8) | (fileData[0x1A] << 16) | (fileData[0x1B] << 24);

		if(DataSize > 0x80 * 0x2000) {
			return false;
		} else if(DataSize > hesFile.GetSize() - 0x20) {
			return false;
		} else if(LoadAddress + DataSize > 0x80 * 0x2000) {
			return false;
		}

		RomData.resize(0x80 * 0x2000);
		memcpy(RomData.data() + LoadAddress, fileData.data() + 0x20, DataSize);

		return true;
	}

	void ProcessAudioPlayerAction(AudioPlayerActionParams p, Emulator* emu)
	{
		int selectedTrack = CurrentTrack;
		switch(p.Action) {
			case AudioPlayerAction::NextTrack: selectedTrack++; break;
			case AudioPlayerAction::PrevTrack:
				if(emu->GetAudioTrackInfo().Position < 2) {
					selectedTrack--;
				}
				break;
			case AudioPlayerAction::SelectTrack: selectedTrack = (int)p.TrackNumber; break;
		}

		if(selectedTrack < 0) {
			selectedTrack = 255;
		} else if(selectedTrack > 255) {
			selectedTrack = 0;
		}

		//Asynchronously move to the next track
		//Can't do this in the current thread in some contexts (e.g when track reaches end)
		//because this is called from the emulation thread, which may cause infinite recursion
		thread switchTrackTask([emu, selectedTrack]() {
			auto lock = emu->AcquireLock(false);
			emu->ReloadRom(false);
			((PceConsole*)emu->GetConsole().get())->InitHesPlayback(selectedTrack);
		});
		switchTrackTask.detach();
	}

	AudioTrackInfo GetAudioTrackInfo(double position)
	{
		AudioTrackInfo info = {};
		info.TrackNumber = CurrentTrack + 1;
		info.TrackCount = 256;
		info.Position = position;
		info.Length = 0;
		info.FadeLength = 0;
		return info;
	}
};