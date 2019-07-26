#include "stdafx.h"
#include "RewindManager.h"
#include "MessageManager.h"
#include "Console.h"
#include "EmuSettings.h"
#include "ControlManager.h"
#include "VideoRenderer.h"
#include "SoundMixer.h"
#include "BaseControlDevice.h"

RewindManager::RewindManager(shared_ptr<Console> console)
{
	_console = console;
	_settings = console->GetSettings();
	_rewindState = RewindState::Stopped;
	_framesToFastForward = 0;
	_hasHistory = false;
	AddHistoryBlock();

	_console->GetControlManager()->RegisterInputProvider(this);
	_console->GetControlManager()->RegisterInputRecorder(this);
}

RewindManager::~RewindManager()
{
	_console->GetControlManager()->UnregisterInputProvider(this);
	_console->GetControlManager()->UnregisterInputRecorder(this);
}

void RewindManager::ClearBuffer()
{
	_hasHistory = false;
	_history.clear();
	_historyBackup.clear();
	_currentHistory = RewindData();
	_framesToFastForward = 0;
	_videoHistory.clear();
	_videoHistoryBuilder.clear();
	_audioHistory.clear();
	_audioHistoryBuilder.clear();
	_rewindState = RewindState::Stopped;
	_currentHistory = RewindData();
}

void RewindManager::ProcessNotification(ConsoleNotificationType type, void * parameter)
{
	if(type == ConsoleNotificationType::PpuFrameDone) {
		_hasHistory = _history.size() >= 2;
		if(_settings->GetRewindBufferSize() > 0) {
			switch(_rewindState) {
				case RewindState::Starting:
				case RewindState::Started:
				case RewindState::Debugging:
					_currentHistory.FrameCount--;
					break;

				case RewindState::Stopping:
					_framesToFastForward--;
					_currentHistory.FrameCount++;
					if(_framesToFastForward == 0) {
						for(int i = 0; i < 4; i++) {
							size_t numberToRemove = _currentHistory.InputLogs[i].size();
							_currentHistory.InputLogs[i] = _historyBackup.front().InputLogs[i];
							for(size_t j = 0; j < numberToRemove; j++) {
								_currentHistory.InputLogs[i].pop_back();
							}
						}
						_historyBackup.clear();
						_rewindState = RewindState::Stopped;
						_settings->ClearFlag(EmulationFlags::Rewind);
						_settings->ClearFlag(EmulationFlags::MaximumSpeed);
					}
					break;
			
				case RewindState::Stopped:
					_currentHistory.FrameCount++;
					break;
			}
		} else {
			ClearBuffer();
		}
	} else if(type == ConsoleNotificationType::StateLoaded) {
		if(_rewindState == RewindState::Stopped) {
			//A save state was loaded by the user, mark as the end of the current "segment" (for history viewer)
			_currentHistory.EndOfSegment = true;
		}
	}
}

void RewindManager::AddHistoryBlock()
{
	uint32_t maxHistorySize = _settings->GetRewindBufferSize() * 60 * RewindManager::BufferSize / 60;
	if(maxHistorySize > 0) {
		while(_history.size() > maxHistorySize) {
			_history.pop_front();
		}

		if(_currentHistory.FrameCount > 0) {
			_history.push_back(_currentHistory);
		}
		_currentHistory = RewindData();
		_currentHistory.SaveState(_console);
	}
}

void RewindManager::PopHistory()
{
	if(_history.empty() && _currentHistory.FrameCount <= 0) {
		StopRewinding();
	} else {
		if(_currentHistory.FrameCount <= 0) {
			_currentHistory = _history.back();
			_history.pop_back();
		}

		_historyBackup.push_front(_currentHistory);
		_currentHistory.LoadState(_console);
		if(!_audioHistoryBuilder.empty()) {
			_audioHistory.insert(_audioHistory.begin(), _audioHistoryBuilder.begin(), _audioHistoryBuilder.end());
			_audioHistoryBuilder.clear();
		}
	}
}

