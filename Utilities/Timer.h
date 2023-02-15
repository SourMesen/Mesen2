#pragma once
#include "pch.h"
#include <chrono>
using namespace std::chrono;

class Timer
{
	private:
		high_resolution_clock::time_point _start;

public:
		Timer();
		void Reset();
		double GetElapsedMS() const;
		void WaitUntil(double targetMillisecond) const;
};