#include "stdafx.h"
#include <algorithm>
#include "Serializer.h"
#include "ISerializable.h"
#include "miniz.h"

Serializer::Serializer(uint32_t version)
{
	_version = version;

	_data.reserve(0x50000);
	_saving = true;
}

Serializer::Serializer(istream &file, uint32_t version, bool compressed)
{
	_version = version;
	_saving = false;

	if(compressed) {
		uint32_t decompressedSize;
		file.read((char*)&decompressedSize, sizeof(decompressedSize));

		uint32_t compressedSize;
		file.read((char*)&compressedSize, sizeof(compressedSize));

		vector<uint8_t> compressedData(compressedSize, 0);
		file.read((char*)compressedData.data(), compressedSize);

		_data = vector<uint8_t>(decompressedSize, 0);

		unsigned long decompSize = decompressedSize;
		if(uncompress(_data.data(), &decompSize, compressedData.data(), (unsigned long)compressedData.size()) != MZ_OK) {
			return;
		}
	} else {
		file.seekg(0, std::ios::end);
		uint32_t size = (uint32_t)file.tellg();
		file.seekg(0, std::ios::beg);

		_data = vector<uint8_t>(size, 0);
		file.read((char*)_data.data(), size);
	}

	uint32_t size = (uint32_t)_data.size();
	uint32_t i = 0;
	while(i < size) {
		string key = (char*)&_data[i];
		i += (uint32_t)key.size() + 1;
		if(i >= size - 4) {
			//invalid
			break;
		}

		uint32_t valueSize = _data[i] | (_data[i + 1] << 8) | (_data[i + 2] << 16) | (_data[i + 3] << 24);
		i += 4;
		if(i + valueSize > size) {
			//invalid
			break;
		}

		_values[key] = SerializeValue(&_data[i], valueSize, key);

		i += valueSize;
	}
}

void Serializer::Save(ostream& file, int compressionLevel)
{
	if(compressionLevel == 0) {
		file.write((char*)_data.data(), _data.size());
	} else {
		unsigned long compressedSize = compressBound((unsigned long)_data.size());
		uint8_t* compressedData = new uint8_t[compressedSize];
		compress2(compressedData, &compressedSize, (unsigned char*)_data.data(), (unsigned long)_data.size(), compressionLevel);

		uint32_t size = (uint32_t)compressedSize;
		uint32_t originalSize = (uint32_t)_data.size();
		file.write((char*)&originalSize, sizeof(uint32_t));
		file.write((char*)&size, sizeof(uint32_t));
		file.write((char*)compressedData, compressedSize);
		delete[] compressedData;
	}
}

void Serializer::PushNamePrefix(const char* name, int index)
{
	string valName = name;
	if(index >= 0) {
		size_t pos = valName.find("[i]");
		if(pos != string::npos) {
			valName.replace(pos, 3, "[" + std::to_string(index) + "]");
		}
	}
	_prefixes.push_back(name[0] == '_' ? valName.substr(1) : valName);
}

void Serializer::PopNamePrefix()
{
	_prefixes.pop_back();
}

