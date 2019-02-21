#include "stdafx.h"
#include "SoundManager.h"
#include "../Core/SoundMixer.h"
#include "../Core/Console.h"
#include "../Core/MessageManager.h"

SoundManager::SoundManager(shared_ptr<Console> console, HWND hwnd)
{
	_console = console;
	_hWnd = hwnd;
	_directSound = 0;
	_primaryBuffer = 0;
	_secondaryBuffer = 0;

	memset(&_audioDeviceID, 0, sizeof(_audioDeviceID));

	if(InitializeDirectSound(44100, true)) {
		_console->GetSoundMixer()->RegisterAudioDevice(this);
	} else {
		MessageManager::DisplayMessage("Error", "CouldNotInitializeAudioSystem");
	}
}

SoundManager::~SoundManager()
{
	Release();
}

bool CALLBACK SoundManager::DirectSoundEnumProc(LPGUID lpGUID, LPCWSTR lpszDesc, LPCSTR lpszDrvName, LPVOID lpContext)
{
	vector<SoundDeviceInfo> *devices = (vector<SoundDeviceInfo>*)lpContext;

	SoundDeviceInfo deviceInfo;
	deviceInfo.description = utf8::utf8::encode(lpszDesc);
	if(lpGUID != nullptr) {
		memcpy((void*)&deviceInfo.guid, lpGUID, 16);
	} else {
		memset((void*)&deviceInfo.guid, 0, 16);
	}
	devices->push_back(deviceInfo);

	return true;
}

vector<SoundDeviceInfo> SoundManager::GetAvailableDeviceInfo()
{
	vector<SoundDeviceInfo> devices;
	DirectSoundEnumerateW((LPDSENUMCALLBACKW)SoundManager::DirectSoundEnumProc, &devices);
	return devices;
}

string SoundManager::GetAvailableDevices()
{
	string deviceString;
	for(SoundDeviceInfo device : GetAvailableDeviceInfo()) {
		deviceString += device.description + "||"s;
	}
	return deviceString;
}

void SoundManager::SetAudioDevice(string deviceName)
{
	for(SoundDeviceInfo device : GetAvailableDeviceInfo()) {
		if(device.description.compare(deviceName) == 0) {
			if(memcmp(&_audioDeviceID, &device.guid, 16) != 0) {
				memcpy(&_audioDeviceID, &device.guid, 16);
				_needReset = true;
			}
			break;
		}
	}
}

bool SoundManager::InitializeDirectSound(uint32_t sampleRate, bool isStereo)
{
	HRESULT result;
	DSBUFFERDESC bufferDesc;
	WAVEFORMATEX waveFormat;
	
	// Initialize the direct sound interface pointer for the default sound device.
	result = DirectSoundCreate8(&_audioDeviceID, &_directSound, NULL);
	if(FAILED(result)) {
		MessageManager::Log("[Audio] Failed to create direct sound device.");
		return false;
	}

	// Set the cooperative level to priority so the format of the primary sound buffer can be modified.
	result = _directSound->SetCooperativeLevel(_hWnd, DSSCL_PRIORITY);
	if(FAILED(result)) {
		MessageManager::Log("[Audio] Failed to set cooperative level.");
		return false;
	}

	// Setup the primary buffer description.
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
	bufferDesc.dwBufferBytes = 0;
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = NULL;
	bufferDesc.guid3DAlgorithm = GUID_NULL;

	// Get control of the primary sound buffer on the default sound device.
	result = _directSound->CreateSoundBuffer(&bufferDesc, &_primaryBuffer, NULL);
	if(FAILED(result)) {
		MessageManager::Log("[Audio] Failed to create primary sound buffer.");
		return false;
	}

	// Setup the format of the primary sound bufffer.
	_sampleRate = sampleRate;
	_isStereo = isStereo;

	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nSamplesPerSec = _sampleRate;
	waveFormat.wBitsPerSample = 16;
	waveFormat.nChannels = isStereo ? 2 : 1;
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;

	// Set the primary buffer to be the wave format specified.
	result = _primaryBuffer->SetFormat(&waveFormat);
	if(FAILED(result)) {
		MessageManager::Log("[Audio] Failed to set the sound format.");
		return false;
	}

	//TODO
	int32_t latency = 100; //_console->GetSettings()->GetAudioLatency()
	int32_t requestedByteLatency = (int32_t)((float)(sampleRate * latency) / 1000.0f * waveFormat.nBlockAlign);
	_bufferSize = (int32_t)std::ceil((double)requestedByteLatency * 2 / 0x10000) * 0x10000;

	// Set the buffer description of the secondary sound buffer that the wave file will be loaded onto.
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS | DSBCAPS_LOCSOFTWARE | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY;
	bufferDesc.dwBufferBytes = _bufferSize;
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = &waveFormat;
	bufferDesc.guid3DAlgorithm = GUID_NULL;

	// Create a temporary sound buffer with the specific buffer settings.
	IDirectSoundBuffer* tempBuffer;
	result = _directSound->CreateSoundBuffer(&bufferDesc, &tempBuffer, NULL);
	if(FAILED(result)) {
		MessageManager::Log("[Audio] Failed to create temporary sound buffer.");
		return false;
	}

	// Test the buffer format against the direct sound 8 interface and create the secondary buffer.
	result = tempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (LPVOID*)&_secondaryBuffer);
	if(FAILED(result)) {
		MessageManager::Log("[Audio] Failed to obtain secondary sound buffer.");
		return false;
	}

	// Set volume of the buffer to 100%.
	result = _secondaryBuffer->SetVolume(DSBVOLUME_MAX);
	if(FAILED(result)) {
		MessageManager::Log("[Audio] Failed to set volume of the secondary sound buffer.");
		return false;
	}

	// Release the temporary buffer.
	tempBuffer->Release();

	_playing = false;

	return true;
}

