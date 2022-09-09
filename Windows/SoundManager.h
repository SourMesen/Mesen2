#pragma once

#include "Common.h"
#include "Core/Shared/Audio/BaseSoundManager.h"

class Emulator;

struct SoundDeviceInfo
{
	string description;
	GUID guid;
};

class SoundManager : public BaseSoundManager
{
public:
	SoundManager(Emulator* emu, HWND hWnd);
	~SoundManager();

	void Release();
	void ProcessEndOfFrame();
	void PlayBuffer(int16_t *soundBuffer, uint32_t bufferSize, uint32_t sampleRate, bool isStereo);
	void Play();	
	void Pause();
	void Stop();

	string GetAvailableDevices();
	void SetAudioDevice(string deviceName);

private:
	vector<SoundDeviceInfo> GetAvailableDeviceInfo();
	static bool CALLBACK DirectSoundEnumProc(LPGUID lpGUID, LPCWSTR lpszDesc, LPCSTR lpszDrvName, LPVOID lpContext);
	bool InitializeDirectSound(uint32_t sampleRate, bool isStereo);
	void ClearSecondaryBuffer();
	void CopyToSecondaryBuffer(uint8_t *data, uint32_t size);
	void ValidateWriteCursor(DWORD safeWriteCursor);

private:
	Emulator* _emu;
	HWND _hWnd;
	GUID _audioDeviceID;
	bool _needReset = false;
	string _audioDeviceName = "";

	DWORD _lastWriteOffset = 0;
	uint32_t _previousLatency = 0;
	bool _playing = false;

	IDirectSound8* _directSound;
	IDirectSoundBuffer* _primaryBuffer;
	IDirectSoundBuffer8* _secondaryBuffer;
};
