#pragma once
#include "pch.h"
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
		if(_ptr) {
			//Only need to take the lock if _ptr is still set
			//Otherwise the shared_ptr is empty and there's no need to copy it
			auto lock = _lock.AcquireSafe();
			return _shared;
		} else {
			return nullptr;
		}
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

	void reset(unique_ptr<T>& ptr)
	{
		auto lock = _lock.AcquireSafe();
		_ptr = ptr.get();
		_shared.reset(ptr.get());
		ptr.release();
	}

	T* get()
	{
		return _ptr;
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
bool operator==(const safe_ptr<T>& ptr, std::nullptr_t)
{
	return !(bool)ptr;
}

template<typename T>
bool operator!=(const safe_ptr<T>& ptr, std::nullptr_t)
{
	return (bool)ptr;
}