#pragma once

#include "stdafx.h"
#include "Utilities/ISerializable.h"
#include "Utilities/FastString.h"
#include "Utilities/magic_enum.hpp"

class Serializer;

#define SV(var) (s.Stream(var, #var))
#define SVArray(arr, count) (s.StreamArray(arr, count, #arr))
#define SVI(var) (s.Stream(var, #var, i))

#define SVVector(var) (s.Stream(var, #var))
#define SVVectorI(var) (s.Stream(var, #var, i))

enum class SerializeMapValueFormat
{
	Integer,
	Double,
	Bool,
	String
};

union MapValue
{
	int64_t Integer;
	double Double;
	bool Bool;

	MapValue(bool b) { Bool = b; }
	MapValue(double d) { Double = d; }
	MapValue(int64_t i) { Integer = i; }
};

struct SerializeMapValue
{
	SerializeMapValueFormat Format;
	MapValue Value;
	string StringValue;

	SerializeMapValue(SerializeMapValueFormat f, MapValue v) : Format(f), Value(v) {}
	SerializeMapValue(string v) : Format(SerializeMapValueFormat::String), Value(false), StringValue(v) {}
};

struct SerializeValue
{
	uint8_t* DataPtr;
	uint32_t Size;

	SerializeValue()
	{
		DataPtr = nullptr;
		Size = 0;
	}

	SerializeValue(uint8_t* ptr, uint32_t size)
	{
		DataPtr = ptr;
		Size = size;
	}
};

enum class SerializeFormat
{
	Binary,
	Text,
	Map
};

class Serializer
{
private:
	vector<uint8_t> _data;
	vector<string> _prefixes;
	string _prefix;

	unordered_set<string> _usedKeys;
	unordered_map<string, SerializeValue> _values;

	//Used by Lua API
	unordered_map<string, SerializeMapValue> _mapValues;

	uint32_t _version = 0;
	bool _saving = false;
	SerializeFormat _format = SerializeFormat::Binary;

private:
	bool LoadFromTextFormat(istream& file);
	string NormalizeName(const char* name, int index);
	void UpdatePrefix();

	string GetKey(const char* name, int index)
	{
		string valName = NormalizeName(name, index);
		if(valName.empty()) {
			throw std::runtime_error("invalid value name");
		}
		return _prefix + valName;
	}

	template<typename T>
	void WriteValue(T value)
	{
		uint8_t* ptr = (uint8_t*)&value;
		constexpr bool isBigEndian = false;
		constexpr int mask = isBigEndian ? sizeof(T) - 1 : 0;
		for(int i = 0; i < sizeof(T); i++) {
			_data.push_back(ptr[i ^ mask]);
		}
	}

	template<typename T>
	void ReadValue(T& value, uint8_t* src)
	{
		uint8_t* ptr = (uint8_t*)&value;
		constexpr bool isBigEndian = false;
		constexpr int mask = isBigEndian ? sizeof(T) - 1 : 0;
		for(int i = 0; i < sizeof(T); i++) {
			ptr[i ^ mask] = src[i];
		}
	}

	template<typename T>
	void WriteMapFormat(string& key, T& value)
	{
		if constexpr(std::is_same<T, bool>::value) {
			_mapValues.try_emplace(key, SerializeMapValueFormat::Bool, (bool)value);
		} else if constexpr(std::is_integral<T>::value) {
			_mapValues.try_emplace(key, SerializeMapValueFormat::Integer, (int64_t)value);
		} else if constexpr(std::is_floating_point<T>::value) {
			_mapValues.try_emplace(key, SerializeMapValueFormat::Double, (double)value);
		} else if constexpr(std::is_same<T, string>::value) {
			_mapValues.try_emplace(key, value);
		}	
	}

