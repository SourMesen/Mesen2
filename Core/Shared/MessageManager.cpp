#include "pch.h"
#include "Shared/MessageManager.h"

std::unordered_map<string, string> MessageManager::_enResources = {
	{ "Cheats", u8"Cheats" },
	{ "Debug", u8"Debug" },
	{ "EmulationSpeed", u8"Emulation Speed" },
	{ "ClockRate", u8"Clock Rate" },
	{ "Error", u8"Error" },
	{ "GameInfo", u8"Game Info" },
	{ "GameLoaded", u8"Game loaded" },
	{ "Input", u8"Input" },
	{ "Patch", u8"Patch" },
	{ "Movies", u8"Movies" },
	{ "NetPlay", u8"Net Play" },
	{ "Overclock", u8"Overclock" },
	{ "Region", u8"Region" },
	{ "SaveStates", u8"Save States" },
	{ "ScreenshotSaved", u8"Screenshot Saved" },
	{ "SoundRecorder", u8"Sound Recorder" },
	{ "Test", u8"Test" },
	{ "VideoRecorder", u8"Video Recorder" },

	{ "ApplyingPatch", u8"Applying patch: %1" },
	{ "CheatApplied", u8"1 cheat applied." },
	{ "CheatsApplied", u8"%1 cheats applied." },
	{ "CheatsDisabled", u8"All cheats disabled." },
	{ "CoinInsertedSlot", u8"Coin inserted (slot %1)" },
	{ "ConnectedToServer", u8"Connected to server." },
	{ "ConnectionLost", u8"Connection to server lost." },
	{ "CouldNotConnect", u8"Could not connect to the server." },
	{ "CouldNotInitializeAudioSystem", u8"Could not initialize audio system" },
	{ "CouldNotFindRom", u8"Could not find matching game ROM. (%1)" },
	{ "CouldNotWriteToFile", u8"Could not write to file: %1" },
	{ "CouldNotLoadFile", u8"Could not load file: %1" },
	{ "EmulationMaximumSpeed", u8"Maximum speed" },
	{ "EmulationSpeedPercent", u8"%1%" },
	{ "FdsDiskInserted", u8"Disk %1 Side %2 inserted." },
	{ "Frame", u8"Frame" },
	{ "GameCrash", u8"Game has crashed (%1)" },
	{ "KeyboardModeDisabled", u8"Keyboard mode disabled." },
	{ "KeyboardModeEnabled", u8"Keyboard connected - shortcut keys disabled." },
	{ "Lag", u8"Lag" },
	{ "Mapper", u8"Mapper: %1, SubMapper: %2" },
	{ "MovieEnded", u8"Movie ended." },
	{ "MovieStopped", u8"Movie stopped." },
	{ "MovieInvalid", u8"Invalid movie file." },
	{ "MovieMissingRom", u8"Missing ROM required (%1) to play movie." },
	{ "MovieNewerVersion", u8"Cannot load movies created by a more recent version of Mesen. Please download the latest version." },
	{ "MovieIncompatibleVersion", u8"This movie is incompatible with this version of Mesen." },
	{ "MovieIncorrectConsole", u8"This movie was recorded on another console (%1) and can't be loaded." },
	{ "MoviePlaying", u8"Playing movie: %1" },
	{ "MovieRecordingTo", u8"Recording to: %1" },
	{ "MovieSaved", u8"Movie saved to file: %1" },
	{ "NetplayVersionMismatch", u8"Netplay client is not running the same version of Mesen and has been disconnected." },
	{ "NetplayNotAllowed", u8"This action is not allowed while connected to a server." },
	{ "OverclockEnabled", u8"Overclocking enabled." },
	{ "OverclockDisabled", u8"Overclocking disabled." },
	{ "PrgSizeWarning", u8"PRG size is smaller than 32kb" },
	{ "SaveStateEmpty", u8"Slot is empty." },
	{ "SaveStateIncompatibleVersion", u8"Save state is incompatible with this version of Mesen." },
	{ "SaveStateInvalidFile", u8"Invalid save state file." },
	{ "SaveStateWrongSystem", u8"Error: State cannot be loaded (wrong console type)" },
	{ "SaveStateLoaded", u8"State #%1 loaded." },
	{ "SaveStateLoadedFile", u8"State loaded: %1" },
	{ "SaveStateSavedFile", u8"State saved: %1" },
	{ "SaveStateMissingRom", u8"Missing ROM required (%1) to load save state." },
	{ "SaveStateNewerVersion", u8"Cannot load save states created by a more recent version of Mesen. Please download the latest version." },
	{ "SaveStateSaved", u8"State #%1 saved." },
	{ "SaveStateSlotSelected", u8"Slot #%1 selected." },
	{ "ScanlineTimingWarning", u8"PPU timing has been changed." },
	{ "ServerStarted", u8"Server started (Port: %1)" },
	{ "ServerStopped", u8"Server stopped" },
	{ "SoundRecorderStarted", u8"Recording to: %1" },
	{ "SoundRecorderStopped", u8"Recording saved to: %1" },
	{ "TestFileSavedTo", u8"Test file saved to: %1" },
	{ "UnexpectedError", u8"Unexpected error: %1" },
	{ "UnsupportedMapper", u8"Unsupported mapper (%1), cannot load game." },
	{ "VideoRecorderStarted", u8"Recording to: %1" },
	{ "VideoRecorderStopped", u8"Recording saved to: %1" },
};

