#include "stdafx.h"
#include "../Utilities/FolderUtilities.h"
#include "../Utilities/ZipWriter.h"
#include "../Utilities/ZipReader.h"
#include "SaveStateManager.h"
#include "MessageManager.h"
#include "Console.h"
#include "EmuSettings.h"
#include "VideoDecoder.h"
#include "BaseCartridge.h"

SaveStateManager::SaveStateManager(shared_ptr<Console> console)
{
	_console = console;
	_lastIndex = 1;
}

string SaveStateManager::GetStateFilepath(int stateIndex)
{
	string romFile = _console->GetRomInfo().RomFile.GetFileName();
	string folder = FolderUtilities::GetSaveStateFolder();
	string filename = FolderUtilities::GetFilename(romFile, false) + "_" + std::to_string(stateIndex) + ".mss";
	return FolderUtilities::CombinePath(folder, filename);
}

uint64_t SaveStateManager::GetStateInfo(int stateIndex)
{
	string filepath = SaveStateManager::GetStateFilepath(stateIndex);
	ifstream file(filepath, ios::in | ios::binary);

	if(file) {
		file.close();
		return FolderUtilities::GetFileModificationTime(filepath);
	}
	return 0;
}

void SaveStateManager::MoveToNextSlot()
{
	_lastIndex = (_lastIndex % MaxIndex) + 1;
	MessageManager::DisplayMessage("SaveStates", "SaveStateSlotSelected", std::to_string(_lastIndex));
}

void SaveStateManager::MoveToPreviousSlot()
{
	_lastIndex = (_lastIndex == 1 ? SaveStateManager::MaxIndex : (_lastIndex - 1));
	MessageManager::DisplayMessage("SaveStates", "SaveStateSlotSelected", std::to_string(_lastIndex));
}

void SaveStateManager::SaveState()
{
	SaveState(_lastIndex);
}

bool SaveStateManager::LoadState()
{
	return LoadState(_lastIndex);
}

void SaveStateManager::GetSaveStateHeader(ostream &stream)
{
	uint32_t emuVersion = _console->GetSettings()->GetVersion();
	uint32_t formatVersion = SaveStateManager::FileFormatVersion;
	stream.write("MSS", 3);
	stream.write((char*)&emuVersion, sizeof(emuVersion));
	stream.write((char*)&formatVersion, sizeof(uint32_t));

	string sha1Hash = _console->GetCartridge()->GetSha1Hash();
	stream.write(sha1Hash.c_str(), sha1Hash.size());

	RomInfo romInfo = _console->GetCartridge()->GetRomInfo();
	string romName = FolderUtilities::GetFilename(romInfo.RomFile.GetFileName(), true);
	uint32_t nameLength = (uint32_t)romName.size();
	stream.write((char*)&nameLength, sizeof(uint32_t));
	stream.write(romName.c_str(), romName.size());
}

void SaveStateManager::SaveState(ostream &stream)
{
	GetSaveStateHeader(stream);
	_console->Serialize(stream);
}

bool SaveStateManager::SaveState(string filepath)
{
	ofstream file(filepath, ios::out | ios::binary);

	if(file) {
		auto lock = _console->AcquireLock();
		SaveState(file);
		file.close();

		//TODO LUA
		/*shared_ptr<Debugger> debugger = _console->GetDebugger(false);
		if(debugger) {
			debugger->ProcessEvent(EventType::StateSaved);
		}*/
		return true;
	}
	return false;
}

void SaveStateManager::SaveState(int stateIndex, bool displayMessage)
{
	string filepath = SaveStateManager::GetStateFilepath(stateIndex);
	if(SaveState(filepath)) {
		if(displayMessage) {
			MessageManager::DisplayMessage("SaveStates", "SaveStateSaved", std::to_string(stateIndex));
		}
	}
}

