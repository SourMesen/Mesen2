#pragma once
#include "pch.h"
#include <random>

class RandomHelper
{
public:
	template<typename T>
	static T GetValue(T min, T max)
	{
		std::random_device rd;
		std::mt19937 mt(rd());
		std::uniform_int_distribution<> dist(min, max);
		return dist(mt);
	}

	static bool GetBool()
	{
		return GetValue(0, 1) == 1;
	}
};
