#include "pch.h"
#include "Shared/ShortcutKeyHandler.h"
#include "Shared/SystemActionManager.h"
#include "Shared/EmuSettings.h"
#include "Shared/KeyManager.h"
#include "Shared/Video/VideoDecoder.h"
#include "Shared/Emulator.h"
#include "Shared/RewindManager.h"
#include "Shared/NotificationManager.h"
#include "Shared/SaveStateManager.h"
#include "Shared/Movies/MovieManager.h"
#include "Shared/BaseControlManager.h"
#include "Shared/Interfaces/IBarcodeReader.h"
#include "Shared/Interfaces/ITapeRecorder.h"
#include "Netplay/GameClient.h"

ShortcutKeyHandler::ShortcutKeyHandler(Emulator* emu)
{
	_emu = emu;

	_keySetIndex = 0;
	_isKeyUp = false;
	_isKeyboardConnected = false;
	_repeatStarted = false;
	_needRepeat = false;

	_stopThread = false;
	_thread = std::thread([=]() {
		while(!_stopThread) {
			ProcessKeys();
			std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(50));
		}
	});
}

ShortcutKeyHandler::~ShortcutKeyHandler()
{
	_stopThread = true;
	_thread.join();
}

bool ShortcutKeyHandler::IsKeyPressed(EmulatorShortcut shortcut)
{
	//When running while a keyboard is plugged into the console, disable all keyboard
	//shortcut keys. The pause shortcut is always enabled, allowing it to be used to
	//pause normally, which allows other shortcuts to be used (while paused)
	bool blockKeyboardKeys = shortcut != EmulatorShortcut::Pause && _isKeyboardConnected && !_isPaused;

	KeyCombination keyComb = _emu->GetSettings()->GetShortcutKey(shortcut, _keySetIndex);
	vector<KeyCombination> supersets = _emu->GetSettings()->GetShortcutSupersets(shortcut, _keySetIndex);
	for(KeyCombination &superset : supersets) {
		if(IsKeyPressed(superset, blockKeyboardKeys)) {
			//A superset is pressed, ignore this subset
			return false;
		}
	}

	//No supersets are pressed, check if all matching keys are pressed
	return IsKeyPressed(keyComb, blockKeyboardKeys);
}

bool ShortcutKeyHandler::IsKeyPressed(KeyCombination comb, bool blockKeyboardKeys)
{
	int keyCount = (comb.Key1 ? 1 : 0) + (comb.Key2 ? 1 : 0) + (comb.Key3 ? 1 : 0);

	if(keyCount == 0 || _pressedKeys.empty()) {
		return false;
	}

	bool mergeCtrlAltShift = keyCount > 1;

	return IsKeyPressed(comb.Key1, mergeCtrlAltShift, blockKeyboardKeys) &&
		(comb.Key2 == 0 || IsKeyPressed(comb.Key2, mergeCtrlAltShift, blockKeyboardKeys)) &&
		(comb.Key3 == 0 || IsKeyPressed(comb.Key3, mergeCtrlAltShift, blockKeyboardKeys));
}

bool ShortcutKeyHandler::IsKeyPressed(uint16_t keyCode, bool mergeCtrlAltShift, bool blockKeyboardKeys)
{
	if(blockKeyboardKeys && keyCode < IKeyManager::BaseMouseButtonIndex) {
		return false;
	}

	if(keyCode >= 116 && keyCode <= 121 && mergeCtrlAltShift) {
		//Left/right ctrl/alt/shift
		//Return true if either the left or right key is pressed
		return KeyManager::IsKeyPressed(keyCode | 1) || KeyManager::IsKeyPressed(keyCode & ~0x01);
	}

	return KeyManager::IsKeyPressed(keyCode);
}

bool ShortcutKeyHandler::DetectKeyPress(EmulatorShortcut shortcut)
{
	if(IsKeyPressed(shortcut)) {
		bool newlyPressed = _prevKeysDown[_keySetIndex].find((uint32_t)shortcut) == _prevKeysDown[_keySetIndex].end();
		_keysDown[_keySetIndex].emplace((uint32_t)shortcut);

		if(newlyPressed && !_isKeyUp) {
			return true;
		}
	}
	return false;
}

bool ShortcutKeyHandler::DetectKeyRelease(EmulatorShortcut shortcut)
{
	if(!IsKeyPressed(shortcut)) {
		if(_prevKeysDown[_keySetIndex].find((uint32_t)shortcut) != _prevKeysDown[_keySetIndex].end()) {
			return true;
		}
	}
	return false;
}

