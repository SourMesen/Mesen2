#include "stdafx.h"
#include <deque>
#include "Utilities/HexUtilities.h"
#include "Utilities/FolderUtilities.h"
#include "Utilities/ZipWriter.h"
#include "Utilities/VirtualFile.h"
#include "MovieRecorder.h"
#include "MessageManager.h"
#include "SNES/ControlManager.h"
#include "BaseControlDevice.h"
#include "Emulator.h"
#include "EmuSettings.h"
#include "SaveStateManager.h"
#include "NotificationManager.h"
#include "RewindData.h"
#include "MovieTypes.h"
#include "BatteryManager.h"
#include "CheatManager.h"

MovieRecorder::MovieRecorder(shared_ptr<Emulator> emu)
{
	_emu = emu;
}

MovieRecorder::~MovieRecorder()
{
	Stop();
}

bool MovieRecorder::Record(RecordMovieOptions options)
{
	_filename = options.Filename;
	_author = options.Author;
	_description = options.Description;
	_writer.reset(new ZipWriter());
	_inputData = stringstream();
	_saveStateData = stringstream();
	_hasSaveState = false;

	if(!_writer->Initialize(_filename)) {
		MessageManager::DisplayMessage("Movies", "CouldNotWriteToFile", FolderUtilities::GetFilename(_filename, true));
		_writer.reset();
		return false;
	} else {
		_emu->Lock();
		_emu->GetNotificationManager()->RegisterNotificationListener(shared_from_this());

		if(options.RecordFrom == RecordMovieFrom::StartWithoutSaveData) {
			//Power cycle and ignore save data that exists on the disk
			_emu->GetBatteryManager()->SetBatteryProvider(shared_from_this());
			_emu->PowerCycle();
		} else if(options.RecordFrom == RecordMovieFrom::StartWithSaveData) {
			//Power cycle and save existing battery files into the movie file
			_emu->GetBatteryManager()->SetBatteryRecorder(shared_from_this());
			_emu->PowerCycle();
		} else if(options.RecordFrom == RecordMovieFrom::CurrentState) {
			//Record from current state, store a save state in the movie file
			_emu->GetControlManager()->RegisterInputRecorder(this);
			_emu->GetSaveStateManager()->SaveState(_saveStateData);
			_hasSaveState = true;
		}
		
		_emu->GetBatteryManager()->SetBatteryRecorder(nullptr);
		_emu->Unlock();

		MessageManager::DisplayMessage("Movies", "MovieRecordingTo", FolderUtilities::GetFilename(_filename, true));

		return true;
	}
}

void MovieRecorder::GetGameSettings(stringstream &out)
{
	EmuSettings* settings = _emu->GetSettings().get();
	EmulationConfig emuConfig = settings->GetEmulationConfig();
	InputConfig inputConfig = settings->GetInputConfig();
	
	WriteString(out, MovieKeys::MesenVersion, settings->GetVersionString());
	WriteInt(out, MovieKeys::MovieFormatVersion, MovieRecorder::MovieFormatVersion);

	VirtualFile romFile = _emu->GetRomInfo().RomFile;
	WriteString(out, MovieKeys::GameFile, romFile.GetFileName());
	WriteString(out, MovieKeys::Sha1, _emu->GetHash(HashType::Sha1));

	VirtualFile patchFile = _emu->GetRomInfo().PatchFile;
	if(patchFile.IsValid()) {
		WriteString(out, MovieKeys::PatchFile, patchFile.GetFileName());
		WriteString(out, MovieKeys::PatchFileSha1, patchFile.GetSha1Hash());
	
		romFile.ApplyPatch(patchFile);
		WriteString(out, MovieKeys::PatchedRomSha1, romFile.GetSha1Hash());
	}

	ConsoleRegion region = _emu->GetRegion();
	switch(region) {
		case ConsoleRegion::Auto:
		case ConsoleRegion::Ntsc: WriteString(out, MovieKeys::Region, "NTSC"); break;

		case ConsoleRegion::Pal: WriteString(out, MovieKeys::Region, "PAL"); break;
	}

	WriteString(out, MovieKeys::Controller1, ControllerTypeNames[(int)inputConfig.Controllers[0].Type]);
	WriteString(out, MovieKeys::Controller2, ControllerTypeNames[(int)inputConfig.Controllers[1].Type]);
	WriteString(out, MovieKeys::Controller3, ControllerTypeNames[(int)inputConfig.Controllers[2].Type]);
	WriteString(out, MovieKeys::Controller4, ControllerTypeNames[(int)inputConfig.Controllers[3].Type]);
	WriteString(out, MovieKeys::Controller5, ControllerTypeNames[(int)inputConfig.Controllers[4].Type]);

	WriteInt(out, MovieKeys::ExtraScanlinesBeforeNmi, emuConfig.PpuExtraScanlinesBeforeNmi);
	WriteInt(out, MovieKeys::ExtraScanlinesAfterNmi, emuConfig.PpuExtraScanlinesAfterNmi);
	WriteInt(out, MovieKeys::GsuClockSpeed, emuConfig.GsuClockSpeed);
	
	switch(emuConfig.RamPowerOnState) {
		case RamState::AllZeros: WriteString(out, MovieKeys::RamPowerOnState, "AllZeros"); break;
		case RamState::AllOnes: WriteString(out, MovieKeys::RamPowerOnState, "AllOnes"); break;
		case RamState::Random: WriteString(out, MovieKeys::RamPowerOnState, "AllOnes"); break; //TODO: Random memory isn't supported for movies yet
	}

	for(CheatCode &code : _emu->GetCheatManager()->GetCheats()) {
		out << "Cheat " << HexUtilities::ToHex24(code.Address) << " " << HexUtilities::ToHex(code.Value) << "\n";
	}
}

