#pragma once 
#include "pch.h"
#include <thread>

class SimpleLock;

class LockHandler
{
private:
	SimpleLock *_lock;
	bool _released = false;
public:
	LockHandler(SimpleLock *lock);
	void Release();
	~LockHandler();
};

class SimpleLock
{
private:
	thread_local static std::thread::id _threadID;

	std::thread::id _holderThreadID;
	uint32_t _lockCount;
	atomic_flag _lock;
	
	bool WaitForAcquire(uint32_t msTimeout);

public:
	SimpleLock();
	~SimpleLock();

	LockHandler AcquireSafe();

	void Acquire();
	bool TryAcquire(uint32_t msTimeout);
	bool IsFree();
	bool IsLockedByCurrentThread();
	void WaitForRelease();
	void Release();
};

