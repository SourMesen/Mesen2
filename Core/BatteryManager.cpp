#include "stdafx.h"
#include "BatteryManager.h"
#include "../Utilities/VirtualFile.h"
#include "../Utilities/FolderUtilities.h"

void BatteryManager::Initialize(string romName)
{
	_romName = romName;
}

string BatteryManager::GetBasePath()
{
	return FolderUtilities::CombinePath(FolderUtilities::GetSaveFolder(), _romName);
}

void BatteryManager::SetBatteryProvider(shared_ptr<IBatteryProvider> provider)
{
	_provider = provider;
}

void BatteryManager::SetBatteryRecorder(shared_ptr<IBatteryRecorder> recorder)
{
	_recorder = recorder;
}

void BatteryManager::SaveBattery(string extension, uint8_t* data, uint32_t length)
{
	ofstream out(GetBasePath() + extension, ios::binary);
	if(out) {
		out.write((char*)data, length);
	}
}

vector<uint8_t> BatteryManager::LoadBattery(string extension)
{
	shared_ptr<IBatteryProvider> provider = _provider.lock();
	
	vector<uint8_t> batteryData;
	if(provider) {
		//Used by movie player to provider initial state of ram at startup
		batteryData = provider->LoadBattery(extension);
	} else {
		VirtualFile file = GetBasePath() + extension;
		if(file.IsValid()) {
			file.ReadFile(batteryData);
		}
	}

	if(!batteryData.empty()) {
		shared_ptr<IBatteryRecorder> recorder = _recorder.lock();
		if(recorder) {
			//Used by movies to record initial state of battery-backed ram at power on
			recorder->OnLoadBattery(extension, batteryData);
		}
	}

	return batteryData;
}

void BatteryManager::LoadBattery(string extension, uint8_t* data, uint32_t length)
{
	vector<uint8_t> batteryData = LoadBattery(extension);
	memset(data, 0, length);
	memcpy(data, batteryData.data(), std::min((uint32_t)batteryData.size(), length));
}