std::list<string> MessageManager::_log;
SimpleLock MessageManager::_logLock;
SimpleLock MessageManager::_messageLock;
bool MessageManager::_osdEnabled = true;
IMessageManager* MessageManager::_messageManager = nullptr;

void MessageManager::RegisterMessageManager(IMessageManager* messageManager)
{
	auto lock = _messageLock.AcquireSafe();
	if(MessageManager::_messageManager == nullptr) {
		MessageManager::_messageManager = messageManager;
	}
}

void MessageManager::UnregisterMessageManager(IMessageManager* messageManager)
{
	auto lock = _messageLock.AcquireSafe();
	if(MessageManager::_messageManager == messageManager) {
		MessageManager::_messageManager = nullptr;
	}
}

void MessageManager::SetOsdState(bool enabled)
{
	_osdEnabled = enabled;
}

string MessageManager::Localize(string key)
{
	std::unordered_map<string, string> *resources = &_enResources;
	/*switch(EmulationSettings::GetDisplayLanguage()) {
		case Language::English: resources = &_enResources; break;
		case Language::French: resources = &_frResources; break;
		case Language::Japanese: resources = &_jaResources; break;
		case Language::Russian: resources = &_ruResources; break;
		case Language::Spanish: resources = &_esResources; break;
		case Language::Ukrainian: resources = &_ukResources; break;
		case Language::Portuguese: resources = &_ptResources; break;
		case Language::Catalan: resources = &_caResources; break;
		case Language::Chinese: resources = &_zhResources; break;
	}*/
	if(resources) {
		if(resources->find(key) != resources->end()) {
			return (*resources)[key];
		}/* else if(EmulationSettings::GetDisplayLanguage() != Language::English) {
			//Fallback on English if resource key is missing another language
			resources = &_enResources;
			if(resources->find(key) != resources->end()) {
				return (*resources)[key];
			}
		}*/
	}

	return key;
}

void MessageManager::DisplayMessage(string title, string message, string param1, string param2)
{
	if(MessageManager::_messageManager) {
		auto lock = _messageLock.AcquireSafe();
		if(!MessageManager::_messageManager) {
			return;
		}

		title = Localize(title);
		message = Localize(message);

		size_t startPos = message.find(u8"%1");
		if(startPos != std::string::npos) {
			message.replace(startPos, 2, param1);
		}

		startPos = message.find(u8"%2");
		if(startPos != std::string::npos) {
			message.replace(startPos, 2, param2);
		}

		if(_osdEnabled) {
			MessageManager::_messageManager->DisplayMessage(title, message);
		} else {
			MessageManager::Log("[" + title + "] " + message);
		}
	}
}

void MessageManager::Log(string message)
{
	auto lock = _logLock.AcquireSafe();
	if(message.empty()) {
		message = "------------------------------------------------------";
	}
	if(_log.size() >= 1000) {
		_log.pop_front();
	}
	_log.push_back(message);
}

void MessageManager::ClearLog()
{
	auto lock = _logLock.AcquireSafe();
	_log.clear();
}

string MessageManager::GetLog()
{
	auto lock = _logLock.AcquireSafe();
	stringstream ss;
	for(string &msg : _log) {
		ss << msg << "\n";
	}
	return ss.str();
}