	template<typename T>
	void ReadMapFormat(string& key, T& value)
	{
		auto result = _mapValues.find(key);
		if(result != _mapValues.end()) {
			SerializeMapValue mapVal = result->second;
			if constexpr(std::is_same<T, bool>::value) {
				if(mapVal.Format == SerializeMapValueFormat::Bool) {
					value = mapVal.Value.Bool;
				}
			} else if constexpr(std::is_integral<T>::value) {
				if(mapVal.Format == SerializeMapValueFormat::Integer) {
					value = (T)mapVal.Value.Integer;
				}
			} else if constexpr(std::is_floating_point<T>::value) {
				if(mapVal.Format == SerializeMapValueFormat::Double) {
					value = (double)mapVal.Value.Double;
				}
			} else if constexpr(std::is_same<T, string>::value) {
				if(mapVal.Format == SerializeMapValueFormat::String) {
					value = mapVal.StringValue;
				}
			}
		}
	}

	template<typename T>
	void WriteTextFormat(string& key, T& value)
	{
		//Write key
		_data.insert(_data.end(), key.begin(), key.end());
		_data.push_back(' ');
		if constexpr(std::is_enum<T>::value) {
			auto enumStr = magic_enum::enum_name(value);
			_data.insert(_data.end(), enumStr.begin(), enumStr.end());
		} else if constexpr(std::is_same<T, bool>::value) {
			string boolStr = value ? "true" : "false";
			_data.insert(_data.end(), boolStr.begin(), boolStr.end());
		} else {
			auto valueStr = std::to_string(value);
			_data.insert(_data.end(), valueStr.begin(), valueStr.end());
		}
		_data.push_back('\n');
	}

	template<typename T>
	void ReadTextFormat(SerializeValue& savedValue, T& value)
	{
		string textValue(savedValue.DataPtr, savedValue.DataPtr + savedValue.Size);
		if constexpr(std::is_enum<T>::value) {
			auto enumValue = magic_enum::enum_cast<T>(textValue);
			if(enumValue.has_value()) {
				value = enumValue.value();
			}
		} else if constexpr(std::is_same<T, bool>::value) {
			value = textValue == "true";
		} else {
			if(textValue.find_first_not_of("0123456789") == string::npos) {
				try {
					value = (T)std::stoll(textValue);
				} catch(std::exception&) {
					value = (T)0;
				}
			}
		}
	}

public:
	Serializer(uint32_t version, bool forSave, SerializeFormat format = SerializeFormat::Binary);

	uint32_t GetVersion() { return _version; }
	bool IsSaving() { return _saving; }
	
	SerializeFormat GetFormat() { return _format; }
	unordered_map<string, SerializeMapValue>& GetMapValues() { return _mapValues; }

	template <class T> struct is_unique_ptr : std::false_type {};
	template <class T, class D> struct is_unique_ptr<std::unique_ptr<T, D>> : std::true_type {};
	template <class T> struct is_shared_ptr : std::false_type {};
	template <class T> struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

	template<typename T> void Stream(T& value, const char* name, int index = -1)
	{
		static_assert(!is_unique_ptr<std::remove_cv_t<T>>::value, "[Serializer] Unexpected unique_ptr");
		static_assert(!is_shared_ptr<std::remove_cv_t<T>>::value, "[Serializer] Unexpected shared_ptr");
		static_assert(!std::is_pointer<T>::value, "[Serializer] Unexpected pointer");
		static_assert(!std::is_class<T>::value || std::is_base_of<ISerializable, T>::value, "[Serializer] Object does not implement ISerializable");
		static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value || std::is_base_of<ISerializable, T>::value, "[Serializer] Invalid value type");
		
