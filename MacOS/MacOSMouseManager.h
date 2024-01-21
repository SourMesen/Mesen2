#pragma once

enum class CursorIcon
{
	Hidden,
	Arrow,
	Cross
};

struct MouseState
{
	double XPosition;
	double YPosition;
	bool LeftButton;
	bool RightButton;
	bool MiddleButton;
	bool Button4;
	bool Button5;
};

class MacOSMouseManager
{
private:
	static double _relativeX;
	static double _relativeY;
	static bool _mouseCaptured;
	static bool _cursorHidden;

public:
	static MouseState GetMouseState();
	static void SetMouseCaptured(bool enabled);
	static void SetMousePosition(double x, double y);
	static void SetRelativeMovement(double x, double y);
	static void SetCursor(CursorIcon cursor);
	static double GetPixelScale();
};
