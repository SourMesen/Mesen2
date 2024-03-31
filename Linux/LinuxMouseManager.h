#pragma once
#include "Shared/Interfaces/IMouseManager.h"

class LinuxMouseManager : public IMouseManager
{
private:

public:
	LinuxMouseManager();
	virtual ~LinuxMouseManager();

	SystemMouseState GetSystemMouseState(void* windowHandle);
	bool CaptureMouse(int32_t x, int32_t y, int32_t width, int32_t height, void* windowHandle);
	void ReleaseMouse();
	void SetSystemMousePosition(int32_t x, int32_t y);
	void SetCursorImage(CursorImage cursor);
	double GetPixelScale();
};
