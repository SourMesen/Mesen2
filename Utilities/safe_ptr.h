#pragma once
#include "stdafx.h"
#include "Utilities/SimpleLock.h"

template<typename T>
class safe_ptr
{
private:
	T* _ptr = nullptr;
	shared_ptr<T> _shared;
	SimpleLock _lock;

public:
	safe_ptr(T* ptr = nullptr)
	{
		reset(ptr);
	}

	~safe_ptr()
	{
		//shared_ptr will free the instance as needed
	}

	shared_ptr<T> lock()
	{
		auto lock = _lock.AcquireSafe();
		return _shared;
	}

	void reset(T* ptr = nullptr)
	{
		auto lock = _lock.AcquireSafe();
		_ptr = ptr;
		_shared.reset(ptr);
	}

	void reset(shared_ptr<T> ptr)
	{
		auto lock = _lock.AcquireSafe();
		_ptr = ptr.get();
		_shared = ptr;
	}

	operator bool() const
	{
		return _ptr != nullptr;
	}

	T* operator->() const
	{
		//Accessing the pointer this way is fast, but not thread-safe
		return _ptr;
	}
};

template<typename T>
bool operator==(const safe_ptr<T>& ptr, nullptr_t)
{
	return !(bool)ptr;
}

template<typename T>
bool operator!=(const safe_ptr<T>& ptr, nullptr_t)
{
	return (bool)ptr;
}