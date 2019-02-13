#pragma once
#include "stdafx.h"

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
};
