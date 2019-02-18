#pragma once
#include "stdafx.h"
#include <cstring>

struct ControlDeviceState
{
	vector<uint8_t> State;

	bool operator!=(ControlDeviceState &other)
	{
		return State.size() != other.State.size() || memcmp(State.data(), other.State.data(), State.size()) != 0;
	}
};