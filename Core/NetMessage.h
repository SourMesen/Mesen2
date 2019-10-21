#pragma once
#include "stdafx.h"
#include "MessageType.h"
#include "../Utilities/Socket.h"

class NetMessage
{
protected:
	MessageType _type;
	bool _sending;
	vector<uint8_t> _buffer;
	uint32_t _position = 0;
	vector<uint8_t*> _arraysToRelease;
	vector<char*> _stringsToRelease;

	template<typename T>
	void Stream(T &value)
	{
		if(_sending) {
			uint8_t* bytes = (uint8_t*)&value;
			int typeSize = sizeof(T);
			for(int i = 0; i < typeSize; i++) {
				_buffer.push_back(bytes[i]);
			}
		} else {
			memcpy(&value, _buffer.data()+_position, sizeof(T));
			_position += sizeof(T);
		}
	}

	void StreamArray(void* value, uint32_t length)
	{
		void* pointer = value;
		uint32_t len = length;
		StreamArray(&pointer, len);
	}

	void StreamArray(void** value, uint32_t &length)
	{
		if(_sending) {
			Stream<uint32_t>(length);
			uint8_t* bytes = (uint8_t*)(*value);
			for(uint32_t i = 0, len = length; i < len; i++) {
				_buffer.push_back(bytes[i]);
				_position++;
			}
		} else {
			Stream<uint32_t>(length);
			if(*value == nullptr) {
				*value = (void*)new uint8_t[length];
				_arraysToRelease.push_back((uint8_t*)*value);
			}
			uint8_t* bytes = (uint8_t*)(*value);
			for(uint32_t i = 0, len = length; i < len; i++) {
				bytes[i] = _buffer[_position];
				_position++;
			}
		}
	}

	void StreamArray(vector<uint8_t> &data)
	{
		uint32_t length = (uint32_t)data.size();
		Stream<uint32_t>(length);
		if(_sending) {
			uint8_t* bytes = (uint8_t*)data.data();
			for(uint32_t i = 0, len = length; i < len; i++) {
				_buffer.push_back(bytes[i]);
				_position++;
			}
		} else {
			data.resize(length, 0);
			uint8_t* bytes = (uint8_t*)data.data();
			for(uint32_t i = 0, len = length; i < len; i++) {
				bytes[i] = _buffer[_position];
				_position++;
			}
		}
	}

	void StreamState()
	{
		Stream<MessageType>(_type);
		ProtectedStreamState();
	}

	NetMessage(MessageType type)
	{
		_type = type;
		_sending = true;
	}

	NetMessage(void* buffer, uint32_t length)
	{
		_buffer.assign((uint8_t*)buffer, (uint8_t*)buffer + length);
		_sending = false;
	}

public:
	virtual ~NetMessage() 
	{	
		for(uint8_t *arrayPtr: _arraysToRelease) {
			delete[] arrayPtr;
		}
		for(char *stringPtr: _stringsToRelease) {
			delete[] stringPtr;
		}
	}

	void Initialize()
	{
		StreamState();
	}

	MessageType GetType()
	{
		return _type;
	}

	void Send(Socket &socket)
	{
		StreamState();
		uint32_t messageLength = (uint32_t)_buffer.size();

		_buffer.insert(_buffer.begin(), (char*)&messageLength, (char*)&messageLength + sizeof(messageLength));
		socket.Send((char*)_buffer.data(), (int)_buffer.size(), 0);
	}

	void CopyString(char** dest, uint32_t &length, string src)
	{
		length = (uint32_t)(src.length() + 1);
		*dest = new char[length];
		memcpy(*dest, src.c_str(), length);
		_stringsToRelease.push_back(*dest);
	}

protected:
	virtual void ProtectedStreamState() = 0;
};
