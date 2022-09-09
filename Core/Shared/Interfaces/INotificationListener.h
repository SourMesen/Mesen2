#pragma once
#include "pch.h"

enum class EmulatorShortcut;

enum class ConsoleNotificationType
{
	GameLoaded,
	StateLoaded,
	GameReset,
	GamePaused,
	GameResumed,
	CodeBreak,
	DebuggerResumed,
	PpuFrameDone,
	ResolutionChanged,
	ConfigChanged,
	ExecuteShortcut,
	ReleaseShortcut,
	EmulationStopped,
	BeforeEmulationStop,
	ViewerRefresh,
	EventViewerRefresh,
	MissingFirmware,
	BeforeGameUnload,
	BeforeGameLoad,
	GameLoadFailed,
	CheatsChanged,
	RequestConfigChange
};

class INotificationListener
{
public:
	virtual void ProcessNotification(ConsoleNotificationType type, void* parameter) = 0;
};

struct ExecuteShortcutParams
{
	EmulatorShortcut Shortcut;
	uint32_t Param;
};