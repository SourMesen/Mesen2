#include <string>
#include <chrono>
#include <thread>
#include <stdint.h>

#include <X11/Xlib.h>
#include <X11/cursorfont.h>

#include "LinuxMouseManager.h"
#include "Core/Shared/MessageManager.h"

LinuxMouseManager::LinuxMouseManager(void* windowHandle)
{
	_mainWindow = windowHandle;

	_display = XOpenDisplay(nullptr);
	_defaultScreen = XDefaultScreen(_display);
	_rootWindow = XRootWindow(_display, _defaultScreen);

	_defaultCursor = XCreateFontCursor(_display, XC_left_ptr);
	_crossCursor = XCreateFontCursor(_display, XC_crosshair);

	XColor color = {};
	uint8_t nullCursorData[1] = {0};
	Pixmap pixmap = XCreateBitmapFromData(_display, _rootWindow, nullCursorData, 1, 1);
	_hiddenCursor = XCreatePixmapCursor(_display, pixmap, pixmap, &color, &color, 0, 0);
}

LinuxMouseManager::~LinuxMouseManager() {}

SystemMouseState LinuxMouseManager::GetSystemMouseState(void* rendererHandle)
{
	SystemMouseState state = {};

	Window root = nullptr;
	Window c = nullptr;
	Window child = nullptr;
	int rootX, rootY, childX, childY, mask;

	XGrabServer(_display);
	XQueryPointer(_display, _rootWindow, &root, &c, &rootX, &rootY, &childX, &childY, &mask);
	if(root != _rootWindow) c = root;
	while(c != nullptr) {
		child = c;
		XQueryPointer(_display, c, &root, &c, &rootX, &rootY, &childX, &childY, &mask);
	}
	XUngrabServer(_display);
	XFlush(_display);

	state.XPosition = rootX;
	state.YPosition = rootY;
	state.LeftButton = (mask & (1 << 8)) != 0;
	state.RightButton = (mask & (1 << 9)) != 0;
	state.MiddleButton = (mask & (1 << 10)) != 0;
	//TODO back/forward are not supported by XQueryPointer?
	//state.Button4 = (mask & (1 << 11)) != 0;
	//state.Button5 = (mask & (1 << 12)) != 0;

	return state;
}

bool LinuxMouseManager::CaptureMouse(int32_t x, int32_t y, int32_t width, int32_t height, void* rendererHandle)
{
	if(rendererHandle == nullptr) {
		//Due to the mouse position constantly being set to the center of the window and the cursor being hidden
		//actually capturing the cursor when using the software renderer is not strictly needed
		return true;
	}

	for(int i = 0; i < 10; i++) {
		int result = XGrabPointer(_display, rendererHandle, true, NoEventMask, GrabModeAsync, GrabModeAsync, rendererHandle, _hiddenCursor, nullptr);
		XFlush(_display);
		if(result == 1 && i < 9) {
			//XGrabPointer can fail with AlreadyGrabbed - this can be normal, retry a few times
			std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(100));
		} else {
			if(result != 0) {
				std::string message = "XGrabPointer failed: ";
				MessageManager::Log(message + std::to_string(result));
			}
			return false;
		}
	}

	return true;
}

void LinuxMouseManager::ReleaseMouse()
{
	XUngrabPointer(_display, nullptr);
	XFlush(_display);
}

void LinuxMouseManager::SetSystemMousePosition(int32_t x, int32_t y)
{
	XWarpPointer(_display, nullptr, _rootWindow, 0, 0, 0, 0, x, y);
	XFlush(_display);
}

void LinuxMouseManager::SetCursorImage(CursorImage cursor)
{
	switch(cursor) {
		case CursorImage::Hidden: XDefineCursor(_display, _mainWindow, _hiddenCursor); break;
		case CursorImage::Arrow: XDefineCursor(_display, _mainWindow, _defaultCursor); break;
		case CursorImage::Cross: XDefineCursor(_display, _mainWindow, _crossCursor); break;
	}
	XFlush(_display);
}

double LinuxMouseManager::GetPixelScale()
{
	return 1.0;
}
