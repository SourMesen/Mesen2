#pragma once
#include "stdafx.h"

enum class ConsoleNotificationType
{
	GameLoaded = 0,
	StateLoaded = 1,
	GameReset = 2,
	GamePaused = 3,
	GameResumed = 4,
	GameStopped = 5,
	CodeBreak = 6,
	PpuFrameDone = 9,
	ResolutionChanged = 11,
	ConfigChanged = 13,
	ExecuteShortcut = 16,
	EmulationStopped = 17,
	BeforeEmulationStop = 19,
};

class INotificationListener
{
public:
	virtual void ProcessNotification(ConsoleNotificationType type, void* parameter) = 0;
};