void RewindManager::Start(bool forDebugger)
{
	if(_rewindState == RewindState::Stopped && _settings->GetRewindBufferSize() > 0) {
		auto lock = _console->AcquireLock();

		_rewindState = forDebugger ? RewindState::Debugging : RewindState::Starting;
		_videoHistoryBuilder.clear();
		_videoHistory.clear();
		_audioHistoryBuilder.clear();
		_audioHistory.clear();
		_historyBackup.clear();
		
		PopHistory();
		_console->GetSoundMixer()->StopAudio(true);
		_settings->SetFlag(EmulationFlags::MaximumSpeed);
		_settings->SetFlag(EmulationFlags::Rewind);
	}
}

void RewindManager::ForceStop()
{
	if(_rewindState != RewindState::Stopped) {
		while(_historyBackup.size() > 1) {
			_history.push_back(_historyBackup.front());
			_historyBackup.pop_front();
		}
		_currentHistory = _historyBackup.front();
		_historyBackup.clear();
		_rewindState = RewindState::Stopped;
		_settings->ClearFlag(EmulationFlags::MaximumSpeed);
		_settings->ClearFlag(EmulationFlags::Rewind);
	}
}

void RewindManager::Stop()
{
	if(_rewindState >= RewindState::Starting) {
		auto lock = _console->AcquireLock();
		if(_rewindState == RewindState::Started) {
			//Move back to the save state containing the frame currently shown on the screen
			if(_historyBackup.size() > 1) {
				_framesToFastForward = (uint32_t)_videoHistory.size() + _historyBackup.front().FrameCount;
				do {
					_history.push_back(_historyBackup.front());
					_framesToFastForward -= _historyBackup.front().FrameCount;
					_historyBackup.pop_front();

					_currentHistory = _historyBackup.front();
				}
				while(_framesToFastForward > RewindManager::BufferSize && _historyBackup.size() > 1);
			}
		} else {
			//We started rewinding, but didn't actually visually rewind anything yet
			//Move back to the save state containing the frame currently shown on the screen
			while(_historyBackup.size() > 1) {
				_history.push_back(_historyBackup.front());
				_historyBackup.pop_front();
			}
			_currentHistory = _historyBackup.front();
			_framesToFastForward = _historyBackup.front().FrameCount;
		}

		_currentHistory.LoadState(_console);
		if(_framesToFastForward > 0) {
			_rewindState = RewindState::Stopping;
			_currentHistory.FrameCount = 0;
			_settings->SetFlag(EmulationFlags::MaximumSpeed);
		} else {
			_rewindState = RewindState::Stopped;
			_historyBackup.clear();
			_settings->ClearFlag(EmulationFlags::MaximumSpeed);
			_settings->ClearFlag(EmulationFlags::Rewind);
		}

		_videoHistoryBuilder.clear();
		_videoHistory.clear();
		_audioHistoryBuilder.clear();
		_audioHistory.clear();
	}
}

void RewindManager::ProcessEndOfFrame()
{
	if(_rewindState >= RewindState::Starting) {
		if(_currentHistory.FrameCount <= 0 && _rewindState != RewindState::Debugging) {
			//If we're debugging, we want to keep running the emulation to the end of the next frame (even if it's incomplete)
			//Otherwise the emulation might diverge due to missing inputs.
			PopHistory();
		}
	} else if(_currentHistory.FrameCount >= RewindManager::BufferSize) {
		AddHistoryBlock();
	}
}

