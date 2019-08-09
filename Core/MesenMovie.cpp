#include "stdafx.h"
#include "../Utilities/ZipReader.h"
#include "../Utilities/StringUtilities.h"
#include "../Utilities/HexUtilities.h"
#include "../Utilities/VirtualFile.h"
#include "MesenMovie.h"
#include "MessageManager.h"
#include "ControlManager.h"
#include "BaseControlDevice.h"
#include "Console.h"
#include "EmuSettings.h"
#include "SaveStateManager.h"
#include "MovieTypes.h"
#include "MovieManager.h"
#include "NotificationManager.h"
#include "BatteryManager.h"

MesenMovie::MesenMovie(shared_ptr<Console> console)
{
	_console = console;
}

MesenMovie::~MesenMovie()
{
	Stop();
}

void MesenMovie::Stop()
{
	if(_playing) {
		MessageManager::DisplayMessage("Movies", "MovieEnded");

		if(_console->GetSettings()->GetPreferences().PauseOnMovieEnd) {
			_console->Pause();
		}

		_playing = false;
	}
	_console->GetControlManager()->UnregisterInputProvider(this);
}

bool MesenMovie::SetInput(BaseControlDevice *device)
{
	uint32_t inputRowIndex = _console->GetControlManager()->GetPollCounter();
	_lastPollCounter = inputRowIndex;

	if(_inputData.size() > inputRowIndex && _inputData[inputRowIndex].size() > _deviceIndex) {
		device->SetTextState(_inputData[inputRowIndex][_deviceIndex]);

		_deviceIndex++;
		if(_deviceIndex >= _inputData[inputRowIndex].size()) {
			//Move to the next frame's data
			_deviceIndex = 0;
		}
	} else {
		MovieManager::Stop();
	}
	return true;
}

bool MesenMovie::IsPlaying()
{
	return _playing;
}

vector<uint8_t> MesenMovie::LoadBattery(string extension)
{
	vector<uint8_t> batteryData;
	_reader->ExtractFile("Battery" + extension, batteryData);
	return batteryData;
}

void MesenMovie::ProcessNotification(ConsoleNotificationType type, void * parameter)
{
	if(type == ConsoleNotificationType::GameLoaded) {
		_console->GetControlManager()->RegisterInputProvider(this);
		_console->GetControlManager()->SetPollCounter(_lastPollCounter + 1);
	}
}

bool MesenMovie::Play(VirtualFile &file)
{
	_movieFile = file;

	std::stringstream ss;
	file.ReadFile(ss);

	_reader.reset(new ZipReader());
	_reader->LoadArchive(ss);

	stringstream settingsData, inputData;
	if(!_reader->GetStream("GameSettings.txt", settingsData)) {
		MessageManager::Log("[Movie] File not found: GameSettings.txt");
		return false;
	}
	if(!_reader->GetStream("Input.txt", inputData)) {
		MessageManager::Log("[Movie] File not found: Input.txt");
		return false;
	}

	while(inputData) {
		string line;
		std::getline(inputData, line);
		if(line.substr(0, 1) == "|") {
			_inputData.push_back(StringUtilities::Split(line.substr(1), '|'));
		}
	}

	_deviceIndex = 0;

	ParseSettings(settingsData);
	
	_console->Lock();
		
	_console->GetBatteryManager()->SetBatteryProvider(shared_from_this());
	_console->GetNotificationManager()->RegisterNotificationListener(shared_from_this());
	ApplySettings();

	//TODO
	//Disable auto-configure input option (otherwise the movie file's input types are ignored)
	//bool autoConfigureInput = _console->GetSettings()->CheckFlag(EmulationFlags::AutoConfigureInput);
	//_console->GetSettings()->ClearFlags(EmulationFlags::AutoConfigureInput);

	ControlManager *controlManager = _console->GetControlManager().get();
	if(controlManager) {
		//ControlManager can be empty if no game is loaded
		controlManager->SetPollCounter(0);
	}

	//bool gameLoaded = LoadGame();
	//TODO
	//_console->GetSettings()->SetFlagState(EmulationFlags::AutoConfigureInput, autoConfigureInput);

	/*if(!gameLoaded) {
		_console->Unlock();
		return false;
	}*/

	_console->PowerCycle();

	stringstream saveStateData;
	if(_reader->GetStream("SaveState.mss", saveStateData)) {
		if(!_console->GetSaveStateManager()->LoadState(saveStateData, true)) {
			_console->Resume();
			return false;
		} else {
			_console->GetControlManager()->SetPollCounter(0);
		}
	}

	_playing = true;

	_console->Unlock();

	return true;
}

template<typename T>
T FromString(string name, const vector<string> &enumNames, T defaultValue)
{
	for(size_t i = 0; i < enumNames.size(); i++) {
		if(name == enumNames[i]) {
			return (T)i;
		}
	}
	return defaultValue;
}

