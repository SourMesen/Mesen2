#include "pch.h"
#include <algorithm>
#include "Serializer.h"
#include "ISerializable.h"
#include "miniz.h"

Serializer::Serializer(uint32_t version, bool forSave, SerializeFormat format)
{
	_version = version;
	_saving = forSave;
	_format = format;
	if(forSave) {
		switch(format) {
			case SerializeFormat::Binary: _data.reserve(0x50000); break;
			case SerializeFormat::Map: _mapValues.reserve(500); break;
			case SerializeFormat::Text: _values.reserve(500); break;
		}
	}
}

void Serializer::AddKeyPrefix(string prefix)
{
	vector<string> keys;
	for(auto& kvp : _values) {
		keys.push_back(kvp.first);
	}

	for(string& key : keys) {
		_values[prefix + key] = _values[key];
		_values.erase(key);
	}
}

void Serializer::RemoveKeyPrefix(string prefix)
{
	vector<string> keys;
	vector<string> keysToRemove;

	for(auto& kvp : _values) {
		if(kvp.first.size() > prefix.size() && kvp.first.substr(0, prefix.size()) == prefix) {
			keys.push_back(kvp.first);
		} else {
			keysToRemove.push_back(kvp.first);
		}
	}

	for(string& key : keysToRemove) {
		_values.erase(key);
	}

	for(string& key : keys) {
		_values[key.substr(prefix.length())] = _values[key];
		_values.erase(key);
	}
}

void Serializer::RemoveKeys(vector<string>& keysToRemove)
{
	for(string& key : keysToRemove) {
		_values.erase(key);
	}
}

bool Serializer::LoadFrom(istream &file)
{
	if(_saving) {
		return false;
	}

	if(_format == SerializeFormat::Text) {
		return LoadFromTextFormat(file);
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
	string key;
	while(i < size) {
		key.clear();
		for(uint32_t j = i; j < size; j++) {
			if(_data[j] == 0) {
				key.append((char*)&_data[i]);
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
		if(i + 4 > size) {
			//invalid
			return false;
		}

		uint32_t valueSize = _data[i] | (_data[i + 1] << 8) | (_data[i + 2] << 16) | (_data[i + 3] << 24);
		i += 4;
		if(i + valueSize > size) {
			//invalid
			return false;
		}

		_values.emplace(key, SerializeValue(i < _data.size() ? &_data[i] : nullptr, valueSize));

		i += valueSize;
	}

	return _values.size() > 0;
}

bool Serializer::LoadFromTextFormat(istream& file)
{
	uint32_t pos = (uint32_t)file.tellg();
	file.seekg(0, std::ios::end);
	uint32_t stateSize = (uint32_t)file.tellg() - pos;
	file.seekg(pos, std::ios::beg);

	_data = vector<uint8_t>(stateSize, 0);
	file.read((char*)_data.data(), stateSize);

	uint32_t size = (uint32_t)_data.size();
	uint32_t i = 0;
	string key;
	while(i < size) {
		key.clear();
		for(uint32_t j = i; j < size; j++) {
			if(_data[j] == ' ') {
				key.append((char*)&_data[i], j - i);
				break;
			} else if(_data[j] < ' ' || _data[j] >= 127) {
				//invalid characters in key, state is invalid
				return false;
			}
		}

		if(key.empty()) {
			//invalid
			return false;
		}

		i += (uint32_t)key.size() + 1;
		if(i >= size) {
			//invalid
			return false;
		}

		uint32_t valueSize = 0;
		for(uint32_t j = i; j < size; j++) {
			if(_data[j] == '\n') {
				valueSize = j - i;
				break;
			}
		}

		if(i + valueSize > size || valueSize == 0) {
			//invalid
			return false;
		}

		_values.emplace(key, SerializeValue(&_data[i], valueSize));

		i += valueSize + 1;
	}

	return true;
}

void Serializer::SaveTo(ostream& file, int compressionLevel)
{
	if(_format == SerializeFormat::Text) {
		file.write((char*)_data.data(), _data.size());
	} else {
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
}

void Serializer::LoadFromMap(unordered_map<string, SerializeMapValue>& map)
{
	_mapValues = map;
}

string Serializer::NormalizeName(const char* name, int index)
{
	string valName = name[0] == '_' ? name + 1 : name;
	if(valName.size() > 6 && memcmp(valName.c_str(), "state.", 6) == 0) {
		valName = valName.substr(6);
	}

	for(size_t i = 0, len = valName.size(); i < len; i++) {
		char c = valName[i];
		if(c >= 'A' && c <= 'Z') {
			valName[i] = ::tolower(c);
		} else {
			size_t pos = valName.find_first_of('.', i);
			if(pos == string::npos) {
				break;
			} else {
				i = pos;
			}
		}
	}

	if(index >= 0) {
		size_t pos = valName.find("[i]");
		if(pos != string::npos) {
			valName.replace(pos, 3, "[" + std::to_string(index) + "]");
		} else {
			valName += "[" + std::to_string(index) + "]";
		}
	}

	return valName;
}

void Serializer::PushNamePrefix(const char* name, int index)
{
	_prefixes.push_back(NormalizeName(name, index));
	UpdatePrefix();
}

void Serializer::PopNamePrefix()
{
	_prefixes.pop_back();
	UpdatePrefix();
}

void Serializer::UpdatePrefix()
{
	_prefix.clear();
	for(string& prefix : _prefixes) {
		if(prefix.size()) {
			_prefix += prefix + ".";
		}
	}
}