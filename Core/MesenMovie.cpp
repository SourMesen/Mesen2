#include "stdafx.h"
#include "Utilities/ZipReader.h"
#include "Utilities/StringUtilities.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/VirtualFile.h"
#include "MesenMovie.h"
#include "MessageManager.h"
#include "SNES/ControlManager.h"
#include "BaseControlDevice.h"
#include "Emulator.h"
#include "EmuSettings.h"
#include "SaveStateManager.h"
#include "MovieTypes.h"
#include "MovieManager.h"
#include "NotificationManager.h"
#include "BatteryManager.h"
#include "CheatManager.h"

MesenMovie::MesenMovie(shared_ptr<Emulator> emu, bool forTest)
{
	_emu = emu;
	_forTest = forTest;
}

MesenMovie::~MesenMovie()
{
	Stop();
}

void MesenMovie::Stop()
{
	if(_playing) {
		if(!_forTest) {
			MessageManager::DisplayMessage("Movies", "MovieEnded");
		}

		if(_emu->GetSettings()->GetPreferences().PauseOnMovieEnd) {
			_emu->Pause();
		}

		_emu->GetCheatManager()->SetCheats(_originalCheats);

		_playing = false;
	}
	_emu->GetControlManager()->UnregisterInputProvider(this);
}

bool MesenMovie::SetInput(BaseControlDevice *device)
{
	uint32_t inputRowIndex = _emu->GetControlManager()->GetPollCounter();
	_lastPollCounter = inputRowIndex;

	if(_inputData.size() > inputRowIndex && _inputData[inputRowIndex].size() > _deviceIndex) {
		device->SetTextState(_inputData[inputRowIndex][_deviceIndex]);

		_deviceIndex++;
		if(_deviceIndex >= _inputData[inputRowIndex].size()) {
			//Move to the next frame's data
			_deviceIndex = 0;
		}
	} else {
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
		_emu->GetControlManager()->RegisterInputProvider(this);
		_emu->GetControlManager()->SetPollCounter(_lastPollCounter);
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
	
	_emu->Lock();
		
	_emu->GetBatteryManager()->SetBatteryProvider(shared_from_this());
	_emu->GetNotificationManager()->RegisterNotificationListener(shared_from_this());
	ApplySettings();

	//TODO
	//Disable auto-configure input option (otherwise the movie file's input types are ignored)
	//bool autoConfigureInput = _console->GetSettings()->CheckFlag(EmulationFlags::AutoConfigureInput);
	//_console->GetSettings()->ClearFlags(EmulationFlags::AutoConfigureInput);

	ControlManager *controlManager = _emu->GetControlManager().get();
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

	_originalCheats = _emu->GetCheatManager()->GetCheats();

	controlManager->UpdateControlDevices();
	if(!_forTest) {
		_emu->PowerCycle();
	} else {
		controlManager->RegisterInputProvider(this);
	}

	LoadCheats();	

	stringstream saveStateData;
	if(_reader->GetStream("SaveState.mss", saveStateData)) {
		if(!_emu->GetSaveStateManager()->LoadState(saveStateData, true)) {
			_emu->Resume();
			return false;
		} else {
			_emu->GetControlManager()->SetPollCounter(0);
		}
	}

	_playing = true;

	_emu->Unlock();

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
	EmuSettings* settings = _emu->GetSettings().get();
	EmulationConfig emuConfig = settings->GetEmulationConfig();
	InputConfig inputConfig = settings->GetInputConfig();

	inputConfig.Controllers[0].Type = FromString(LoadString(_settings, MovieKeys::Controller1), ControllerTypeNames, ControllerType::None);
	inputConfig.Controllers[1].Type = FromString(LoadString(_settings, MovieKeys::Controller2), ControllerTypeNames, ControllerType::None);
	inputConfig.Controllers[2].Type = FromString(LoadString(_settings, MovieKeys::Controller3), ControllerTypeNames, ControllerType::None);
	inputConfig.Controllers[3].Type = FromString(LoadString(_settings, MovieKeys::Controller4), ControllerTypeNames, ControllerType::None);
	inputConfig.Controllers[4].Type = FromString(LoadString(_settings, MovieKeys::Controller5), ControllerTypeNames, ControllerType::None);

	emuConfig.Region = FromString(LoadString(_settings, MovieKeys::Region), ConsoleRegionNames, ConsoleRegion::Ntsc);
	if(!_forTest) {
		emuConfig.RamPowerOnState = FromString(LoadString(_settings, MovieKeys::RamPowerOnState), RamStateNames, RamState::AllOnes);
	}
	emuConfig.PpuExtraScanlinesAfterNmi = LoadInt(_settings, MovieKeys::ExtraScanlinesAfterNmi);
	emuConfig.PpuExtraScanlinesBeforeNmi = LoadInt(_settings, MovieKeys::ExtraScanlinesBeforeNmi);
	emuConfig.GsuClockSpeed = LoadInt(_settings, MovieKeys::GsuClockSpeed, 100);

	settings->SetEmulationConfig(emuConfig);
	settings->SetInputConfig(inputConfig);
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
		code.Address = HexUtilities::FromHex(data[0]);
		code.Value = HexUtilities::FromHex(data[1]);
		return true;
	} else {
		MessageManager::Log("[Movie] Invalid cheat definition: " + cheatData);
	}
	return false;
}
