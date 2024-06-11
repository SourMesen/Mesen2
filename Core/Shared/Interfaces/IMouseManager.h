#pragma once
#include "pch.h"

enum class CursorImage
{
	Hidden,
	Arrow,
	Cross
};

struct SystemMouseState
{
	int32_t XPosition;
	int32_t YPosition;
	bool LeftButton;
	bool RightButton;
	bool MiddleButton;
	bool Button4;
	bool Button5;
};

class IMouseManager
{
public:
	virtual ~IMouseManager() {}

	virtual SystemMouseState GetSystemMouseState(void* rendererHandle) = 0;
	virtual bool CaptureMouse(int32_t x, int32_t y, int32_t width, int32_t height, void* rendererHandle) = 0;
	virtual void ReleaseMouse() = 0;
	virtual void SetSystemMousePosition(int32_t x, int32_t y) = 0;
	virtual void SetCursorImage(CursorImage cursor) = 0;
	virtual double GetPixelScale() = 0;
};
