#pragma once
#include "pch.h"
#include <chrono>
#include "Shared/SettingTypes.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/Interfaces/IMessageManager.h"
#include "Utilities/Timer.h"
#include "Utilities/SimpleLock.h"

class MessageInfo;

class SystemHud final : public IMessageManager
{
private:
	Emulator* _emu = nullptr;

	SimpleLock _msgLock;
	list<unique_ptr<MessageInfo>> _messages;

	Timer _fpsTimer;
	Timer _animationTimer;
	uint32_t _lastFrameCount = 0;
	uint32_t _lastRenderedFrameCount = 0;
	uint32_t _currentFPS = 0;
	uint32_t _currentRenderedFPS = 0;
	uint32_t _renderedFrameCount = 0;
	
	void DrawMessages(DebugHud* hud, uint32_t screenWidth, uint32_t screenHeight) const;
	void DrawBar(DebugHud* hud, int x, int y, int width, int height) const;
	void DrawPauseIcon(DebugHud* hud) const;
	void DrawPlayIcon(DebugHud* hud) const;
	void DrawRecordIcon(DebugHud* hud) const;
	void DrawTurboRewindIcon(DebugHud* hud, bool forRewind, int xOffset) const;
	void DrawMessage(DebugHud* hud, MessageInfo& msg, uint32_t screenWidth, uint32_t screenHeight, int& lastHeight) const;
	void DrawString(DebugHud* hud, uint32_t screenWidth, string msg, int x, int y, uint8_t opacity = 255) const;
	void DisplayMessage(string title, string message) override;

	void ShowFpsCounter(DebugHud* hud, uint32_t screenWidth, int lineNumber) const;
	void ShowFrameCounter(DebugHud* hud, uint32_t screenWidth, int lineNumber) const;
	void ShowLagCounter(DebugHud* hud, uint32_t screenWidth, int lineNumber) const;
	void ShowGameTimer(DebugHud* hud, uint32_t screenWidth, int lineNumber) const;

	void DrawCounters(DebugHud* hud, uint32_t screenWidth) const;

public:
	SystemHud(Emulator* emu);
	~SystemHud();

	void Draw(DebugHud* hud, uint32_t width, uint32_t height) const;
	void UpdateHud();
};


class MessageInfo
{
private:
	string _title;
	string _message;
	uint64_t _endTime;
	uint64_t _startTime;

	uint64_t GetCurrentTime()
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	}

public:
	MessageInfo(string title, string message, int displayDuration)
	{
		_title = title;
		_message = message;
		_startTime = GetCurrentTime();
		_endTime = _startTime + displayDuration;
	}

	string GetTitle()
	{
		return _title;
	}

	string GetMessage()
	{
		return _message;
	}

	float GetOpacity()
	{
		uint64_t currentTime = GetCurrentTime();
		if(currentTime - _startTime < 100) {
			return (currentTime - _startTime) * 10.0f / 1000.0f;
		} else if(_endTime - currentTime < 200) {
			return (_endTime - currentTime) * 5.0f / 1000.0f;
		} else if(currentTime >= _endTime) {
			return 0.0f;
		} else {
			return 1.0f;
		}
	}

	bool IsExpired()
	{
		return _endTime < GetCurrentTime();
	}
};