#include "stdafx.h"
#include "Timer.h"

#ifndef LIBRETRO

#include <thread>

#ifdef _WIN32
#include <Windows.h>

Timer::Timer() 
{
	LARGE_INTEGER li;
	if(!QueryPerformanceFrequency(&li)) {
		throw;
	}

	_frequency = double(li.QuadPart) / 1000.0;

	QueryPerformanceCounter(&li);
	_start = li.QuadPart;
}

void Timer::Reset()
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	_start = li.QuadPart;
}

double Timer::GetElapsedMS()
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return double(li.QuadPart - _start) / _frequency;
}

#else 
#include <time.h>

Timer::Timer() 
{
	Reset();
}

void Timer::Reset()
{
	timespec start;
	clock_gettime(CLOCK_MONOTONIC, &start);

	_start = start.tv_sec * 1000000000 + start.tv_nsec;
}

double Timer::GetElapsedMS()
{
	timespec end;
	clock_gettime(CLOCK_MONOTONIC, &end);

	uint64_t currentTime = end.tv_sec * 1000000000 + end.tv_nsec;
	
	return (double)(currentTime - _start) / 1000000.0;
}

#endif

void Timer::WaitUntil(double targetMillisecond) 
{
	if(targetMillisecond > 0) {
		double elapsedTime = GetElapsedMS();
		if(targetMillisecond - elapsedTime > 1) {
			std::this_thread::sleep_for(std::chrono::duration<int, std::milli>((int)(targetMillisecond - elapsedTime)));
		}
	}
}

#else 

//This is not used by Libretro port, remove its dependencies

Timer::Timer()
{
}

void Timer::Reset()
{
}

double Timer::GetElapsedMS()
{
	return 0.0;
}

void Timer::WaitUntil(double targetMillisecond)
{
}

#endif