void MesenMovie::ParseSettings(stringstream &data)
{
	while(!data.eof()) {
		string line;
		std::getline(data, line);

		if(!line.empty()) {
			size_t index = line.find_first_of(' ');
			if(index != string::npos) {
				string name = line.substr(0, index);
				string value = line.substr(index + 1);

				if(name == "Cheat") {
					_cheats.push_back(value);
				} else {
					_settings[name] = value;
				}
			}
		}
	}
}

bool MesenMovie::LoadGame()
{
	/*string mesenVersion = LoadString(_settings, MovieKeys::MesenVersion);
	string gameFile = LoadString(_settings, MovieKeys::GameFile);
	string sha1Hash = LoadString(_settings, MovieKeys::Sha1);
	//string patchFile = LoadString(_settings, MovieKeys::PatchFile);
	//string patchFileSha1 = LoadString(_settings, MovieKeys::PatchFileSha1);
	//string patchedRomSha1 = LoadString(_settings, MovieKeys::PatchedRomSha1);

	if(_console->GetSettings()->CheckFlag(EmulationFlags::AllowMismatchingSaveState) && _console->GetRomInfo().RomName == gameFile) {
		//Loaded game has the right name, and we don't want to validate the hash values
		_console->PowerCycle();
		return true;
	}

	HashInfo hashInfo;
	hashInfo.Sha1 = sha1Hash;

	VirtualFile romFile = _console->FindMatchingRom(gameFile, hashInfo);
	bool gameLoaded = false;
	if(romFile.IsValid()) {
		VirtualFile patchFile(_movieFile.GetFilePath(), "PatchData.dat");
		if(patchFile.IsValid()) {
			gameLoaded = _console->Initialize(romFile, patchFile);
		} else {
			gameLoaded = _console->Initialize(romFile);
		}
	}

	return gameLoaded;*/
	return true;
}

void MesenMovie::ApplySettings()
{
	EmuSettings* settings = _console->GetSettings().get();
	EmulationConfig emuConfig = settings->GetEmulationConfig();
	InputConfig inputConfig = settings->GetInputConfig();

	inputConfig.Controllers[0].Type = FromString(LoadString(_settings, MovieKeys::Controller1), ControllerTypeNames, ControllerType::None);
	inputConfig.Controllers[1].Type = FromString(LoadString(_settings, MovieKeys::Controller2), ControllerTypeNames, ControllerType::None);
	inputConfig.Controllers[2].Type = FromString(LoadString(_settings, MovieKeys::Controller3), ControllerTypeNames, ControllerType::None);
	inputConfig.Controllers[3].Type = FromString(LoadString(_settings, MovieKeys::Controller4), ControllerTypeNames, ControllerType::None);
	inputConfig.Controllers[4].Type = FromString(LoadString(_settings, MovieKeys::Controller5), ControllerTypeNames, ControllerType::None);

	emuConfig.Region = FromString(LoadString(_settings, MovieKeys::Region), ConsoleRegionNames, ConsoleRegion::Ntsc);
	emuConfig.RamPowerOnState = FromString(LoadString(_settings, MovieKeys::RamPowerOnState), RamStateNames, RamState::AllOnes);
	emuConfig.PpuExtraScanlinesAfterNmi = LoadInt(_settings, MovieKeys::ExtraScanlinesAfterNmi);
	emuConfig.PpuExtraScanlinesBeforeNmi = LoadInt(_settings, MovieKeys::ExtraScanlinesBeforeNmi);

	settings->SetEmulationConfig(emuConfig);
	settings->SetInputConfig(inputConfig);
}

uint32_t MesenMovie::LoadInt(std::unordered_map<string, string> &settings, string name, uint32_t defaultValue)
{
	auto result = settings.find(name);
	if(result != settings.end()) {
		try {
			return (uint32_t)std::stoul(result->second);
		} catch(std::exception ex) {
			MessageManager::Log("[Movies] Invalid value for tag: " + name);
			return defaultValue;
		}
	} else {
		return defaultValue;
	}
}

bool MesenMovie::LoadBool(std::unordered_map<string, string> &settings, string name)
{
	auto result = settings.find(name);
	if(result != settings.end()) {
		if(result->second == "true") {
			return true;
		} else if(result->second == "false") {
			return false;
		} else {			
			MessageManager::Log("[Movies] Invalid value for tag: " + name);
			return false;
		}
	} else {
		return false;
	}
}

string MesenMovie::LoadString(std::unordered_map<string, string> &settings, string name)
{
	auto result = settings.find(name);
	if(result != settings.end()) {
		return result->second;
	} else {
		return "";
	}
}
