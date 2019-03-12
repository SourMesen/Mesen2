#pragma once

#include "stdafx.h"

class Serializer;
class ISerializable;

template<typename T>
struct ArrayInfo
{
	T* Array;
	uint32_t ElementCount;
};

template<typename T>
struct VectorInfo
{
	vector<T>* Vector;
};

template<typename T>
struct ValueInfo
{
	T* Value;
	T DefaultValue;
};

template<typename T>
struct EmptyInfo
{
	T Empty;
};

class Serializer
{
private:
	uint8_t* _stream = nullptr;
	uint32_t _position = 0;
	uint32_t _streamSize = 0;
	uint32_t _version = 0;

	bool _inBlock = false;
	uint8_t* _blockBuffer = nullptr;
	uint32_t _blockSize = 0;
	uint32_t _blockPosition = 0;

	bool _saving = false;

private:
	void EnsureCapacity(uint32_t typeSize);

	template<typename T> void StreamElement(T &value, T defaultValue = T());
	template<typename T> void InternalStream(EmptyInfo<T> &info);
	template<typename T> void InternalStream(ArrayInfo<T> &info);
	template<typename T> void InternalStream(VectorInfo<T> &info);
	template<typename T> void InternalStream(ValueInfo<T> &info);
	template<typename T> void InternalStream(T &value);
	void RecursiveStream();

	template<typename T, typename... T2> void RecursiveStream(T &value, T2&... args);

	void StreamStartBlock();
	void StreamEndBlock();

public:
	Serializer(uint32_t version);
	Serializer(istream &file, uint32_t version);
	~Serializer();

	uint32_t GetVersion() { return _version; }
	bool IsSaving() { return _saving; }

	template<typename... T> void Stream(T&... args);
	template<typename T> void StreamArray(T *array, uint32_t size);
	template<typename T> void StreamVector(vector<T> &list);

	void Save(ostream &file);

	void Stream(ISerializable &obj);
	void Stream(ISerializable *obj);

	void WriteEmptyBlock(ostream* file);
	void SkipBlock(istream* file);
};

template<typename T>
void Serializer::StreamElement(T &value, T defaultValue)
{
	if(_saving) {
		uint8_t* bytes = (uint8_t*)&value;
		int typeSize = sizeof(T);

		EnsureCapacity(typeSize);

		for(int i = 0; i < typeSize; i++) {
			if(_inBlock) {
				_blockBuffer[_blockPosition++] = bytes[i];
			} else {
				_stream[_position++] = bytes[i];
			}
		}
	} else {
		if(_inBlock) {
			if(_blockPosition + sizeof(T) <= _blockSize) {
				memcpy(&value, _blockBuffer + _blockPosition, sizeof(T));
				_blockPosition += sizeof(T);
			} else {
				value = defaultValue;
				_blockPosition = _blockSize;
			}
		} else {
			if(_position + sizeof(T) <= _streamSize) {
				memcpy(&value, _stream + _position, sizeof(T));
				_position += sizeof(T);
			} else {
				value = defaultValue;
				_position = _streamSize;
			}
		}
	}
}

template<typename T>
void Serializer::InternalStream(EmptyInfo<T> &info)
{
	if(_inBlock) {
		_blockPosition += sizeof(T);
	} else {
		_position += sizeof(T);
	}
}

template<typename T>
void Serializer::InternalStream(ArrayInfo<T> &info)
{
	T* pointer = info.Array;

	uint32_t count = info.ElementCount;
	StreamElement<uint32_t>(count);

	if(!_saving) {
		//Reset array to 0 before loading from file
		memset(info.Array, 0, sizeof(T) * info.ElementCount);
	}

	//Load the number of elements requested, or the maximum possible (based on what is present in the save state)
	for(uint32_t i = 0; i < info.ElementCount && i < count; i++) {
		StreamElement<T>(*pointer);
		pointer++;
	}
}

template<typename T>
void Serializer::InternalStream(VectorInfo<T> &info)
{
	vector<T> *vector = info.Vector;

	uint32_t count = (uint32_t)vector->size();
	StreamElement<uint32_t>(count);

	if(!_saving) {
		if(count > 0xFFFFFF) {
			throw std::runtime_error("Invalid save state");
		}
		vector->resize(count);
		memset(vector->data(), 0, sizeof(T)*count);
	}

	//Load the number of elements requested
	T* pointer = vector->data();
	for(uint32_t i = 0; i < count; i++) {
		StreamElement<T>(*pointer);
		pointer++;
	}
}

template<typename T>
void Serializer::InternalStream(ValueInfo<T> &info)
{
	StreamElement<T>(*info.Value, info.DefaultValue);
}

template<typename T>
void Serializer::InternalStream(T &value)
{
	StreamElement<T>(value);
}

template<typename T, typename... T2>
void Serializer::RecursiveStream(T &value, T2&... args)
{
	InternalStream(value);
	RecursiveStream(args...);
}

template<typename... T>
void Serializer::Stream(T&... args)
{
	//StreamStartBlock();
	RecursiveStream(args...);
	//StreamEndBlock();
}

template<typename T>
void Serializer::StreamArray(T *array, uint32_t size)
{
	ArrayInfo<T> info;
	info.Array = array;
	info.ElementCount = size;
	InternalStream(info);
}

template<typename T>
void Serializer::StreamVector(vector<T> &list)
{
	VectorInfo<T> info;
	info.Vector = &list;
	InternalStream(info);
}