		if constexpr(std::is_base_of<ISerializable, T>::value) {
			Stream((ISerializable&)value, name, index);
		} else {
			string key = GetKey(name, index);
			if(!_usedKeys.emplace(key).second) {
				throw std::runtime_error("Duplicate key");
			}

			if(_saving) {
				switch(_format) {
					case SerializeFormat::Binary:
						//Write key
						_data.insert(_data.end(), key.begin(), key.end());
						_data.push_back(0);

						//Write value size
						WriteValue((uint32_t)sizeof(T));

						//Write value
						WriteValue(value);
						break;

					case SerializeFormat::Text: WriteTextFormat(key, value); break;
					case SerializeFormat::Map: WriteMapFormat(key, value); break;
				}
			} else {
				switch(_format) {
					case SerializeFormat::Binary: {
						auto result = _values.find(key);
						if(result != _values.end()) {
							SerializeValue& savedValue = result->second;
							if(savedValue.Size >= sizeof(T)) {
								ReadValue(value, savedValue.DataPtr);
							} else {
								value = (T)0;
							}
						} else {
							value = (T)0;
						}
						break;
					}

					case SerializeFormat::Text: {
						auto result = _values.find(key);
						if(result != _values.end()) {
							ReadTextFormat(result->second, value);
						} else {
							value = (T)0;
						}
						break;
					}

					case SerializeFormat::Map:
						ReadMapFormat(key, value);
						break;
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
		static_assert(std::is_base_of<ISerializable, T>::value, "[Serializer] Object does not implement ISerializable");
		PushNamePrefix(name, index);
		((ISerializable*)obj.get())->Serialize(*this);
		PopNamePrefix();
	}

	template<typename T> void Stream(const unique_ptr<T>& obj, const char* name, int index = -1)
	{
		static_assert(std::is_base_of<ISerializable, T>::value, "[Serializer] Object does not implement ISerializable");
		PushNamePrefix(name, index);
		((ISerializable*)obj.get())->Serialize(*this);
		PopNamePrefix();
	}

	template<typename T> void Stream(shared_ptr<T>& obj, const char* name, int index = -1)
	{
		static_assert(std::is_base_of<ISerializable, T>::value, "[Serializer] Object does not implement ISerializable");
		PushNamePrefix(name, index);
		((ISerializable*)obj.get())->Serialize(*this);
		PopNamePrefix();
	}

	template<typename T> void StreamArray(T* arrayValues, uint32_t elementCount, const char* name)
	{
		if(_format == SerializeFormat::Map) {
			return;
		}

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
			if constexpr(sizeof(T) == 1) {
				_data.insert(_data.end(), arrayValues, arrayValues + elementCount);
			} else {
				for(uint32_t i = 0; i < elementCount; i++) {
					WriteValue(arrayValues[i]);
				}
			}
		} else {
			auto result = _values.find(key);
			if(result != _values.end()) {
				SerializeValue& savedValue = result->second;
				if(savedValue.Size >= elementCount * sizeof(T)) {
					if constexpr(sizeof(T) == 1) {
						memcpy(arrayValues, savedValue.DataPtr, elementCount);
					} else {
						uint8_t* src = savedValue.DataPtr;
						for(uint32_t i = 0; i < elementCount; i++) {
							ReadValue(arrayValues[i], src);
							src += sizeof(T);
						}
					}
				} else {
					memset(arrayValues, 0, sizeof(T) * elementCount);
				}
			} else {
				memset(arrayValues, 0, sizeof(T) * elementCount);
			}
		}
	}

	template<typename T> void Stream(vector<T>& values, const char* name, int index = -1)
	{
		if(_format == SerializeFormat::Map) {
			return;
		}

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
			} else {
				values.clear();
			}
		}
	}

	template<> void Stream(string& value, const char* name, int index)
	{
		string key = GetKey(name, index);
		if(!_usedKeys.emplace(key).second) {
			throw std::runtime_error("Duplicate key");
		}

		if(_format == SerializeFormat::Map) {
			if(_saving) {
				WriteMapFormat(key, value);
			} else {
				ReadMapFormat(key, value);
			}
		} else {
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
				} else {
					value = "";
				}
			}
		}
	}

	void PushNamePrefix(const char* name, int index = -1);
	void PopNamePrefix();
	void SaveTo(ostream &file, int compressionLevel = 1);
	bool LoadFrom(istream& file);
	void LoadFromMap(unordered_map<string, SerializeMapValue>& map);
};
