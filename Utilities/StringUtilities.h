#pragma once
#include "pch.h"

class StringUtilities
{
public:
	static vector<string> Split(string input, char delimiter)
	{
		vector<string> result;
		size_t index = 0;
		size_t lastIndex = 0;
		while((index = input.find(delimiter, index)) != string::npos) {
			result.push_back(input.substr(lastIndex, index - lastIndex));
			index++;
			lastIndex = index;
		}
		result.push_back(input.substr(lastIndex));
		return result;
	}

	static string TrimLeft(string str)
	{
		size_t startIndex = str.find_first_not_of("\t ");
		if(startIndex == string::npos) {
			return "";
		} else if(startIndex > 0) {
			return str.substr(startIndex);
		}
		return str;
	}

	static string TrimRight(string str)
	{
		size_t endIndex = str.find_last_not_of("\t\r\n ");
		if(endIndex == string::npos) {
			return "";
		} else if(endIndex > 0) {
			return str.substr(0, endIndex + 1);
		}
		return str;
	}

	static string Trim(string str)
	{
		return TrimLeft(TrimRight(str));
	}

	static string ToUpper(string str)
	{
		std::transform(str.begin(), str.end(), str.begin(), ::toupper);
		return str;
	}

	static string ToLower(string str)
	{
		std::transform(str.begin(), str.end(), str.begin(), ::tolower);
		return str;
	}

	static void CopyToBuffer(string str, char* outBuffer, uint32_t maxSize)
	{
		memcpy(outBuffer, str.c_str(), std::min<uint32_t>((uint32_t)str.size(), maxSize));
	}

	static bool StartsWith(string& str, const char* content)
	{
		size_t length = strlen(content);
		for(size_t i = 0; i < length; i++) {
			if(str[i] != content[i]) {
				return false;
			}
		}
		return true;
	}
};
