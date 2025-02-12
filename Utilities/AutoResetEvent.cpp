#include "pch.h"
#include "AutoResetEvent.h"
#include <chrono>

AutoResetEvent::AutoResetEvent()
{
	_signaled = false;
}

AutoResetEvent::~AutoResetEvent()
{
	//Can't signal here, seems to cause process crashes when this occurs while the
	//application is exiting.
}

bool AutoResetEvent::Wait(int timeoutDelay)
{
	std::unique_lock<std::mutex> lock(_mutex);
	bool receivedSignal = false;
	if(timeoutDelay == 0) {
		//Wait until signaled
		_signal.wait(lock, [this] { return _signaled; });
		receivedSignal = true;
	} else {
		//Wait until signaled or timeout
		auto timeoutTime = std::chrono::system_clock::now() + std::chrono::duration<int, std::milli>(timeoutDelay);
		receivedSignal = _signal.wait_until(lock, timeoutTime, [this] { return _signaled; });
	}
	_signaled = false;
	return receivedSignal;
}

void AutoResetEvent::Reset()
{
	std::unique_lock<std::mutex> lock(_mutex);
	_signaled = false;
}

void AutoResetEvent::Signal()
{
	std::unique_lock<std::mutex> lock(_mutex);
	_signaled = true;
	_signal.notify_all();
}