void ShortcutKeyHandler::ProcessRunSingleFrame()
{
	_runSingleFrameRepeatTimer.Reset();
	_needRepeat = true;
	_emu->PauseOnNextFrame();
}

bool ShortcutKeyHandler::IsShortcutAllowed(EmulatorShortcut shortcut, uint32_t shortcutParam)
{
	bool isRunning = _emu->IsRunning();
	bool isNetplayClient = _emu->GetGameClient()->Connected();
	bool isMoviePlaying = _emu->GetMovieManager()->Playing();
	bool isMovieRecording = _emu->GetMovieManager()->Recording();
	bool isMovieActive = isMoviePlaying || isMovieRecording;

	switch(shortcut) {
		case EmulatorShortcut::ToggleRewind:
		case EmulatorShortcut::Rewind:
		case EmulatorShortcut::RewindTenSecs:
		case EmulatorShortcut::RewindOneMin:
			return isRunning && !isNetplayClient && !isMovieRecording;

		case EmulatorShortcut::IncreaseSpeed:
		case EmulatorShortcut::DecreaseSpeed:
		case EmulatorShortcut::MaxSpeed:
			return !isNetplayClient;

		case EmulatorShortcut::Pause:
			return isRunning && !isNetplayClient;

		case EmulatorShortcut::Reset:
		case EmulatorShortcut::ExecReset:
		case EmulatorShortcut::PowerCycle:
		case EmulatorShortcut::ExecPowerCycle:
		case EmulatorShortcut::ReloadRom:
		case EmulatorShortcut::ExecReloadRom:
			return isRunning && !isNetplayClient && !isMoviePlaying;

		case EmulatorShortcut::PowerOff:
		case EmulatorShortcut::ExecPowerOff:
			return isRunning && !isNetplayClient;

		case EmulatorShortcut::TakeScreenshot:
			return isRunning;

		case EmulatorShortcut::ToggleCheats:
			return !isNetplayClient && !isMovieActive;

		case EmulatorShortcut::SelectSaveSlot1: case EmulatorShortcut::SelectSaveSlot2: case EmulatorShortcut::SelectSaveSlot3: case EmulatorShortcut::SelectSaveSlot4: case EmulatorShortcut::SelectSaveSlot5:
		case EmulatorShortcut::SelectSaveSlot6: case EmulatorShortcut::SelectSaveSlot7: case EmulatorShortcut::SelectSaveSlot8: case EmulatorShortcut::SelectSaveSlot9: case EmulatorShortcut::SelectSaveSlot10:
		case EmulatorShortcut::SaveStateSlot1: case EmulatorShortcut::SaveStateSlot2: case EmulatorShortcut::SaveStateSlot3: case EmulatorShortcut::SaveStateSlot4: case EmulatorShortcut::SaveStateSlot5:
		case EmulatorShortcut::SaveStateSlot6: case EmulatorShortcut::SaveStateSlot7: case EmulatorShortcut::SaveStateSlot8: case EmulatorShortcut::SaveStateSlot9: case EmulatorShortcut::SaveStateSlot10:
		case EmulatorShortcut::MoveToNextStateSlot:
		case EmulatorShortcut::MoveToPreviousStateSlot:
		case EmulatorShortcut::SaveStateDialog:
		case EmulatorShortcut::SaveStateToFile:
		case EmulatorShortcut::SaveState:
			return isRunning;

		case EmulatorShortcut::LoadStateSlot1: case EmulatorShortcut::LoadStateSlot2: case EmulatorShortcut::LoadStateSlot3: case EmulatorShortcut::LoadStateSlot4: case EmulatorShortcut::LoadStateSlot5:
		case EmulatorShortcut::LoadStateSlot6: case EmulatorShortcut::LoadStateSlot7: case EmulatorShortcut::LoadStateSlot8: case EmulatorShortcut::LoadStateSlot9: case EmulatorShortcut::LoadStateSlot10:
		case EmulatorShortcut::LoadStateSlotAuto:
		case EmulatorShortcut::LoadStateDialog:
		case EmulatorShortcut::LoadStateFromFile:
		case EmulatorShortcut::LoadState:
		case EmulatorShortcut::LoadLastSession:
			return isRunning && !isNetplayClient && !isMovieActive;

		case EmulatorShortcut::InputBarcode:
			if(isRunning && !isNetplayClient && !isMoviePlaying) {
				shared_ptr<IConsole> console = _emu->GetConsole();
				if(console) {
					return console->GetControlManager()->GetControlDevice<IBarcodeReader>() != nullptr;
				}
			}
			return false;

		case EmulatorShortcut::RecordTape:
		case EmulatorShortcut::StopRecordTape:
		case EmulatorShortcut::LoadTape: {
			if(isRunning && !isNetplayClient && !isMoviePlaying) {
				shared_ptr<IConsole> console = _emu->GetConsole();
				if(console) {
					shared_ptr<ITapeRecorder> recorder = console->GetControlManager()->GetControlDevice<ITapeRecorder>();
					if(recorder) {
						bool recording = recorder->IsRecording();
						if(recording) {
							return shortcut == EmulatorShortcut::StopRecordTape;
						} else {
							return shortcut != EmulatorShortcut::StopRecordTape;
						}
					}
				}
			}

			return false;
		}
	}

	ShortcutState state = _emu->IsShortcutAllowed(shortcut, shortcutParam);
	if(state == ShortcutState::Default) {
		return shortcut < EmulatorShortcut::InputBarcode;
	} else {
		return state == ShortcutState::Enabled ? true : false;
	}
}

