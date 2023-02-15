#include "pch.h"
#include "Timer.h"

#include <thread>
#include <chrono>

using namespace std::chrono;

Timer::Timer() 
{
	Reset();
}

void Timer::Reset()
{
	_start = high_resolution_clock::now();
}

double Timer::GetElapsedMS() const
{
	high_resolution_clock::time_point end = high_resolution_clock::now();
	duration<double> span = duration_cast<duration<double>>(end - _start);
	return span.count() * 1000.0;
}

void Timer::WaitUntil(double targetMillisecond) const
{
	if(targetMillisecond > 0) {
		double elapsedTime = GetElapsedMS();
		if(targetMillisecond - elapsedTime > 1) {
			std::this_thread::sleep_for(std::chrono::duration<int, std::milli>((int)(targetMillisecond - elapsedTime)));
		}
	}
}
