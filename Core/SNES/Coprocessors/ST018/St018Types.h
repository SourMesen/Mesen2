#pragma once
#include "pch.h"
#include "Shared/BaseState.h"
#include "SNES/Coprocessors/ST018/ArmV3Types.h"

struct St018State : BaseState
{
	bool HasDataForSnes;
	uint8_t DataSnes;

	bool HasDataForArm;
	uint8_t DataArm;

	bool ArmReset;
	bool Ack;
};