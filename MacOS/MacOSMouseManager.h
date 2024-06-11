#pragma once
#include "Shared/Interfaces/IMouseManager.h"

class MacOSMouseManager : public IMouseManager
{
private:
	double _relativeX;
	double _relativeY;
	bool _mouseCaptured;
	bool _cursorHidden;

	void* _eventMonitor;

	void SetRelativeMovement(double x, double y);

public:
	MacOSMouseManager();
	virtual ~MacOSMouseManager();

	SystemMouseState GetSystemMouseState(void* rendererHandle);
	bool CaptureMouse(int32_t x, int32_t y, int32_t width, int32_t height, void* rendererHandle);
	void ReleaseMouse();
	void SetSystemMousePosition(int32_t x, int32_t y);
	void SetCursorImage(CursorImage cursor);
	double GetPixelScale();
};
