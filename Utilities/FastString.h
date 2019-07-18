#pragma once
#include "stdafx.h"

class FastString
{
private:
	char _buffer[1000];
	uint16_t _pos = 0;

	void WriteAll() {}

public:
	FastString() {}
	FastString(const char* str, int size) { Write(str, size); }
	FastString(string &str) { Write(str); }

	void Write(char c)
	{
		_buffer[_pos++] = c;
	}

	void Write(const char* str, int size)
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

	uint16_t GetSize()
	{
		return _pos;
	}

	template<typename T, typename... Args>
	void WriteAll(T first, Args... args)
	{
		Write(first);
		WriteAll(args...);
	}

	const char operator[](int idx)
	{
		return _buffer[idx];
	}
};
