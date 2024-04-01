#include "Common.h"
#include "WindowsMouseManager.h"

WindowsMouseManager::WindowsMouseManager()
{
	_arrowCursor = LoadCursor(nullptr, IDC_ARROW);
	_crossCursor = LoadCursor(nullptr, IDC_CROSS);
}

WindowsMouseManager::~WindowsMouseManager() {}

SystemMouseState WindowsMouseManager::GetSystemMouseState(void* rendererHandle)
{
	SystemMouseState state = {};
	POINT point;
	GetCursorPos(&point);
	if(rendererHandle != nullptr && WindowFromPoint(point) != (HWND) rendererHandle) {
		//Mouse is over another window
		state.XPosition = -1;
		state.YPosition = -1;
	} else {
		state.XPosition = point.x;
		state.YPosition = point.y;
	}
	state.LeftButton = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	state.RightButton = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
	state.MiddleButton = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;
	state.Button4 = (GetAsyncKeyState(VK_XBUTTON1) & 0x8000) != 0;
	state.Button5 = (GetAsyncKeyState(VK_XBUTTON2) & 0x8000) != 0;
	return state;
}

bool WindowsMouseManager::CaptureMouse(int32_t x, int32_t y, int32_t width, int32_t height, void* rendererHandle)
{
	if(rendererHandle == nullptr) {
		//TODO Attempting to capture the cursor when using the sofware renderer behaves really erratically
		//cursor is not actually locked but clicks outside the window do nothing, and movement is really odd (not usable)
		return false;
	}
	RECT rect;
	rect.left = x;
	rect.top = y;
	rect.right = x + width;
	rect.bottom = y + height;
	ClipCursor(&rect);
	return true;
}

void WindowsMouseManager::ReleaseMouse()
{
	ClipCursor(nullptr);
}

void WindowsMouseManager::SetSystemMousePosition(int32_t x, int32_t y)
{
	SetCursorPos(x, y);
}

void WindowsMouseManager::SetCursorImage(CursorImage cursor)
{
	switch(cursor) {
		case CursorImage::Hidden: SetCursor(nullptr); break;
		case CursorImage::Arrow: SetCursor((HCURSOR) _arrowCursor); break;
		case CursorImage::Cross: SetCursor((HCURSOR) _crossCursor); break;
	}
}

double WindowsMouseManager::GetPixelScale()
{
	return 1.0;
}
