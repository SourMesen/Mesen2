#pragma once
#include "stdafx.h"
#include "Utilities/SimpleLock.h"
#include "Shared/SettingTypes.h"

class DrawCommand;

class DebugHud
{
private:
	static constexpr size_t MaxCommandCount = 500000;
	vector<unique_ptr<DrawCommand>> _commands;
	SimpleLock _commandLock;

public:
	DebugHud();
	~DebugHud();

	void Draw(uint32_t* argbBuffer, FrameInfo frameInfo, OverscanDimensions overscan, uint32_t frameNumber, bool autoScale);
	void ClearScreen();

	void DrawPixel(int x, int y, int color, int frameCount, int startFrame = -1);
	void DrawLine(int x, int y, int x2, int y2, int color, int frameCount, int startFrame = -1);
	void DrawRectangle(int x, int y, int width, int height, int color, bool fill, int frameCount, int startFrame = -1);
	void DrawString(int x, int y, string text, int color, int backColor, int frameCount, int startFrame = -1, int maxWidth = 0);

	void AddCommand(unique_ptr<DrawCommand> cmd);
};
