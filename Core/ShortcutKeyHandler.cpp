#include "stdafx.h"
#include "ShortcutKeyHandler.h"
#include "EmuSettings.h"
#include "KeyManager.h"
#include "VideoDecoder.h"
#include "ControlManager.h"
#include "Emulator.h"
#include "RewindManager.h"
#include "NotificationManager.h"
#include "SaveStateManager.h"
#include "MovieManager.h"
#include "GameClient.h"

ShortcutKeyHandler::ShortcutKeyHandler(shared_ptr<Emulator> emu)
{
	_emu = emu;
	_keySetIndex = 0;
	_isKeyUp = false;
	_repeatStarted = false;

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
	KeyCombination keyComb = _emu->GetSettings()->GetShortcutKey(shortcut, _keySetIndex);
	vector<KeyCombination> supersets = _emu->GetSettings()->GetShortcutSupersets(shortcut, _keySetIndex);
	for(KeyCombination &superset : supersets) {
		if(IsKeyPressed(superset)) {
			//A superset is pressed, ignore this subset
			return false;
		}
	}

	//No supersets are pressed, check if all matching keys are pressed
	return IsKeyPressed(keyComb);
}

bool ShortcutKeyHandler::IsKeyPressed(KeyCombination comb)
{
	int keyCount = (comb.Key1 ? 1 : 0) + (comb.Key2 ? 1 : 0) + (comb.Key3 ? 1 : 0);

	if(keyCount == 0 || _pressedKeys.empty()) {
		return false;
	}

	return IsKeyPressed(comb.Key1) &&
		(comb.Key2 == 0 || IsKeyPressed(comb.Key2)) &&
		(comb.Key3 == 0 || IsKeyPressed(comb.Key3));
}

bool ShortcutKeyHandler::IsKeyPressed(uint32_t keyCode)
{
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
	shared_ptr<Timer> timer = _runSingleFrameRepeatTimer;
	if(!timer) {
		timer.reset(new Timer());
		_runSingleFrameRepeatTimer = timer;
	}
	timer->Reset();

	_emu->PauseOnNextFrame();
}

void ShortcutKeyHandler::CheckMappedKeys()
{
	shared_ptr<EmuSettings> settings = _emu->GetSettings();
	bool isNetplayClient = GameClient::Connected();
	bool isMovieActive = _emu->GetMovieManager()->Playing() || _emu->GetMovieManager()->Recording();
	bool isMovieRecording = _emu->GetMovieManager()->Recording();

	//Let the UI handle these shortcuts
	for(uint64_t i = (uint64_t)EmulatorShortcut::TakeScreenshot; i < (uint64_t)EmulatorShortcut::ShortcutCount; i++) {
		if(DetectKeyPress((EmulatorShortcut)i)) {
			void* param = (void*)i;
			_emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::ExecuteShortcut, param);
		}
	}

	if(DetectKeyPress(EmulatorShortcut::FastForward)) {
		settings->SetFlag(EmulationFlags::Turbo);
	} else if(DetectKeyRelease(EmulatorShortcut::FastForward)) {
		settings->ClearFlag(EmulationFlags::Turbo);
	}

	if(DetectKeyPress(EmulatorShortcut::ToggleFastForward)) {
		if(settings->CheckFlag(EmulationFlags::Turbo)) {
			settings->ClearFlag(EmulationFlags::Turbo);
		} else {
			settings->SetFlag(EmulationFlags::Turbo);
		}
	}

	for(int i = 0; i < 10; i++) {
		if(DetectKeyPress((EmulatorShortcut)((int)EmulatorShortcut::SelectSaveSlot1 + i))) {
			_emu->GetSaveStateManager()->SelectSaveSlot(i + 1);
		}
	}

	if(DetectKeyPress(EmulatorShortcut::MoveToNextStateSlot)) {
		_emu->GetSaveStateManager()->MoveToNextSlot();
	}

	if(DetectKeyPress(EmulatorShortcut::MoveToPreviousStateSlot)) {
		_emu->GetSaveStateManager()->MoveToPreviousSlot();
	}

	if(DetectKeyPress(EmulatorShortcut::SaveState)) {
		_emu->GetSaveStateManager()->SaveState();
	}

	if(DetectKeyPress(EmulatorShortcut::LoadState)) {
		_emu->GetSaveStateManager()->LoadState();
	}

	if(DetectKeyPress(EmulatorShortcut::ToggleCheats) && !isNetplayClient && !isMovieActive) {
		_emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::ExecuteShortcut, (void*)EmulatorShortcut::ToggleCheats);
	}

	if(DetectKeyPress(EmulatorShortcut::RunSingleFrame)) {
		ProcessRunSingleFrame();
	}

	if(DetectKeyRelease(EmulatorShortcut::RunSingleFrame)) {
		_runSingleFrameRepeatTimer.reset();
		_repeatStarted = false;
	}

	if(!isNetplayClient && !isMovieRecording) {
		shared_ptr<RewindManager> rewindManager = _emu->GetRewindManager();
		if(rewindManager) {
			if(DetectKeyPress(EmulatorShortcut::ToggleRewind)) {
				if(rewindManager->IsRewinding()) {
					rewindManager->StopRewinding();
				} else {
					rewindManager->StartRewinding();
				}
			}

			if(DetectKeyPress(EmulatorShortcut::Rewind)) {
				rewindManager->StartRewinding();
			} else if(DetectKeyRelease(EmulatorShortcut::Rewind)) {
				rewindManager->StopRewinding();
			} else  if(DetectKeyPress(EmulatorShortcut::RewindTenSecs)) {
				rewindManager->RewindSeconds(10);
			} else if(DetectKeyPress(EmulatorShortcut::RewindOneMin)) {
				rewindManager->RewindSeconds(60);
			}
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

	shared_ptr<Timer> timer = _runSingleFrameRepeatTimer;
	if(timer) {
		double elapsedMs = timer->GetElapsedMS();
		if((_repeatStarted && elapsedMs >= 50) || (!_repeatStarted && elapsedMs >= 500)) {
			//Over 500ms has elapsed since the key was first pressed, or over 50ms since repeat mode started (20fps)
			//In this case, run another frame and pause again.
			_repeatStarted = true;
			ProcessRunSingleFrame();
		}
	}
}