bool SaveStateManager::LoadState(istream &stream, bool hashCheckRequired)
{
	char header[3];
	stream.read(header, 3);
	if(memcmp(header, "MSS", 3) == 0) {
		uint32_t emuVersion, fileFormatVersion;

		stream.read((char*)&emuVersion, sizeof(emuVersion));
		if(emuVersion > _console->GetSettings()->GetVersion()) {
			MessageManager::DisplayMessage("SaveStates", "SaveStateNewerVersion");
			return false;
		}

		stream.read((char*)&fileFormatVersion, sizeof(fileFormatVersion));
		if(fileFormatVersion < 2) {
			MessageManager::DisplayMessage("SaveStates", "SaveStateIncompatibleVersion");
			return false;
		} else {
			char hash[41] = {};
			stream.read(hash, 40);

			uint32_t nameLength = 0;
			stream.read((char*)&nameLength, sizeof(uint32_t));
			
			vector<char> nameBuffer(nameLength);
			stream.read(nameBuffer.data(), nameBuffer.size());
			string romName(nameBuffer.data(), nameLength);
			
			shared_ptr<BaseCartridge> cartridge = _console->GetCartridge();
			if(!cartridge /*|| cartridge->GetSha1Hash() != string(hash)*/) {
				//Game isn't loaded, or CRC doesn't match
				//TODO: Try to find and load the game
				return false;
			}
		}

		//Stop any movie that might have been playing/recording if a state is loaded
		//(Note: Loading a state is disabled in the UI while a movie is playing/recording)
		//TODO MovieManager::Stop();

		_console->Deserialize(stream, fileFormatVersion);

		return true;
	}
	MessageManager::DisplayMessage("SaveStates", "SaveStateInvalidFile");
	return false;
}

bool SaveStateManager::LoadState(string filepath, bool hashCheckRequired)
{
	ifstream file(filepath, ios::in | ios::binary);
	bool result = false;

	if(file.good()) {
		auto lock = _console->AcquireLock();
		if(LoadState(file, hashCheckRequired)) {
			result = true;
		}
		file.close();

		//TODO LUA
		/*shared_ptr<Debugger> debugger = _console->GetDebugger(false);
		if(debugger) {
			debugger->ProcessEvent(EventType::StateLoaded);
		}*/
	} else {
		MessageManager::DisplayMessage("SaveStates", "SaveStateEmpty");
	}

	return result;
}

bool SaveStateManager::LoadState(int stateIndex)
{
	string filepath = SaveStateManager::GetStateFilepath(stateIndex);
	if(LoadState(filepath, false)) {
		MessageManager::DisplayMessage("SaveStates", "SaveStateLoaded", std::to_string(stateIndex));
		return true;
	}
	return false;
}

void SaveStateManager::SaveRecentGame(string romName, string romPath, string patchPath)
{
	string filename = FolderUtilities::GetFilename(_console->GetRomInfo().RomFile.GetFileName(), false) + ".rgd";
	ZipWriter writer;
	writer.Initialize(FolderUtilities::CombinePath(FolderUtilities::GetRecentGamesFolder(), filename));

	std::stringstream pngStream;
	_console->GetVideoDecoder()->TakeScreenshot(pngStream);
	writer.AddFile(pngStream, "Screenshot.png");

	std::stringstream stateStream;
	SaveStateManager::SaveState(stateStream);
	writer.AddFile(stateStream, "Savestate.mss");

	std::stringstream romInfoStream;
	romInfoStream << romName << std::endl;
	romInfoStream << romPath << std::endl;
	romInfoStream << patchPath << std::endl;
	writer.AddFile(romInfoStream, "RomInfo.txt");
	writer.Save();
}

void SaveStateManager::LoadRecentGame(string filename, bool resetGame)
{
	ZipReader reader;
	reader.LoadArchive(filename);

	stringstream romInfoStream, stateStream;
	reader.GetStream("RomInfo.txt", romInfoStream);
	reader.GetStream("Savestate.mss", stateStream);

	string romName, romPath, patchPath;
	std::getline(romInfoStream, romName);
	std::getline(romInfoStream, romPath);
	std::getline(romInfoStream, patchPath);

	_console->Lock();
	try {
		if(_console->LoadRom(romPath, patchPath)) {
			if(!resetGame) {
				SaveStateManager::LoadState(stateStream, false);
			}
		}
	} catch(std::exception &ex) { 
		_console->Stop(true);
	}
	_console->Unlock();
}
