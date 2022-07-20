#pragma once

#include "stdafx.h"
#include "Utilities/ISerializable.h"

class Serializer;

#define SV(var) (s.Stream(var, #var))
#define SVArray(arr, count) (s.StreamArray(arr, count, #arr))
#define SVI(var) (s.Stream(var, #var, i))

#define SVVector(var) (s.Stream(var, #var))
#define SVVectorI(var) (s.Stream(var, #var, i))

struct SerializeValue
{
	string Key;
	uint8_t* DataPtr;
	uint32_t Size;

	SerializeValue()
	{}

	SerializeValue(uint8_t* ptr, uint32_t size, string key)
	{
		DataPtr = ptr;
		Size = size;
		Key = key;
	}
};

class Serializer
{
private:
	vector<uint8_t> _data;
	vector<string> _prefixes;

	unordered_set<string> _usedKeys;
	unordered_map<string, SerializeValue> _values;

	uint32_t _version = 0;
	bool _saving = false;

private:
	string GetKey(const char* name, int index)
	{
		string key;
		for(string& prefix : _prefixes) {
			if(prefix.size()) {
				key += prefix + ".";
			}
		}

		string valName = name[0] == '_' ? name + 1 : name;
		if(valName.size() > 6 && valName.substr(0, 6) == "state.") {
			valName = valName.substr(6);
		}

		if(valName.empty()) {
			throw std::runtime_error("invalid value name");
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

		return key + valName;
	}

	template<typename T>
	void WriteValue(T value)
	{
		uint8_t* ptr = (uint8_t*)&value;
		bool isBigEndian = false;
		int mask = isBigEndian ? sizeof(T) - 1 : 0;
		for(int i = 0; i < sizeof(T); i++) {
			_data.push_back(ptr[i ^ mask]);
		}
	}

	template<typename T>
	void ReadValue(T& value, uint8_t* src)
	{
		uint8_t* ptr = (uint8_t*)&value;
		bool isBigEndian = false;
		int mask = isBigEndian ? sizeof(T) - 1 : 0;
		for(int i = 0; i < sizeof(T); i++) {
			ptr[i ^ mask] = src[i];
		}
	}

public:
	Serializer(uint32_t version);
	Serializer(istream &file, uint32_t version, bool compressed = true);

	uint32_t GetVersion() { return _version; }
	bool IsSaving() { return _saving; }

	template <class T> struct is_unique_ptr : std::false_type {};
	template <class T, class D> struct is_unique_ptr<std::unique_ptr<T, D>> : std::true_type {};
	template <class T> struct is_shared_ptr : std::false_type {};
	template <class T> struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

	template<typename T> void Stream(T& value, const char* name, int index = -1)
	{
		static_assert(!is_unique_ptr<std::remove_cv_t<T>>::value, "Unexpected unique_ptr");
		static_assert(!is_shared_ptr<std::remove_cv_t<T>>::value, "Unexpected shared_ptr");
		static_assert(!std::is_pointer<T>::value, "Unexpected pointer");
		
		if constexpr(std::is_base_of<ISerializable, T>::value) {
			Stream((ISerializable&)value, name, index);
		} else {
			string key = GetKey(name, index);
			if(!_usedKeys.emplace(key).second) {
				throw std::runtime_error("Duplicate key");
			}

			if(_saving) {
				//Write key
				_data.insert(_data.end(), key.begin(), key.end());
				_data.push_back(0);

				//Write value size
				WriteValue((uint32_t)sizeof(T));

				//Write value
				WriteValue(value);
			} else {
				auto result = _values.find(key);
				if(result != _values.end()) {
					SerializeValue& savedValue = result->second;
					if(savedValue.Size >= sizeof(T)) {
						ReadValue(value, savedValue.DataPtr);
					} else {
						value = (T)0;
					}
				}
			}
		}
	}

	void Stream(ISerializable& obj, const char* name, int index)
	{
		PushNamePrefix(name, index);
		obj.Serialize(*this);
		PopNamePrefix();
	}

	template<typename T> void Stream(unique_ptr<T>& obj, const char* name, int index = -1)
	{
		static_assert(std::is_assignable<ISerializable, T>::value);
		PushNamePrefix(name, index);
		((ISerializable*)obj.get())->Serialize(*this);
		PopNamePrefix();
	}

	template<typename T> void Stream(const unique_ptr<T>& obj, const char* name, int index = -1)
	{
		static_assert(std::is_assignable<ISerializable, T>::value);
		PushNamePrefix(name, index);
		((ISerializable*)obj.get())->Serialize(*this);
		PopNamePrefix();
	}

	template<typename T> void Stream(shared_ptr<T>& obj, const char* name, int index = -1)
	{
		static_assert(std::is_assignable<ISerializable, T>::value);
		PushNamePrefix(name, index);
		((ISerializable*)obj.get())->Serialize(*this);
		PopNamePrefix();
	}

	template<typename T> void StreamArray(T* arrayValues, uint32_t elementCount, const char* name)
	{
		string key = GetKey(name, -1);
		if(!_usedKeys.emplace(key).second) {
			throw std::runtime_error("Duplicate key");
		}

		if(_saving) {
			//Write key
			_data.insert(_data.end(), key.begin(), key.end());
			_data.push_back(0);

			//Write array size
			WriteValue((uint32_t)(elementCount * sizeof(T)));

			//Write array content
			for(uint32_t i = 0; i < elementCount; i++) {
				WriteValue(arrayValues[i]);
			}
		} else {
			auto result = _values.find(key);
			if(result != _values.end()) {
				SerializeValue& savedValue = result->second;
				if(savedValue.Size >= elementCount * sizeof(T)) {
					uint8_t* src = savedValue.DataPtr;
					for(uint32_t i = 0; i < elementCount; i++) {
						ReadValue(arrayValues[i], src);
						src += sizeof(T);
					}
				} else {
					memset(arrayValues, 0, sizeof(T) * elementCount);
				}
			}
		}
	}

	template<typename T> void Stream(vector<T>& values, const char* name, int index = -1)
	{
		string key = GetKey(name, index);
		if(!_usedKeys.emplace(key).second) {
			throw std::runtime_error("Duplicate key");
		}

		if(_saving) {
			//Write key
			_data.insert(_data.end(), key.begin(), key.end());
			_data.push_back(0);

			uint32_t elementCount = (uint32_t)values.size();
			//Write array size
			WriteValue((uint32_t)(elementCount * sizeof(T)));

			//Write array content
			for(uint32_t i = 0; i < elementCount; i++) {
				WriteValue(values[i]);
			}
		} else {
			auto result = _values.find(key);
			if(result != _values.end()) {
				SerializeValue& savedValue = result->second;
				uint32_t elementCount = savedValue.Size / sizeof(T);
				values.resize(elementCount);

				uint8_t* src = savedValue.DataPtr;
				for(uint32_t i = 0; i < elementCount; i++) {
					ReadValue(values[i], src);
					src += sizeof(T);
				}
			}
		}
	}

	template<> void Stream(string& value, const char* name, int index)
	{
		string key = GetKey(name, index);
		if(!_usedKeys.emplace(key).second) {
			throw std::runtime_error("Duplicate key");
		}

		if(_saving) {
			//Write key
			_data.insert(_data.end(), key.begin(), key.end());
			_data.push_back(0);

			//Write string size
			WriteValue((uint32_t)value.size());

			//Write string content
			_data.insert(_data.end(), value.begin(), value.end());
		} else {
			auto result = _values.find(key);
			if(result != _values.end()) {
				SerializeValue& savedValue = result->second;
				value = string(savedValue.DataPtr, savedValue.DataPtr + savedValue.Size);
			}
		}
	}

	void PushNamePrefix(const char* name, int index = -1);
	void PopNamePrefix();
	void Save(ostream &file, int compressionLevel = 1);
};
