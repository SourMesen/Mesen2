#pragma once
#include "Shared/Interfaces/IMouseManager.h"

class WindowsMouseManager : public IMouseManager
{
private:
	void* _arrowCursor;
	void* _crossCursor;

public:
	WindowsMouseManager();
	virtual ~WindowsMouseManager();

	SystemMouseState GetSystemMouseState(void* rendererHandle);
	bool CaptureMouse(int32_t x, int32_t y, int32_t width, int32_t height, void* rendererHandle);
	void ReleaseMouse();
	void SetSystemMousePosition(int32_t x, int32_t y);
	void SetCursorImage(CursorImage cursor);
	double GetPixelScale();
};
