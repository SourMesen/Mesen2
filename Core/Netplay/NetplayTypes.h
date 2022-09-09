#pragma once
#include "pch.h"
#include "Shared/SettingTypes.h"

struct NetplayControllerInfo
{
	uint8_t Port = 0;
	uint8_t SubPort = 0;
};

struct NetplayControllerUsageInfo
{
	NetplayControllerInfo Port = {};
	ControllerType Type = ControllerType::None;
	bool InUse = false;
};

struct PlayerInfo
{
	NetplayControllerInfo ControllerPort = {};
	bool IsHost = false;
};
