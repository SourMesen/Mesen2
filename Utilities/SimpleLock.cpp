#include "pch.h"
#include <assert.h>
#include "SimpleLock.h"
#include <Timer.h>

thread_local std::thread::id SimpleLock::_threadID = std::this_thread::get_id();

SimpleLock::SimpleLock()
{
	_lock.clear();
	_lockCount = 0;
	_holderThreadID = std::thread::id();
}

SimpleLock::~SimpleLock()
{
}

LockHandler SimpleLock::AcquireSafe()
{
	return LockHandler(this);
}

void SimpleLock::Acquire()
{
	if(_lockCount == 0 || _holderThreadID != _threadID) {
		WaitForAcquire(0);
		_holderThreadID = _threadID;
		_lockCount = 1;
	} else {
		//Same thread can acquire the same lock multiple times
		_lockCount++;
	}
}

bool SimpleLock::WaitForAcquire(uint32_t msTimeout)
{
	if(_lock.test_and_set()) {
		Timer timer;
		int attempts = 0;
		while(_lock.test_and_set()) {
			if(++attempts >= 10000) {
				//Sleep between attempts after failing to acquire lock 10k times
				std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(msTimeout > 0 ? 10 : 1));
			}
			if(msTimeout > 0 && timer.GetElapsedMS() > msTimeout) {
				return false;
			}
		}
	}
	return true;
}

bool SimpleLock::TryAcquire(uint32_t msTimeout)
{
	if(_lockCount == 0 || _holderThreadID != _threadID) {
		if(!WaitForAcquire(msTimeout)) {
			return false;
		}
		_holderThreadID = _threadID;
		_lockCount = 1;
	} else {
		//Same thread can acquire the same lock multiple times
		_lockCount++;
	}

	return true;
}

bool SimpleLock::IsFree()
{
	return _lockCount == 0;
}

bool SimpleLock::IsLockedByCurrentThread()
{
	return _lockCount > 0 && _holderThreadID == _threadID;
}

void SimpleLock::WaitForRelease()
{
	//Wait until we are able to grab a lock, and then release it again
	Acquire();
	Release();
}

void SimpleLock::Release()
{
	if(IsLockedByCurrentThread()) {
		_lockCount--;
		if(_lockCount == 0) {
			_holderThreadID = std::thread::id();
			_lock.clear();
		}
	} else {
		assert(false);
	}
}


LockHandler::LockHandler(SimpleLock *lock)
{
	_lock = lock;
	_lock->Acquire();
}

void LockHandler::Release()
{
	if(!_released) {
		_released = true;
		_lock->Release();
	}
}

LockHandler::~LockHandler()
{
	if(!_released) {
		_lock->Release();
	}
}