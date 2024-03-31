#include "LinuxMouseManager.h"

LinuxMouseManager::LinuxMouseManager()
{

}

LinuxMouseManager::~LinuxMouseManager() {}

SystemMouseState LinuxMouseManager::GetSystemMouseState(void* windowHandle)
{
	SystemMouseState state = {};
	return state;
}

bool LinuxMouseManager::CaptureMouse(int32_t x, int32_t y, int32_t width, int32_t height, void* windowHandle)
{
	return false;
}

void LinuxMouseManager::ReleaseMouse()
{

}

void LinuxMouseManager::SetSystemMousePosition(int32_t x, int32_t y)
{

}

void LinuxMouseManager::SetCursorImage(CursorImage cursor)
{

}

double LinuxMouseManager::GetPixelScale()
{
	return 1.0;
}