void RewindManager::ProcessFrame(void * frameBuffer, uint32_t width, uint32_t height, bool forRewind)
{
	if(_rewindState == RewindState::Starting || _rewindState == RewindState::Started) {
		if(!forRewind) {
			//Ignore any frames that occur between start of rewind process & first rewinded frame completed
			//These are caused by the fact that VideoDecoder is asynchronous - a previous (extra) frame can end up
			//in the rewind queue, which causes display glitches
			return;
		}

		VideoFrame newFrame;
		newFrame.Data = vector<uint32_t>((uint32_t*)frameBuffer, (uint32_t*)frameBuffer + width * height);
		newFrame.Width = width;
		newFrame.Height = height;
		_videoHistoryBuilder.push_back(newFrame);

		if(_videoHistoryBuilder.size() == (size_t)_historyBackup.front().FrameCount) {
			for(int i = (int)_videoHistoryBuilder.size() - 1; i >= 0; i--) {
				_videoHistory.push_front(_videoHistoryBuilder[i]);
			}
			_videoHistoryBuilder.clear();
		}

		if(_rewindState == RewindState::Started || _videoHistory.size() >= RewindManager::BufferSize) {
			_rewindState = RewindState::Started;
			_settings->ClearFlag(EmulationFlags::MaximumSpeed);
			if(!_videoHistory.empty()) {
				VideoFrame &frameData = _videoHistory.back();
				_console->GetVideoRenderer()->UpdateFrame(frameData.Data.data(), frameData.Width, frameData.Height);
				_videoHistory.pop_back();
			}
		}
	} else if(_rewindState == RewindState::Stopping || _rewindState == RewindState::Debugging) {
		//Display nothing while resyncing
	} else {
		_console->GetVideoRenderer()->UpdateFrame(frameBuffer, width, height);
	}
}

bool RewindManager::ProcessAudio(int16_t * soundBuffer, uint32_t sampleCount)
{
	if(_rewindState == RewindState::Starting || _rewindState == RewindState::Started) {
		_audioHistoryBuilder.insert(_audioHistoryBuilder.end(), soundBuffer, soundBuffer + sampleCount * 2);

		if(_rewindState == RewindState::Started && _audioHistory.size() > sampleCount * 2) {
			for(uint32_t i = 0; i < sampleCount * 2; i++) {
				soundBuffer[i] = _audioHistory.back();
				_audioHistory.pop_back();
			}

			return true;
		} else {
			//Mute while we prepare to rewind
			return false;
		}
	} else if(_rewindState == RewindState::Stopping || _rewindState == RewindState::Debugging) {
		//Mute while we resync
		return false;
	} else {
		return true;
	}
}

void RewindManager::RecordInput(vector<shared_ptr<BaseControlDevice>> devices)
{
	if(_settings->GetRewindBufferSize() > 0 && _rewindState == RewindState::Stopped) {
		for(shared_ptr<BaseControlDevice> &device : devices) {
			_currentHistory.InputLogs[device->GetPort()].push_back(device->GetRawState());
		}
	}
}

bool RewindManager::SetInput(BaseControlDevice *device)
{
	uint8_t port = device->GetPort();
	if(!_currentHistory.InputLogs[port].empty() && IsRewinding()) {
		ControlDeviceState state = _currentHistory.InputLogs[port].front();
		_currentHistory.InputLogs[port].pop_front();
		device->SetRawState(state);
		return true;
	} else {
		return false;
	}
}

void RewindManager::StartRewinding(bool forDebugger)
{
	Start(forDebugger);
}

void RewindManager::StopRewinding(bool forDebugger)
{
	if(forDebugger) {
		ForceStop();
	} else {
		Stop();
	}
}

bool RewindManager::IsRewinding()
{
	return _rewindState != RewindState::Stopped;
}

bool RewindManager::IsStepBack()
{
	return _rewindState == RewindState::Debugging;
}

void RewindManager::RewindSeconds(uint32_t seconds)
{
	if(_rewindState == RewindState::Stopped) {
		uint32_t removeCount = (seconds * 60 / RewindManager::BufferSize) + 1;
		auto lock = _console->AcquireLock();

		for(uint32_t i = 0; i < removeCount; i++) {
			if(!_history.empty()) {
				_currentHistory = _history.back();
				_history.pop_back();
			} else {
				break;
			}
		}
		_currentHistory.LoadState(_console);
	}
}

bool RewindManager::HasHistory()
{
	return _hasHistory;
}

void RewindManager::SendFrame(void * frameBuffer, uint32_t width, uint32_t height, bool forRewind)
{
	ProcessFrame(frameBuffer, width, height, forRewind);
}

bool RewindManager::SendAudio(int16_t * soundBuffer, uint32_t sampleCount)
{
	return ProcessAudio(soundBuffer, sampleCount);
}
