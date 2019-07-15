#pragma once
#include "stdafx.h"

class FastString
{
private:
	char _buffer[1000];
	uint16_t _pos = 0;

	void Write() {}

public:
	FastString() {}
	FastString(const char* str, uint16_t size) { Write(str, size); }
	FastString(string &str) { Write(str); }

	void Write(char c)
	{
		_buffer[_pos++] = c;
	}

	void Write(const char* str, uint16_t size)
	{
		memcpy(_buffer + _pos, str, size);
		_pos += size;
	}

	void Delimiter(const char* str)
	{
		if(_pos > 0) {
			Write(str, (uint16_t)strlen(str));
		}
	}

	void Write(const char* str)
	{
		Write(str, (uint16_t)strlen(str));
	}

	void Write(string &str)
	{
		memcpy(_buffer + _pos, str.c_str(), str.size());
		_pos += (uint16_t)str.size();
	}

	void Write(FastString &str)
	{
		memcpy(_buffer + _pos, str._buffer, str._pos);
		_pos += str._pos;
	}

	const char* ToString()
	{
		_buffer[_pos] = 0;
		return _buffer;
	}

	template<typename T, typename... Args>
	void Write(T first, Args... args)
	{
		Write(first);
		Write(args...);
	}

	const char operator[](int idx)
	{
		return _buffer[idx];
	}
};