void ShortcutKeyHandler::ProcessShortcutPressed(EmulatorShortcut shortcut, uint32_t shortcutParam)
{
	if(!IsShortcutAllowed(shortcut, shortcutParam)) {
		return;
	}

	EmuSettings* settings = _emu->GetSettings();

	switch(shortcut) {
		case EmulatorShortcut::Pause:
			if(_emu->IsPaused()) {
				_emu->Resume();
			} else {
				_emu->Pause();
			}
			break;
		
		case EmulatorShortcut::ExecReset: _emu->GetSystemActionManager()->Reset(); break;
		case EmulatorShortcut::ExecPowerCycle: _emu->GetSystemActionManager()->PowerCycle(); break;
		case EmulatorShortcut::ExecReloadRom: _emu->ReloadRom(false); break;
		case EmulatorShortcut::ExecPowerOff: _emu->Stop(true); break;

		case EmulatorShortcut::FastForward: settings->SetFlag(EmulationFlags::Turbo); break;
		case EmulatorShortcut::ToggleFastForward:
			if(settings->CheckFlag(EmulationFlags::Turbo)) {
				settings->ClearFlag(EmulationFlags::Turbo);
			} else {
				settings->SetFlag(EmulationFlags::Turbo);
			}
			break;

		case EmulatorShortcut::SelectSaveSlot1: case EmulatorShortcut::SelectSaveSlot2: case EmulatorShortcut::SelectSaveSlot3: case EmulatorShortcut::SelectSaveSlot4: case EmulatorShortcut::SelectSaveSlot5:
		case EmulatorShortcut::SelectSaveSlot6: case EmulatorShortcut::SelectSaveSlot7: case EmulatorShortcut::SelectSaveSlot8: case EmulatorShortcut::SelectSaveSlot9: case EmulatorShortcut::SelectSaveSlot10:
			_emu->GetSaveStateManager()->SelectSaveSlot((int)shortcut - (int)EmulatorShortcut::SelectSaveSlot1 + 1);
			break;

		case EmulatorShortcut::SaveStateSlot1: case EmulatorShortcut::SaveStateSlot2: case EmulatorShortcut::SaveStateSlot3: case EmulatorShortcut::SaveStateSlot4: case EmulatorShortcut::SaveStateSlot5:
		case EmulatorShortcut::SaveStateSlot6: case EmulatorShortcut::SaveStateSlot7: case EmulatorShortcut::SaveStateSlot8: case EmulatorShortcut::SaveStateSlot9: case EmulatorShortcut::SaveStateSlot10:
			_emu->GetSaveStateManager()->SaveState((int)shortcut - (int)EmulatorShortcut::SaveStateSlot1 + 1);
			break;
		
		case EmulatorShortcut::LoadStateSlot1: case EmulatorShortcut::LoadStateSlot2: case EmulatorShortcut::LoadStateSlot3: case EmulatorShortcut::LoadStateSlot4: case EmulatorShortcut::LoadStateSlot5:
		case EmulatorShortcut::LoadStateSlot6: case EmulatorShortcut::LoadStateSlot7: case EmulatorShortcut::LoadStateSlot8: case EmulatorShortcut::LoadStateSlot9: case EmulatorShortcut::LoadStateSlot10:
		case EmulatorShortcut::LoadStateSlotAuto:
			_emu->GetSaveStateManager()->LoadState((int)shortcut - (int)EmulatorShortcut::LoadStateSlot1 + 1);
			break;

		case EmulatorShortcut::MoveToNextStateSlot: _emu->GetSaveStateManager()->MoveToNextSlot(); break;
		case EmulatorShortcut::MoveToPreviousStateSlot: _emu->GetSaveStateManager()->MoveToPreviousSlot(); break;
		case EmulatorShortcut::SaveState: _emu->GetSaveStateManager()->SaveState(); break;
		case EmulatorShortcut::LoadState: _emu->GetSaveStateManager()->LoadState(); break;

		case EmulatorShortcut::RunSingleFrame: ProcessRunSingleFrame(); break;
		
		case EmulatorShortcut::ToggleRewind:
			if(_emu->GetRewindManager()->IsRewinding()) {
				_emu->GetRewindManager()->StopRewinding();
			} else {
				_emu->GetRewindManager()->StartRewinding();
			}
			break;

		case EmulatorShortcut::Rewind: _emu->GetRewindManager()->StartRewinding(); break;
		case EmulatorShortcut::RewindTenSecs: _emu->GetRewindManager()->RewindSeconds(10); break;
		case EmulatorShortcut::RewindOneMin: _emu->GetRewindManager()->RewindSeconds(60); break;
		
		default:
			//Anything else is managed by the UI
			break;
	}
}

