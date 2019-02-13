#include "stdafx.h"
#include "AutoResetEvent.h"

AutoResetEvent::AutoResetEvent()
{
	_signaled = false;
}

AutoResetEvent::~AutoResetEvent()
{
	//Can't signal here, seems to cause process crashes when this occurs while the
	//application is exiting.
}

void AutoResetEvent::Wait(int timeoutDelay)
{
	std::unique_lock<std::mutex> lock(_mutex);
	if(timeoutDelay == 0) {
		//Wait until signaled
		_signal.wait(lock, [this] { return _signaled; });
	} else {
		//Wait until signaled or timeout
		auto timeoutTime = std::chrono::system_clock::now() + std::chrono::duration<int, std::milli>(timeoutDelay);
		_signal.wait_until(lock, timeoutTime, [this] { return _signaled; });
	}
	_signaled = false;
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
