#pragma once
#include "pch.h"

enum class MessageType : uint8_t
{
	HandShake = 0,
	SaveState = 1,
	InputData = 2,
	MovieData = 3,
	GameInformation = 4,
	PlayerList = 5,
	SelectController = 6,
	ForceDisconnect = 7,
	ServerInformation = 8
};