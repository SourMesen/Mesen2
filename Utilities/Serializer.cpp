#include "stdafx.h"
#include <algorithm>
#include "Serializer.h"
#include "ISerializable.h"

Serializer::Serializer(uint32_t version)
{
	_version = version;

	_streamSize = 0x50000;
	_stream = new uint8_t[_streamSize];
	_position = 0;
	_saving = true;
}

Serializer::Serializer(istream &file, uint32_t version)
{
	_version = version;

	_position = 0;
	_saving = false;

	file.read((char*)&_streamSize, sizeof(_streamSize));
	_stream = new uint8_t[_streamSize];
	file.read((char*)_stream, _streamSize);
}

Serializer::~Serializer()
{
	delete[] _stream;
}

void Serializer::EnsureCapacity(uint32_t typeSize)
{
	//Make sure the current block/stream is large enough to fit the next write
	uint32_t oldSize;
	uint32_t sizeRequired;
	uint8_t *oldBuffer;
	if(_inBlock) {
		oldBuffer = _blockBuffer;
		oldSize = _blockSize;
		sizeRequired = _blockPosition + typeSize;
	} else {
		oldBuffer = _stream;
		oldSize = _streamSize;
		sizeRequired = _position + typeSize;
	}

	uint8_t *newBuffer = nullptr;
	uint32_t newSize = oldSize * 2;
	if(oldSize < sizeRequired) {
		while(newSize < sizeRequired) {
			newSize *= 2;
		}

		newBuffer = new uint8_t[newSize];
		memcpy(newBuffer, oldBuffer, oldSize);
		delete[] oldBuffer;
	}

	if(newBuffer) {
		if(_inBlock) {
			_blockBuffer = newBuffer;
			_blockSize = newSize;
		} else {
			_stream = newBuffer;
			_streamSize = newSize;
		}
	}
}

void Serializer::RecursiveStream()
{
}

void Serializer::StreamStartBlock()
{
	if(_inBlock) {
		throw new std::runtime_error("Cannot start a new block before ending the last block");
	}

	if(!_saving) {
		InternalStream(_blockSize);
		_blockSize = std::min(_blockSize, (uint32_t)0xFFFFF);
		_blockBuffer = new uint8_t[_blockSize];
		ArrayInfo<uint8_t> arrayInfo = { _blockBuffer, _blockSize };
		InternalStream(arrayInfo);
	} else {
		_blockSize = 0x100;
		_blockBuffer = new uint8_t[_blockSize];
	}
	_blockPosition = 0;
	_inBlock = true;
}

void Serializer::StreamEndBlock()
{
	_inBlock = false;
	if(_saving) {
		InternalStream(_blockPosition);
		ArrayInfo<uint8_t> arrayInfo = { _blockBuffer, _blockPosition };
		InternalStream(arrayInfo);
	}

	delete[] _blockBuffer;
	_blockBuffer = nullptr;
}

void Serializer::Save(ostream& file)
{
	file.write((char*)&_position, sizeof(_position));
	file.write((char*)_stream, _position);
}

void Serializer::WriteEmptyBlock(ostream* file)
{
	int blockSize = 0;
	file->write((char*)&blockSize, sizeof(blockSize));
}

void Serializer::SkipBlock(istream* file)
{
	int blockSize = 0;
	file->read((char*)&blockSize, sizeof(blockSize));
	file->seekg(blockSize, std::ios::cur);
}

void Serializer::Stream(ISerializable &obj)
{
	//StreamStartBlock();
	obj.Serialize(*this);
	//StreamEndBlock();
}

void Serializer::Stream(ISerializable *obj)
{
	//StreamStartBlock();
	obj->Serialize(*this);
	//StreamEndBlock();
}