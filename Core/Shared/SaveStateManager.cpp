#include "pch.h"
#include "Utilities/FolderUtilities.h"
#include "Utilities/ZipWriter.h"
#include "Utilities/ZipReader.h"
#include "Utilities/PNGHelper.h"
#include "Shared/SaveStateManager.h"
#include "Shared/MessageManager.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/Movies/MovieManager.h"
#include "Shared/RenderedFrame.h"
#include "Shared/EventType.h"
#include "Debugger/Debugger.h"
#include "Netplay/GameClient.h"
#include "Shared/Video/VideoDecoder.h"
#include "Shared/Video/VideoRenderer.h"
#include "Shared/Video/BaseVideoFilter.h"

SaveStateManager::SaveStateManager(Emulator* emu)
{
	_emu = emu;
	_lastIndex = 1;
}

string SaveStateManager::GetStateFilepath(int stateIndex)
{
	string romFile = _emu->GetRomInfo().RomFile.GetFileName();
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
	uint32_t emuVersion = _emu->GetSettings()->GetVersion();
	uint32_t formatVersion = SaveStateManager::FileFormatVersion;
	stream.write("MSS", 3);
	WriteValue(stream, emuVersion);
	WriteValue(stream, formatVersion);

	WriteValue(stream, (uint32_t)_emu->GetConsoleType());

	SaveVideoData(stream);

	RomInfo romInfo = _emu->GetRomInfo();
	string romName = FolderUtilities::GetFilename(romInfo.RomFile.GetFileName(), true);
	WriteValue(stream, (uint32_t)romName.size());
	stream.write(romName.c_str(), romName.size());
}

void SaveStateManager::SaveState(ostream &stream)
{
	GetSaveStateHeader(stream);
	_emu->Serialize(stream, false);
}

bool SaveStateManager::SaveState(string filepath, bool showSuccessMessage)
{
	ofstream file(filepath, ios::out | ios::binary);

	if(file) {
		{
			auto lock = _emu->AcquireLock();
			SaveState(file);
			_emu->ProcessEvent(EventType::StateSaved);
		}
		file.close();

		if(showSuccessMessage) {
			MessageManager::DisplayMessage("SaveStates", "SaveStateSavedFile", filepath);
		}
		return true;
	}
	return false;
}

void SaveStateManager::SaveState(int stateIndex, bool displayMessage)
{
	string filepath = SaveStateManager::GetStateFilepath(stateIndex);
	if(SaveState(filepath, false)) {
		if(displayMessage) {
			MessageManager::DisplayMessage("SaveStates", "SaveStateSaved", std::to_string(stateIndex));
		}
	}
}

void SaveStateManager::SaveVideoData(ostream& stream)
{
	PpuFrameInfo frame = _emu->GetPpuFrame();
	WriteValue(stream, frame.FrameBufferSize);
	WriteValue(stream, frame.Width);
	WriteValue(stream, frame.Height);
	WriteValue(stream, (uint32_t)(_emu->GetVideoDecoder()->GetLastFrameScale() * 100));

	unsigned long compressedSize = compressBound(frame.FrameBufferSize);
	vector<uint8_t> compressedData(compressedSize, 0);
	compress2(compressedData.data(), &compressedSize, (const unsigned char*)frame.FrameBuffer, frame.FrameBufferSize, MZ_DEFAULT_LEVEL);

	WriteValue(stream, (uint32_t)compressedSize);
	stream.write((char*)compressedData.data(), (uint32_t)compressedSize);
}

bool SaveStateManager::GetVideoData(vector<uint8_t>& out, RenderedFrame& frame, istream& stream)
{
	uint32_t frameBufferSize = ReadValue(stream);
	frame.Width = ReadValue(stream);
	frame.Height = ReadValue(stream);
	frame.Scale = ReadValue(stream) / 100.0;

	uint32_t compressedSize = ReadValue(stream);
	if(compressedSize > 1024 * 1024 * 2) {
		//Data is larger than 2mb, this is probably invalid
		return false;
	}

	vector<uint8_t> compressedData(compressedSize, 0);
	stream.read((char*)compressedData.data(), compressedSize);

	out = vector<uint8_t>(frameBufferSize, 0);
	unsigned long decompSize = frameBufferSize;
	if(uncompress(out.data(), &decompSize, compressedData.data(), (unsigned long)compressedData.size()) == MZ_OK) {
		return true;
	}
	return false;
}

