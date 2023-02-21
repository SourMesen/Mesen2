#include "pch.h"
#include "Shared/HistoryViewer.h"
#include "Shared/RewindData.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/Video/VideoRenderer.h"
#include "Shared/BaseControlManager.h"
#include "Shared/RewindManager.h"
#include "Shared/CheatManager.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/Audio/SoundMixer.h"
#include "Shared/NotificationManager.h"
#include "Shared/Movies/MovieRecorder.h"
#include "Shared/SaveStateManager.h"

HistoryViewer::HistoryViewer(Emulator* emu)
{
	_emu = emu;
	_position = 0;
	_pollCounter = 0;
}

HistoryViewer::~HistoryViewer()
{
}

bool HistoryViewer::Initialize(Emulator* mainEmu)
{
	auto mainLock = mainEmu->AcquireLock();
	auto subLock = _emu->AcquireLock();

	if(!_emu->LoadRom(mainEmu->GetRomInfo().RomFile, mainEmu->GetRomInfo().PatchFile)) {
		return false;
	}

	_mainEmu = mainEmu;

	vector<CheatCode> cheats = _mainEmu->GetCheatManager()->GetCheats();
	_emu->GetCheatManager()->SetCheats(cheats);

	//Disable rewind history to reduce memory usage
	_emu->GetSettings()->GetPreferences().RewindBufferSize = 0;

	//Disable battery saving for this instance
	_emu->GetBatteryManager()->Initialize("");
	
	_history = mainEmu->GetRewindManager()->GetHistory();
	
	_emu->UnregisterInputProvider(this);
	_emu->RegisterInputProvider(this);
	
	SeekTo(0);

	return true;
}

void HistoryViewer::SetOptions(HistoryViewerOptions options)
{
	if(options.IsPaused) {
		_emu->Pause();
	} else {
		_emu->Resume();
	}

	_emu->GetSettings()->GetAudioConfig().MasterVolume = options.Volume;
	_emu->GetVideoRenderer()->SetRendererSize(options.Width, options.Height);
}

HistoryViewerState HistoryViewer::GetState()
{
	HistoryViewerState state = {};
	state.Volume = _emu->GetSettings()->GetAudioConfig().MasterVolume;
	state.IsPaused = _emu->IsPaused();
	state.Position = _position * RewindManager::BufferSize;
	state.Length = (uint32_t)(_history.size() * RewindManager::BufferSize);
	state.Fps = _emu->GetTimingInfo(_emu->GetCpuTypes()[0]).Fps;

	uint32_t segmentCount = 0;
	for(size_t i = 0; i < _history.size(); i++) {
		if(_history[i].EndOfSegment || i == _history.size() - 1) {
			state.Segments[segmentCount] = (uint32_t)i * RewindManager::BufferSize;
			segmentCount++;

			if(segmentCount == 1000) {
				//Reached max count, can't return any more values
				break;
			}
		}
	}

	state.SegmentCount = segmentCount;

	return state;
}

void HistoryViewer::SeekTo(uint32_t seekPosition)
{
	//Seek to the specified position
	seekPosition /= RewindManager::BufferSize;
	if(seekPosition < _history.size()) {
		auto lock = _emu->AcquireLock();
		
		_position = seekPosition;
		RewindData rewindData = _history[_position];
		rewindData.LoadState(_emu, _history, _position);

		_emu->GetSoundMixer()->StopAudio(true);
		_pollCounter = 0;
	}
}

bool HistoryViewer::CreateSaveState(string outputFile, uint32_t position)
{
	if(_history.empty()) {
		return false;
	}

	position /= RewindManager::BufferSize;
	position = std::min(position, (uint32_t)_history.size() - 1);

	std::stringstream stateData;
	_emu->GetSaveStateManager()->GetSaveStateHeader(stateData);
	_history[position].GetStateData(stateData, _history, position);

	ofstream output(outputFile, ios::binary);
	if(output) {
		output << stateData.rdbuf();
		output.close();
		return true;
	}
	return false;
}

bool HistoryViewer::SaveMovie(string movieFile, uint32_t startPosition, uint32_t endPosition)
{
	startPosition /= RewindManager::BufferSize;
	endPosition /= RewindManager::BufferSize;

	//Take a savestate to be able to restore it after generating the movie file
	//(the movie generation uses the console's inputs, which could affect the emulation otherwise)
	stringstream state;
	auto lock = _emu->AcquireLock();
	_emu->Serialize(state, true, false);

	//Convert the rewind data to a .mmo file
	unique_ptr<MovieRecorder> recorder(new MovieRecorder(_emu));
	bool result = recorder->CreateMovie(movieFile, _history, startPosition, endPosition, _mainEmu->GetBatteryManager()->HasBattery());

	//Resume the state and resume
	_emu->Deserialize(state, SaveStateManager::FileFormatVersion, true);
	return result;
}

void HistoryViewer::ResumeGameplay(uint32_t resumePosition)
{
	resumePosition /= RewindManager::BufferSize;

	auto lock = _mainEmu->AcquireLock();
	RomInfo mainRom = _mainEmu->GetRomInfo();
	RomInfo viewerRom = _emu->GetRomInfo();

	if(!_mainEmu->IsRunning() || (string)mainRom.RomFile != (string)viewerRom.RomFile || (string)mainRom.PatchFile != (string)viewerRom.PatchFile) {
		//Load game on the main window if they aren't the same
		if(!_mainEmu->LoadRom(_emu->GetRomInfo().RomFile, _emu->GetRomInfo().PatchFile)) {
			return;
		}
	}

	if(resumePosition < _history.size()) {
		_history[resumePosition].LoadState(_mainEmu, _history, resumePosition);
	} else {
		_history[_history.size() - 1].LoadState(_mainEmu, _history, (int32_t)_history.size() - 1);
	}
}

bool HistoryViewer::SetInput(BaseControlDevice *device)
{
	uint8_t port = device->GetPort();
	if(_position < _history.size()) {
		std::deque<ControlDeviceState> &stateData = _history[_position].InputLogs[port];
		if(_pollCounter < stateData.size()) {
			ControlDeviceState state = stateData[_pollCounter];
			device->SetRawState(state);
		}
	}
	if(port == 0 && _pollCounter < RewindManager::BufferSize) {
		_pollCounter++;
	}
	return true;
}

void HistoryViewer::ProcessEndOfFrame()
{
	if(_history.empty()) {
		return;
	}

	if(_pollCounter >= (uint32_t)_history[_position].FrameCount) {
		_pollCounter = 0;
		_position++;

		if(_position >= _history.size()) {
			//Reached the end of history data
			SeekTo(0);
			_emu->Pause();
			return;
		}

		RewindData rewindData = _history[_position];
		rewindData.LoadState(_emu, _history, _position);
	}
}
