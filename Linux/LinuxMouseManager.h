#pragma once
#include <X11/Xlib.h>
#include <X11/cursorfont.h>

//X11 defines global macros 'Button4' and 'Button5', colliding with the fields of SystemMouseState
//Undefine them here, as they are not needed
#undef Button4
#undef Button5

#include "Shared/Interfaces/IMouseManager.h"

class LinuxMouseManager : public IMouseManager
{
private:
	Window _mainWindow;

	Display* _display;
	int _defaultScreen;
	Window _rootWindow;

	Cursor _defaultCursor;
	Cursor _crossCursor;
	Cursor _hiddenCursor;

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
