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
	PpuFrameDone = 7,
	ResolutionChanged = 8,
	ConfigChanged = 9,
	ExecuteShortcut = 10,
	EmulationStopped = 11,
	BeforeEmulationStop = 12,
	ViewerRefresh = 13,
	EventViewerRefresh = 14,
	MissingBios = 15
};

class INotificationListener
{
public:
	virtual void ProcessNotification(ConsoleNotificationType type, void* parameter) = 0;
};