#pragma once

#include "stdafx.h"
#include <chrono>

class ToastInfo;

class IMessageManager
{
public:
	virtual void DisplayMessage(string title, string message) = 0;
};

class ToastInfo
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
	ToastInfo(string title, string message, int displayDuration)
	{
		_title = title;
		_message = message;
		_startTime = GetCurrentTime();
		_endTime = _startTime + displayDuration;
	}

	string GetToastTitle()
	{
		return _title;
	}

	string GetToastMessage() 
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

	bool IsToastExpired()
	{
		return _endTime < GetCurrentTime();
	}
};