#pragma once
#include "pch.h"

struct SaveStateCompatInfo
{
	bool IsCompatible;
	string PrefixToAdd;
	string PrefixToRemove;
	vector<string> FieldsToRemove;
};