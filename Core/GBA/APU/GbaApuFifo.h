#pragma once
#include "pch.h"
#include "Utilities/Serializer.h"

struct GbaApuFifo : ISerializable
{
private:
	uint32_t _data[7] = {};
	uint8_t _readPos = 0;
	uint8_t _writePos = 0;
	
	uint32_t _writeBuffer = 0;
	uint32_t _readValue = 0;
	uint8_t _readCount = 0;
	uint8_t _size = 0;

public:
	void Push(uint8_t value, uint8_t pos, bool commit)
	{
		_writeBuffer = (_writeBuffer & ~(0xFF << (pos * 8))) | (value << (pos*8));
		
		if(commit) {
			if(_size >= 7) {
				//TODOGBA is this correct?
				return;
			}

			_data[_writePos] = _writeBuffer;
			_writePos = (_writePos + 1) % 7;
			_size++;
		}
	}

	uint8_t Pop()
	{
		if(_readCount == 0) {
			_readValue = _data[_readPos];
			_readPos = (_readPos + 1) % 7;
			_size--;
		}
		uint8_t value = (uint8_t)_readValue;
		_readCount = (_readCount + 1) & 0x03;
		_readValue >>= 8;
		return value;
	}

	uint8_t Size()
	{
		return _size;
	}

	bool Empty()
	{
		return _size == 0 && _readCount == 0;
	}

	void Clear()
	{
		_writePos = 0;
		_readPos = 0;
		_readCount = 0;
		_size = 0;
		_readValue = 0;
		_writeBuffer = 0;
	}

	void Serialize(Serializer& s) override
	{
		SVArray(_data, 7);
		SV(_readPos);
		SV(_writePos);
		SV(_readCount);
		SV(_size);
		SV(_readValue);
		SV(_writeBuffer);
	}
};
