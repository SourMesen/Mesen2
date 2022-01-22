#pragma once
#include "stdafx.h"
#include <chrono>
#include "Shared/SettingTypes.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/Interfaces/IMessageManager.h"
#include "Utilities/Timer.h"
#include "Utilities/SimpleLock.h"

class MessageInfo;

class SystemHud : public IMessageManager
{
private:
	Emulator* _emu = nullptr;
	DebugHud* _hud = nullptr;

	SimpleLock _msgLock;
	list<unique_ptr<MessageInfo>> _messages;

	Timer _fpsTimer;
	uint32_t _lastFrameCount = 0;
	uint32_t _lastRenderedFrameCount = 0;
	uint32_t _currentFPS = 0;
	uint32_t _currentRenderedFPS = 0;
	uint32_t _renderedFrameCount = 0;
	
	uint32_t _screenWidth = 0;
	uint32_t _screenHeight = 0;


	void DrawMessages();
	void DrawBar(int x, int y, int width, int height);
	void DrawPauseIcon();
	void DrawMessage(MessageInfo& msg, int& lastHeight);
	void DrawString(string msg, int x, int y, uint8_t opacity = 255);
	void DisplayMessage(string title, string message) override;

	void ShowFpsCounter(int lineNumber);
	void ShowFrameCounter(int lineNumber);
	void ShowGameTimer(int lineNumber);

	void DrawCounters();

public:
	SystemHud(Emulator* emu, DebugHud* hud);
	~SystemHud();

	void Draw(uint32_t width, uint32_t height);
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
		if(currentTime - _startTime < 200) {
			return (currentTime - _startTime) * 5.0f / 1000.0f;
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