void ShortcutKeyHandler::ProcessShortcutReleased(EmulatorShortcut shortcut, uint32_t shortcutParam)
{
	if(!IsShortcutAllowed(shortcut, shortcutParam)) {
		return;
	}

	EmuSettings* settings = _emu->GetSettings();
	switch(shortcut) {
		case EmulatorShortcut::FastForward: settings->ClearFlag(EmulationFlags::Turbo); break;
		case EmulatorShortcut::Rewind: _emu->GetRewindManager()->StopRewinding(); break;
		
		case EmulatorShortcut::RunSingleFrame:
			_repeatStarted = false;
			_needRepeat = false;
			break;
	}
}

void ShortcutKeyHandler::CheckMappedKeys()
{
	for(uint64_t i = 0; i < (uint64_t)EmulatorShortcut::ShortcutCount; i++) {
		EmulatorShortcut shortcut = (EmulatorShortcut)i;
		if(DetectKeyPress(shortcut)) {
			if(!IsShortcutAllowed(shortcut, 0)) {
				continue;
			}

			ExecuteShortcutParams params = {};
			params.Shortcut = shortcut;
			_emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::ExecuteShortcut, &params);
		} else if(DetectKeyRelease(shortcut)) {
			ExecuteShortcutParams params = {};
			params.Shortcut = shortcut;
			_emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::ReleaseShortcut, &params);
		}
	}
}

void ShortcutKeyHandler::ProcessKeys()
{
	if(!_emu->GetSettings()->IsInputEnabled()) {
		return;
	}

	auto lock = _lock.AcquireSafe();
	KeyManager::RefreshKeyState();

	_isKeyboardConnected = _emu->IsKeyboardConnected();
	_isPaused = _emu->IsPaused();

	_pressedKeys = KeyManager::GetPressedKeys();
	_isKeyUp = _pressedKeys.size() < _lastPressedKeys.size();

	bool noChange = false;
	if(_pressedKeys.size() == _lastPressedKeys.size()) {
		noChange = true;
		for(size_t i = 0; i < _pressedKeys.size(); i++) {
			if(_pressedKeys[i] != _lastPressedKeys[i]) {
				noChange = false;
				break;
			}
		}
	}

	if(!noChange) {
		//Only run this if the keys have changed
		for(int i = 0; i < 2; i++) {
			_keysDown[i].clear();
			_keySetIndex = i;
			CheckMappedKeys();
			_prevKeysDown[i] = _keysDown[i];
		}

		_lastPressedKeys = _pressedKeys;
	}

	if(_needRepeat) {
		double elapsedMs = _runSingleFrameRepeatTimer.GetElapsedMS();
		if((_repeatStarted && elapsedMs >= 50) || (!_repeatStarted && elapsedMs >= 500)) {
			//Over 500ms has elapsed since the key was first pressed, or over 50ms since repeat mode started (20fps)
			//In this case, run another frame and pause again.
			_repeatStarted = true;
			ProcessRunSingleFrame();
		}
	}
}

void ShortcutKeyHandler::ProcessNotification(ConsoleNotificationType type, void* parameter)
{
	switch(type) {
		case ConsoleNotificationType::ExecuteShortcut: {
			ExecuteShortcutParams p = *(ExecuteShortcutParams*)parameter;
			ProcessShortcutPressed(p.Shortcut, p.Param);
			break;
		}

		case ConsoleNotificationType::ReleaseShortcut: {
			ExecuteShortcutParams p = *(ExecuteShortcutParams*)parameter;
			ProcessShortcutReleased(p.Shortcut, p.Param);
			break;
		}
	}
}