void SoundManager::Release()
{
	_playing = false;
	_needReset = false;
	_lastWriteOffset = 0;

	if(_secondaryBuffer) {
		_secondaryBuffer->Release();
		_secondaryBuffer = nullptr;
	}

	if(_primaryBuffer) {
		_primaryBuffer->Release();
		_primaryBuffer = nullptr;
	}
	
	if(_directSound) {
		_directSound->Release();
		_directSound = nullptr;
	}
}

void SoundManager::ClearSecondaryBuffer()
{
	unsigned char* bufferPtr;
	DWORD bufferSize;
	_secondaryBuffer->Lock(0, 0, (void**)&bufferPtr, (DWORD*)&bufferSize, nullptr, 0, DSBLOCK_ENTIREBUFFER);
	memset(bufferPtr, 0, bufferSize);
	_secondaryBuffer->Unlock((void*)bufferPtr, bufferSize, nullptr, 0);

	_secondaryBuffer->SetCurrentPosition(0);
	_lastWriteOffset = 0;
}

void SoundManager::CopyToSecondaryBuffer(uint8_t *data, uint32_t size)
{
	uint8_t* bufferPtrA;
	uint8_t* bufferPtrB;
	DWORD bufferASize;
	DWORD bufferBSize;

	_secondaryBuffer->Lock(_lastWriteOffset, size, (void**)&bufferPtrA, (DWORD*)&bufferASize, (void**)&bufferPtrB, (DWORD*)&bufferBSize, 0);
	_lastWriteOffset = (_lastWriteOffset + size) % _bufferSize;

	memcpy(bufferPtrA, data, bufferASize);
	if(bufferPtrB && bufferBSize > 0) {
		memcpy(bufferPtrB, data + bufferASize, bufferBSize);
	}

	_secondaryBuffer->Unlock((void*)bufferPtrA, bufferASize, (void*)bufferPtrB, bufferBSize);
}

void SoundManager::Pause()
{
	if(_secondaryBuffer) {
		_secondaryBuffer->Stop();
	}
	_playing = false;
}

void SoundManager::Stop()
{
	if(_secondaryBuffer) {
		_secondaryBuffer->Stop();
		ClearSecondaryBuffer();
	}

	_playing = false;
	ResetStats();
}

void SoundManager::Play()
{
	if(_secondaryBuffer) {
		_secondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
		_playing = true;
	}
}

void SoundManager::ValidateWriteCursor(DWORD safeWriteCursor)
{
	int32_t writeGap = _lastWriteOffset - safeWriteCursor;
	if(writeGap < -10000) {
		writeGap += _bufferSize;
	} else if(writeGap < 0) {
		_bufferUnderrunEventCount++;
		_lastWriteOffset = safeWriteCursor;
	}
}

void SoundManager::ProcessEndOfFrame()
{
	DWORD currentPlayCursor;
	DWORD safeWriteCursor;
	_secondaryBuffer->GetCurrentPosition(&currentPlayCursor, &safeWriteCursor);
	ValidateWriteCursor(safeWriteCursor);

	//TODO
	uint32_t emulationSpeed = 100; // _console->GetSettings()->GetEmulationSpeed();
	uint32_t targetRate = _sampleRate;
	if(emulationSpeed > 0 && emulationSpeed < 100) {
		//Slow down playback when playing at less than 100%
		targetRate = (uint32_t)(targetRate * ((double)emulationSpeed / 100.0));
	}
	_secondaryBuffer->SetFrequency((DWORD)(targetRate));

	ProcessLatency(currentPlayCursor, _lastWriteOffset);

	//TODO
	uint32_t latency = 100; //_console->GetSettings()->GetAudioLatency();
	if(_averageLatency > 0 && emulationSpeed <= 100 && emulationSpeed > 0 && std::abs(_averageLatency - latency) > 50) {
		//Latency is way off (over 50ms gap), stop audio & start again
		Stop();
	}
}

void SoundManager::PlayBuffer(int16_t *soundBuffer, uint32_t sampleCount, uint32_t sampleRate, bool isStereo)
{
	uint32_t bytesPerSample = 2 * (isStereo ? 2 : 1);
	//TODO
	uint32_t latency = 100; //_console->GetSettings()->GetAudioLatency();
	if(_sampleRate != sampleRate || _isStereo != isStereo || _needReset || latency != _previousLatency) {
		_previousLatency = latency;
		Release();
		InitializeDirectSound(sampleRate, isStereo);
		_secondaryBuffer->SetFrequency(sampleRate);
	}

	DWORD currentPlayCursor, safeWriteCursor;
	_secondaryBuffer->GetCurrentPosition(&currentPlayCursor, &safeWriteCursor);
	ValidateWriteCursor(safeWriteCursor);

	uint32_t soundBufferSize = sampleCount * bytesPerSample;
	CopyToSecondaryBuffer((uint8_t*)soundBuffer, soundBufferSize);
	
	if(!_playing) {
		DWORD byteLatency = (int32_t)((float)(sampleRate * latency) / 1000.0f * bytesPerSample);
		if(_lastWriteOffset >= byteLatency / 2) {
			Play();
		}
	}
}