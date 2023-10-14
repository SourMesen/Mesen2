#pragma once 
#include "pch.h"

#include <condition_variable>
#include <mutex>

class AutoResetEvent
{
private:
	std::condition_variable _signal;
	std::mutex _mutex;
	bool _signaled;

public:
	AutoResetEvent();
	~AutoResetEvent();

	void Reset();
	bool Wait(int timeoutDelay = 0);
	void Signal();
};