void MovieRecorder::WriteString(stringstream &out, string name, string value)
{
	out << name << " " << value << "\n";
}

void MovieRecorder::WriteInt(stringstream &out, string name, uint32_t value)
{
	out << name << " " << std::to_string(value) << "\n";
}

void MovieRecorder::WriteBool(stringstream &out, string name, bool enabled)
{
	out << name << " " << (enabled ? "true" : "false") << "\n";
}

bool MovieRecorder::Stop()
{
	if(_writer) {
		_emu->GetControlManager()->UnregisterInputRecorder(this);

		_writer->AddFile(_inputData, "Input.txt");

		stringstream out;
		GetGameSettings(out);
		_writer->AddFile(out, "GameSettings.txt");

		if(!_author.empty() || !_description.empty()) {
			stringstream movieInfo;
			WriteString(movieInfo, "Author", _author);
			movieInfo << "Description\n" << _description;
			_writer->AddFile(movieInfo, "MovieInfo.txt");
		}

		VirtualFile patchFile = _emu->GetRomInfo().PatchFile;
		vector<uint8_t> patchData;
		if(patchFile.IsValid() && patchFile.ReadFile(patchData)) {
			_writer->AddFile(patchData, "PatchData.dat");
		}

		if(_hasSaveState) {
			_writer->AddFile(_saveStateData, "SaveState.mss");
		}

		for(auto kvp : _batteryData) {
			_writer->AddFile(kvp.second, "Battery" + kvp.first);
		}

		bool result = _writer->Save();
		if(result) {
			MessageManager::DisplayMessage("Movies", "MovieSaved", FolderUtilities::GetFilename(_filename, true));
		}
		return result;
	}

	return false;
}

void MovieRecorder::RecordInput(vector<shared_ptr<BaseControlDevice>> devices)
{
	for(shared_ptr<BaseControlDevice> &device : devices) {
		_inputData << ("|" + device->GetTextState());
	}
	_inputData << "\n";
}

void MovieRecorder::OnLoadBattery(string extension, vector<uint8_t> batteryData)
{
	_batteryData[extension] = batteryData;
}

vector<uint8_t> MovieRecorder::LoadBattery(string extension)
{
	return vector<uint8_t>();
}

void MovieRecorder::ProcessNotification(ConsoleNotificationType type, void *parameter)
{
	if(type == ConsoleNotificationType::GameLoaded) {
		_emu->GetControlManager()->RegisterInputRecorder(this);
	}
}
/*
bool MovieRecorder::CreateMovie(string movieFile, std::deque<RewindData> &data, uint32_t startPosition, uint32_t endPosition)
{
	_filename = movieFile;
	_writer.reset(new ZipWriter());
	if(startPosition < data.size() && endPosition <= data.size() && _writer->Initialize(_filename)) {
		vector<shared_ptr<BaseControlDevice>> devices = _console->GetControlManager()->GetControlDevices();
		
		if(startPosition > 0 || _console->GetRomInfo().HasBattery || _console->GetSettings()->GetRamPowerOnState() == RamPowerOnState::Random) {
			//Create a movie from a savestate if we don't start from the beginning (or if the game has save ram, or if the power on ram state is random)
			_hasSaveState = true;
			_saveStateData = stringstream();
			_console->GetSaveStateManager()->GetSaveStateHeader(_saveStateData);
			data[startPosition].GetStateData(_saveStateData);
		}

		_inputData = stringstream();

		for(uint32_t i = startPosition; i < endPosition; i++) {
			RewindData rewindData = data[i];
			for(uint32_t i = 0; i < 30; i++) {
				for(shared_ptr<BaseControlDevice> &device : devices) {
					uint8_t port = device->GetPort();
					if(i < rewindData.InputLogs[port].size()) {
						device->SetRawState(rewindData.InputLogs[port][i]);
						_inputData << ("|" + device->GetTextState());
					}
				}
				_inputData << "\n";
			}
		}

		//Write the movie file
		return Stop();
	}
	return false;
}
*/