bool SaveStateManager::LoadState(istream &stream)
{
	if(!_emu->IsRunning()) {
		//Can't load a state if no game is running
		return false;
	} else if(_emu->GetGameClient()->Connected()) {
		MessageManager::DisplayMessage("Netplay", "NetplayNotAllowed");
		return false;
	}

	char header[3];
	stream.read(header, 3);
	if(memcmp(header, "MSS", 3) == 0) {
		uint32_t emuVersion = ReadValue(stream);
		if(emuVersion > _emu->GetSettings()->GetVersion()) {
			MessageManager::DisplayMessage("SaveStates", "SaveStateNewerVersion");
			return false;
		}

		uint32_t fileFormatVersion = ReadValue(stream);
		if(fileFormatVersion < SaveStateManager::MinimumSupportedVersion) {
			MessageManager::DisplayMessage("SaveStates", "SaveStateIncompatibleVersion");
			return false;
		}
		
		if(fileFormatVersion <= 3) {
			//Skip over old SHA1 field
			stream.seekg(40, ios::cur);
		}

		ConsoleType stateConsoleType = (ConsoleType)ReadValue(stream);

		RenderedFrame frame;
		vector<uint8_t> frameData;
		if(GetVideoData(frameData, frame, stream)) {
			frame.FrameBuffer = frameData.data();
		} else {
			MessageManager::DisplayMessage("SaveStates", "SaveStateInvalidFile");
			return false;
		}

		uint32_t nameLength = ReadValue(stream);
			
		vector<char> nameBuffer(nameLength);
		stream.read(nameBuffer.data(), nameBuffer.size());
		string romName(nameBuffer.data(), nameLength);

		DeserializeResult result = _emu->Deserialize(stream, fileFormatVersion, false, stateConsoleType);

		if(result == DeserializeResult::Success) {
			//Stop any movie that might have been playing/recording if a state is loaded
			//(Note: Loading a state is disabled in the UI while a movie is playing/recording)
			_emu->GetMovieManager()->Stop();

			if(_emu->IsPaused() && !_emu->GetVideoRenderer()->IsRecording()) {
				//Only send the saved frame if the emulation is paused and no avi recording is in progress
				//Otherwise the avi recorder will receive an extra frame that has no sound, which will
				//create a video vs audio desync in the avi file.
				_emu->GetVideoDecoder()->UpdateFrame(frame, true, false);
			}
			return true;
		} else if(result == DeserializeResult::SpecificError) {
			return false;
		}
	}

	MessageManager::DisplayMessage("SaveStates", "SaveStateInvalidFile");
	return false;
}

bool SaveStateManager::LoadState(string filepath, bool showSuccessMessage)
{
	ifstream file(filepath, ios::in | ios::binary);
	bool result = false;

	if(file.good()) {
		{
			auto lock = _emu->AcquireLock();
			result = LoadState(file);
			if(result) {
				_emu->ProcessEvent(EventType::StateLoaded);
			}
		}
		file.close();

		if(result) {
			if(showSuccessMessage) {
				MessageManager::DisplayMessage("SaveStates", "SaveStateLoadedFile", filepath);
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
	if(_emu->GetSettings()->CheckFlag(EmulationFlags::ConsoleMode)) {
		//Skip saving the recent game file when running in testrunner/CLI console mode
		return;
	}

	string filename = FolderUtilities::GetFilename(_emu->GetRomInfo().RomFile.GetFileName(), false) + ".rgd";
	ZipWriter writer;
	writer.Initialize(FolderUtilities::CombinePath(FolderUtilities::GetRecentGamesFolder(), filename));

	std::stringstream pngStream;
	_emu->GetVideoDecoder()->TakeScreenshot(pngStream);
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
	VirtualFile file(filename);
	if(!file.IsValid()) {
		MessageManager::DisplayMessage("Error", "CouldNotLoadFile", file.GetFileName());
		return;
	}

	ZipReader reader;
	reader.LoadArchive(filename);

	stringstream romInfoStream, stateStream;
	reader.GetStream("RomInfo.txt", romInfoStream);
	reader.GetStream("Savestate.mss", stateStream);

	string romName, romPath, patchPath;
	std::getline(romInfoStream, romName);
	std::getline(romInfoStream, romPath);
	std::getline(romInfoStream, patchPath);

	try {
		if(_emu->LoadRom(romPath, patchPath)) {
			if(!resetGame) {
				auto lock = _emu->AcquireLock();
				SaveStateManager::LoadState(stateStream);
			}
		}
	} catch(std::exception&) { 
		_emu->Stop(true);
	}
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
		uint32_t emuVersion = ReadValue(stream);
		if(emuVersion > _emu->GetSettings()->GetVersion() || emuVersion <= 0x10000) {
			//Prevent loading files created with a newer version of Mesen or with Mesen 0.9.x or lower.
			return -1;
		}

		uint32_t fileFormatVersion = ReadValue(stream);
		if(fileFormatVersion < SaveStateManager::MinimumSupportedVersion) {
			return -1;
		}

		//Skip console type field
		stream.seekg(4, ios::cur);

		vector<uint8_t> frameData;
		RenderedFrame frame;
		if(GetVideoData(frameData, frame, stream)) {
			FrameInfo baseFrameInfo;
			baseFrameInfo.Width = frame.Width;
			baseFrameInfo.Height = frame.Height;
			
			unique_ptr<BaseVideoFilter> filter(_emu->GetVideoFilter(true));
			filter->SetBaseFrameInfo(baseFrameInfo);
			FrameInfo frameInfo = filter->SendFrame((uint16_t*)frameData.data(), 0, 0, nullptr);

			std::stringstream pngStream;
			PNGHelper::WritePNG(pngStream, filter->GetOutputBuffer(), frameInfo.Width, frameInfo.Height);

			string data = pngStream.str();
			memcpy(pngData, data.c_str(), data.size());

			return (int32_t)frameData.size();
		}
	}
	return -1;
}

void SaveStateManager::WriteValue(ostream& stream, uint32_t value)
{
	stream.put(value & 0xFF);
	stream.put((value >> 8) & 0xFF);
	stream.put((value >> 16) & 0xFF);
	stream.put((value >> 24) & 0xFF);
}

uint32_t SaveStateManager::ReadValue(istream& stream)
{
	char a = 0, b = 0, c = 0, d = 0;
	stream.get(a);
	stream.get(b);
	stream.get(c);
	stream.get(d);
	
	uint32_t result = (uint8_t)a | ((uint8_t)b << 8) | ((uint8_t)c << 16) | ((uint8_t)d << 24);
	return result;
}