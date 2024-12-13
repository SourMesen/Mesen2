#include "pch.h"
#include "Shared/Movies/MesenMovie.h"
#include "Shared/Movies/MovieTypes.h"
#include "Shared/Movies/MovieManager.h"
#include "Shared/MessageManager.h"
#include "Shared/BaseControlManager.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/SaveStateManager.h"
#include "Shared/NotificationManager.h"
#include "Shared/BatteryManager.h"
#include "Shared/CheatManager.h"
#include "Utilities/ZipReader.h"
#include "Utilities/StringUtilities.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/VirtualFile.h"
#include "Utilities/magic_enum.hpp"
#include "Utilities/Serializer.h"

MesenMovie::MesenMovie(Emulator* emu, bool forTest)
{
	_emu = emu;
	_forTest = forTest;
}

MesenMovie::~MesenMovie()
{
	_emu->UnregisterInputProvider(this);
}

void MesenMovie::Stop()
{
	if(_playing) {
		bool isEndOfMovie = _lastPollCounter >= _inputData.size();

		if(!_forTest) {
			MessageManager::DisplayMessage("Movies", isEndOfMovie ? "MovieEnded" : "MovieStopped");
		}

		EmuSettings* settings = _emu->GetSettings();
		if(isEndOfMovie && settings->GetPreferences().PauseOnMovieEnd) {
			_emu->PauseOnNextFrame();
		}

		_emu->GetCheatManager()->SetCheats(_originalCheats);

		Serializer backup(0, false);
		backup.LoadFrom(_emuSettingsBackup);
		backup.Stream(*settings, "", -1);

		_playing = false;
	}

	_emu->UnregisterInputProvider(this);
	_controlManager = nullptr;
}

bool MesenMovie::SetInput(BaseControlDevice *device)
{
	uint32_t inputRowIndex = _controlManager->GetPollCounter();

	if(_lastPollCounter != inputRowIndex) {
		_lastPollCounter = inputRowIndex;
		assert(_deviceIndex == 0);
		_deviceIndex = 0;
	}

	if(_inputData.size() > inputRowIndex && _inputData[inputRowIndex].size() > _deviceIndex) {
		device->SetTextState(_inputData[inputRowIndex][_deviceIndex]);

		_deviceIndex++;
		if(_deviceIndex >= _inputData[inputRowIndex].size()) {
			//Move to the next frame's data
			_deviceIndex = 0;
		}
	} else {
		//End of input data reached (movie end)
		_emu->GetMovieManager()->Stop();
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

void MesenMovie::ProcessNotification(ConsoleNotificationType type, void* parameter)
{
	if(type == ConsoleNotificationType::GameLoaded) {
		_emu->RegisterInputProvider(this);
		shared_ptr<IConsole> console = _emu->GetConsole();
		if(console) {
			console->GetControlManager()->SetPollCounter(_lastPollCounter);
		}
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
	
	string version = LoadString(_settings, MovieKeys::MesenVersion);
	if(version.size() < 2 || version.substr(0, 2) == "0." || version.substr(0, 2) == "1.") {
		//Prevent loading movies from Mesen/Mesen-S version 0.x.x or 1.x.x
		MessageManager::DisplayMessage("Movies", "MovieIncompatibleVersion");
		return false;
	}

	if(LoadInt(_settings, MovieKeys::MovieFormatVersion, 0) < 2) {
		MessageManager::DisplayMessage("Movies", "MovieIncompatibleVersion");
		return false;
	}

	auto emuLock = _emu->AcquireLock(false);

	if(!ApplySettings(settingsData)) {
		return false;
	}
	
	_emu->GetBatteryManager()->SetBatteryProvider(shared_from_this());
	_emu->GetNotificationManager()->RegisterNotificationListener(shared_from_this());

	_emu->PowerCycle();

	//Re-apply settings - power cycling can alter some (e.g auto-configure input types, etc.)
	ApplySettings(settingsData);

	_originalCheats = _emu->GetCheatManager()->GetCheats();

	_controlManager = _emu->GetConsole()->GetControlManager();

	LoadCheats();	

	stringstream saveStateData;
	if(_reader->GetStream("SaveState.mss", saveStateData)) {
		if(!_emu->GetSaveStateManager()->LoadState(saveStateData)) {
			return false;
		}
	}

	_controlManager->UpdateControlDevices();
	_controlManager->SetPollCounter(0);
	_playing = true;

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

bool MesenMovie::ApplySettings(istream& settingsData)
{
	EmuSettings* settings = _emu->GetSettings();
	
	settingsData.clear();
	settingsData.seekg(0, std::ios::beg);
	Serializer s(0, false, SerializeFormat::Text);
	s.LoadFrom(settingsData);

	ConsoleType consoleType = {};
	s.Stream(consoleType, "emu.consoleType", -1);

	if(consoleType != _emu->GetConsoleType()) {
		MessageManager::DisplayMessage("Movies", "MovieIncorrectConsole", string(magic_enum::enum_name<ConsoleType>(consoleType)));
		return false;
	}

	Serializer backup(0, true);
	backup.Stream(*settings, "", -1);
	backup.SaveTo(_emuSettingsBackup);

	//Load settings
	s.Stream(*settings, "", -1);

	return true;
}

uint32_t MesenMovie::LoadInt(std::unordered_map<string, string> &settings, string name, uint32_t defaultValue)
{
	auto result = settings.find(name);
	if(result != settings.end()) {
		try {
			return (uint32_t)std::stoul(result->second);
		} catch(std::exception&) {
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

void MesenMovie::LoadCheats()
{
	vector<CheatCode> cheats;
	for(string cheatData : _cheats) {
		CheatCode code;
		if(LoadCheat(cheatData, code)) {
			cheats.push_back(code);
		}
	}
	_emu->GetCheatManager()->SetCheats(cheats);
}

bool MesenMovie::LoadCheat(string cheatData, CheatCode &code)
{
	vector<string> data = StringUtilities::Split(cheatData, ' ');

	if(data.size() == 2) {
		auto cheatType = magic_enum::enum_cast<CheatType>(data[0]);
		if(cheatType.has_value() && data[1].size() <= 15) {
			code.Type = cheatType.value();
			memcpy(code.Code, data[1].c_str(), data[1].size() + 1);
			return true;
		}
	}
	
	MessageManager::Log("[Movie] Invalid cheat definition: " + cheatData);
	return false;
}
