#pragma once
#include "Utilities/Timer.h"

class FrameLimiter
{
private:
	Timer _clockTimer;
	double _targetTime;
	double _delay;
	bool _resetRunTimers;

public:
	FrameLimiter(double delay)
	{
		_delay = delay;
		_targetTime = _delay;
		_resetRunTimers = false;
	}

	void SetDelay(double delay)
	{
		_delay = delay;
		_resetRunTimers = true;
	}

	void ProcessFrame()
	{
		if(_resetRunTimers || (_clockTimer.GetElapsedMS() - _targetTime) > 100) {
			//Reset the timers, this can happen in 3 scenarios:
			//1) Target frame rate changed
			//2) The console was reset/power cycled or the emulation was paused (with or without the debugger)
			//3) As a satefy net, if we overshoot our target by over 100 milliseconds, the timer is reset, too.
			//   This can happen when something slows the emulator down severely (or when breaking execution in VS when debugging Mesen itself, etc.)
			_clockTimer.Reset();
			_targetTime = 0;
			_resetRunTimers = false;
		}

		_targetTime += _delay;
	}

	bool WaitForNextFrame()
	{
		if(_targetTime - _clockTimer.GetElapsedMS() > 50) {
			//When sleeping for a long time (e.g <= 25% speed), sleep in small chunks and check to see if we need to stop sleeping between each sleep call
			_clockTimer.WaitUntil(_clockTimer.GetElapsedMS() + 40);
			return true;
		}

		_clockTimer.WaitUntil(_targetTime);
		return false;
	}
};