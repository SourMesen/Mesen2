#pragma once
#include "stdafx.h"
#include <chrono>
using namespace std::chrono;

class Timer
{
	private:
		high_resolution_clock::time_point _start;

public:
		Timer();
		void Reset();
		double GetElapsedMS();
		void WaitUntil(double targetMillisecond);
};