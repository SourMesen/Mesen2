#include "Common.h"
#include "WindowsMouseManager.h"

WindowsMouseManager::WindowsMouseManager()
{

}

WindowsMouseManager::~WindowsMouseManager() {}

SystemMouseState WindowsMouseManager::GetSystemMouseState(void* windowHandle)
{
	SystemMouseState state = {};
	return state;
}

bool WindowsMouseManager::CaptureMouse(int32_t x, int32_t y, int32_t width, int32_t height, void* windowHandle)
{
	return false;
}

void WindowsMouseManager::ReleaseMouse()
{

}

void WindowsMouseManager::SetSystemMousePosition(int32_t x, int32_t y)
{

}

void WindowsMouseManager::SetCursorImage(CursorImage cursor)
{

}

double WindowsMouseManager::GetPixelScale()
{
	return 1.0;
}
