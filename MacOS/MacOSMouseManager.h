#pragma once
#include "Shared/Interfaces/IMouseManager.h"

class MacOSMouseManager : public IMouseManager
{
private:
	double _relativeX;
	double _relativeY;
	bool _mouseCaptured;
	bool _cursorHidden;

public:
	MacOSMouseManager();
	virtual ~MacOSMouseManager();

	SystemMouseState GetSystemMouseState(void* windowHandle);
	bool CaptureMouse(int32_t x, int32_t y, int32_t width, int32_t height, void* windowHandle);
  void ReleaseMouse();
  void SetSystemMousePosition(int32_t x, int32_t y);
  void SetCursorImage(CursorImage cursor);
  double GetPixelScale();

	void SetRelativeMovement(double x, double y);
};
