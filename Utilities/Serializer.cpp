#include "stdafx.h"
#include <algorithm>
#include "Serializer.h"
#include "ISerializable.h"
#include "miniz.h"

Serializer::Serializer(uint32_t version)
{
	_version = version;

	_block.Data = vector<uint8_t>(0x50000);
	_block.Position = 0;
	_saving = true;
}

Serializer::Serializer(istream &file, uint32_t version)
{
	_version = version;

	_block.Position = 0;
	_saving = false;

	uint32_t decompressedSize;
	file.read((char*)&decompressedSize, sizeof(decompressedSize));

	uint32_t compressedSize;
	file.read((char*)&compressedSize, sizeof(compressedSize));

	vector<uint8_t> compressedData(compressedSize, 0);
	file.read((char*)compressedData.data(), compressedSize);

	_block.Data = vector<uint8_t>(decompressedSize, 0);

	unsigned long decompSize = decompressedSize;
	uncompress(_block.Data.data(), &decompSize, compressedData.data(), (unsigned long)compressedData.size());
}

void Serializer::EnsureCapacity(uint32_t typeSize)
{
	//Make sure the current block/stream is large enough to fit the next write
	uint32_t oldSize = (uint32_t)_block.Data.size();
	if(oldSize == 0) {
		oldSize = typeSize * 2;
	}

	uint32_t sizeRequired = _block.Position + typeSize;
	
	uint32_t newSize = oldSize;
	while(newSize < sizeRequired) {
		newSize *= 2;
	}

	_block.Data.resize(newSize);
}

void Serializer::RecursiveStream()
{
}

void Serializer::StreamStartBlock()
{
	BlockData block;
	block.Position = 0;

	if(!_saving) {
		VectorInfo<uint8_t> vectorInfo = { &block.Data };
		InternalStream(vectorInfo);
	} else {
		block.Data = vector<uint8_t>(0x100);
	}

	_blocks.push_back(_block);
	_block = block;
}

void Serializer::StreamEndBlock()
{
	if(_blocks.empty()) {
		throw std::runtime_error("Invalid call to end block");
	}

	BlockData block = _block;

	_block = _blocks.back();
	_blocks.pop_back();

	if(_saving) {
		ArrayInfo<uint8_t> arrayInfo { block.Data.data(), block.Position };
		InternalStream(arrayInfo);
	}
}

void Serializer::Save(ostream& file, int compressionLevel)
{
	unsigned long compressedSize = compressBound((unsigned long)_block.Position);
	uint8_t* compressedData = new uint8_t[compressedSize];
	compress2(compressedData, &compressedSize, (unsigned char*)_block.Data.data(), (unsigned long)_block.Position, compressionLevel);

	uint32_t size = (uint32_t)compressedSize;
	file.write((char*)&_block.Position, sizeof(uint32_t));
	file.write((char*)&size, sizeof(uint32_t));
	file.write((char*)compressedData, compressedSize);
	delete[] compressedData;
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
	StreamStartBlock();
	obj.Serialize(*this);
	StreamEndBlock();
}
void Serializer::Stream(ISerializable *obj)
{
	StreamStartBlock();
	obj->Serialize(*this);
	StreamEndBlock();
}

void Serializer::InternalStream(string &str)
{
	if(_saving) {
		vector<uint8_t> stringData;
		stringData.resize(str.size());
		memcpy(stringData.data(), str.data(), str.size());
		StreamVector(stringData);
	} else {
		vector<uint8_t> stringData;
		StreamVector(stringData);
		str = string(stringData.begin(), stringData.end());
	}
}
