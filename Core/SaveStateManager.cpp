#include "stdafx.h"
#include "../Utilities/FolderUtilities.h"
#include "../Utilities/ZipWriter.h"
#include "../Utilities/ZipReader.h"
#include "../Utilities/PNGHelper.h"
#include "SaveStateManager.h"
#include "MessageManager.h"
#include "Console.h"
#include "EmuSettings.h"
#include "VideoDecoder.h"
#include "BaseCartridge.h"
#include "MovieManager.h"
#include "EventType.h"
#include "Debugger.h"
#include "GameClient.h"
#include "Ppu.h"
#include "DefaultVideoFilter.h"

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

void SaveStateManager::SelectSaveSlot(int slotIndex)
{
	_lastIndex = slotIndex;
	MessageManager::DisplayMessage("SaveStates", "SaveStateSlotSelected", std::to_string(_lastIndex));
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

	#ifndef LIBRETRO
	SaveScreenshotData(stream);
	#endif

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
		_console->Lock();
		SaveState(file);
		_console->Unlock();
		file.close();

		shared_ptr<Debugger> debugger = _console->GetDebugger(false);
		if(debugger) {
			debugger->ProcessEvent(EventType::StateSaved);
		}
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

void SaveStateManager::SaveScreenshotData(ostream& stream)
{
	bool isHighRes = _console->GetPpu()->IsHighResOutput();
	uint32_t height = isHighRes ? 478 : 239;
	uint32_t width = isHighRes ? 512 : 256;

	stream.write((char*)&width, sizeof(uint32_t));
	stream.write((char*)&height, sizeof(uint32_t));

	unsigned long compressedSize = compressBound(512*478*2);
	vector<uint8_t> compressedData(compressedSize, 0);
	compress2(compressedData.data(), &compressedSize, (const unsigned char*)_console->GetPpu()->GetScreenBuffer(), width*height*2, MZ_DEFAULT_LEVEL);

	uint32_t screenshotLength = (uint32_t)compressedSize;
	stream.write((char*)&screenshotLength, sizeof(uint32_t));
	stream.write((char*)compressedData.data(), screenshotLength);
}

bool SaveStateManager::GetScreenshotData(vector<uint8_t>& out, uint32_t &width, uint32_t &height, istream& stream)
{
	stream.read((char*)&width, sizeof(uint32_t));
	stream.read((char*)&height, sizeof(uint32_t));

	uint32_t screenshotLength = 0;
	stream.read((char*)&screenshotLength, sizeof(uint32_t));

	vector<uint8_t> compressedData(screenshotLength, 0);
	stream.read((char*)compressedData.data(), screenshotLength);

	out = vector<uint8_t>(width * height * 2, 0);
	unsigned long decompSize = width * height * 2;
	if(uncompress(out.data(), &decompSize, compressedData.data(), (unsigned long)compressedData.size()) == MZ_OK) {
		return true;
	}
	return false;
}

bool SaveStateManager::LoadState(istream &stream, bool hashCheckRequired)
{
	if(GameClient::Connected()) {
		MessageManager::DisplayMessage("Netplay", "NetplayNotAllowed");
		return false;
	}

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
		if(fileFormatVersion <= 5) {
			MessageManager::DisplayMessage("SaveStates", "SaveStateIncompatibleVersion");
			return false;
		} else {
			char hash[41] = {};
			stream.read(hash, 40);

			if(fileFormatVersion >= 7) {
				#ifndef LIBRETRO
				vector<uint8_t> frameData;
				uint32_t width = 0;
				uint32_t height = 0;
				if(GetScreenshotData(frameData, width, height, stream)) {
					_console->GetVideoDecoder()->UpdateFrameSync((uint16_t*)frameData.data(), width, height, 0, true);
				}
				#endif
			}

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
		_console->GetMovieManager()->Stop();

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
		_console->Lock();
		result = LoadState(file, hashCheckRequired);
		_console->Unlock();
		file.close();

		if(result) {
			shared_ptr<Debugger> debugger = _console->GetDebugger(false);
			if(debugger) {
				debugger->ProcessEvent(EventType::StateLoaded);
			}
		}
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
#ifndef LIBRETRO
	//Don't do this for libretro core
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
#endif
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
	} catch(std::exception&) { 
		_console->Stop(true);
	}
	_console->Unlock();
}

int32_t SaveStateManager::GetSaveStatePreview(string saveStatePath, uint8_t* pngData)
{
	ifstream stream(saveStatePath, ios::binary);

	if(!stream) {
		return -1;
	}

	char header[3];
	stream.read(header, 3);
	if(memcmp(header, "MSS", 3) == 0) {
		uint32_t emuVersion = 0;

		stream.read((char*)&emuVersion, sizeof(emuVersion));
		if(emuVersion > _console->GetSettings()->GetVersion()) {
			return -1;
		}

		uint32_t fileFormatVersion = 0;
		stream.read((char*)&fileFormatVersion, sizeof(fileFormatVersion));
		if(fileFormatVersion <= 6) {
			return -1;
		}

		//Skip some header fields
		stream.seekg(40, ios::cur);

		vector<uint8_t> frameData;
		uint32_t width = 0;
		uint32_t height = 0;
		if(GetScreenshotData(frameData, width, height, stream)) {
			FrameInfo baseFrameInfo;
			baseFrameInfo.Width = width;
			baseFrameInfo.Height = height;
			
			DefaultVideoFilter filter(_console);
			filter.SetBaseFrameInfo(baseFrameInfo);
			FrameInfo frameInfo = filter.GetFrameInfo();
			filter.SendFrame((uint16_t*)frameData.data(), 0);

			std::stringstream pngStream;
			PNGHelper::WritePNG(pngStream, filter.GetOutputBuffer(), frameInfo.Width, frameInfo.Height);

			string data = pngStream.str();
			memcpy(pngData, data.c_str(), data.size());

			return (int32_t)frameData.size();
		}
	}
	return -1;
}