#pragma once
#include "Shared/Interfaces/IMouseManager.h"

class LinuxMouseManager : public IMouseManager
{
private:
	void* _mainWindow;

	void* _display;
	int _defaultScreen;
	void* _rootWindow;

	void* _defaultCursor;
	void* _crossCursor;
	void* _hiddenCursor;

public:
	LinuxMouseManager(void* windowHandle);
	virtual ~LinuxMouseManager();

	SystemMouseState GetSystemMouseState(void* rendererHandle);
	bool CaptureMouse(int32_t x, int32_t y, int32_t width, int32_t height, void* rendererHandle);
	void ReleaseMouse();
	void SetSystemMousePosition(int32_t x, int32_t y);
	void SetCursorImage(CursorImage cursor);
	double GetPixelScale();
};
