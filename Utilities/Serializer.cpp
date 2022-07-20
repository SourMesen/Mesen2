#include "stdafx.h"
#include <algorithm>
#include "Serializer.h"
#include "ISerializable.h"
#include "miniz.h"

Serializer::Serializer(uint32_t version, bool forSave)
{
	_version = version;
	_saving = forSave;
	if(forSave) {
		_data.reserve(0x50000);
	}
}

bool Serializer::LoadFrom(istream &file)
{
	if(_saving) {
		return false;
	}

	char value = 0;
	file.get(value);
	bool isCompressed = value == 1;

	if(isCompressed) {
		uint32_t decompressedSize;
		file.read((char*)&decompressedSize, sizeof(decompressedSize));

		uint32_t compressedSize;
		file.read((char*)&compressedSize, sizeof(compressedSize));

		if(decompressedSize >= 1024 * 1024 * 10 || compressedSize >= 1024 * 1024 * 10) {
			//Limit to 10mb the data's size
			return false;
		}

		vector<uint8_t> compressedData(compressedSize, 0);
		file.read((char*)compressedData.data(), compressedSize);

		_data = vector<uint8_t>(decompressedSize, 0);

		unsigned long decompSize = decompressedSize;
		if(uncompress(_data.data(), &decompSize, compressedData.data(), (unsigned long)compressedData.size()) != MZ_OK) {
			return false;
		}
	} else {
		uint32_t pos = (uint32_t)file.tellg();
		file.seekg(0, std::ios::end);
		uint32_t stateSize = (uint32_t)file.tellg() - pos;
		file.seekg(pos, std::ios::beg);

		_data = vector<uint8_t>(stateSize, 0);
		file.read((char*)_data.data(), stateSize);
	}

	uint32_t size = (uint32_t)_data.size();
	uint32_t i = 0;
	while(i < size) {
		string key;
		for(uint32_t j = i; j < size; j++) {
			if(_data[j] == 0) {
				key = string(&_data[i], &_data[j]);
				break;
			} else if(_data[j] <= ' ' || _data[j] >= 127) {
				//invalid characters in key, state is invalid
				return false;
			}
		}

		if(key.empty()) {
			//invalid
			return false;
		}

		i += (uint32_t)key.size() + 1;
		if(i >= size - 4) {
			//invalid
			return false;
		}

		uint32_t valueSize = _data[i] | (_data[i + 1] << 8) | (_data[i + 2] << 16) | (_data[i + 3] << 24);
		i += 4;
		if(i + valueSize > size) {
			//invalid
			return false;
		}

		_values[key] = SerializeValue(&_data[i], valueSize, key);

		i += valueSize;
	}

	return _values.size() > 0;
}

void Serializer::SaveTo(ostream& file, int compressionLevel)
{
	bool isCompressed = compressionLevel > 0;
	file.put((char)isCompressed);

	if(isCompressed) {
		unsigned long compressedSize = compressBound((unsigned long)_data.size());
		uint8_t* compressedData = new uint8_t[compressedSize];
		compress2(compressedData, &compressedSize, (unsigned char*)_data.data(), (unsigned long)_data.size(), compressionLevel);

		uint32_t size = (uint32_t)compressedSize;
		uint32_t originalSize = (uint32_t)_data.size();
		file.write((char*)&originalSize, sizeof(uint32_t));
		file.write((char*)&size, sizeof(uint32_t));
		file.write((char*)compressedData, compressedSize);
		delete[] compressedData;
	} else {
		file.write((char*)_data.data(), _data.size());
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

