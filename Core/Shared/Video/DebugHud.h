#pragma once
#include "pch.h"
#include "Utilities/SimpleLock.h"
#include "Shared/SettingTypes.h"
#include "Shared/Video/DrawCommand.h"

class DebugHud
{
private:
	static constexpr size_t MaxCommandCount = 500000;
	vector<unique_ptr<DrawCommand>> _commands;
	atomic<uint32_t> _commandCount;
	SimpleLock _commandLock;
	unordered_map<uint32_t, uint32_t> _drawPixels;

public:
	DebugHud();
	~DebugHud();

	bool HasCommands() { return _commandCount > 0; }

	bool Draw(uint32_t* argbBuffer, FrameInfo frameInfo, OverscanDimensions overscan, uint32_t frameNumber, HudScaleFactors scaleFactors, bool clearAndUpdate = false);
	void ClearScreen();

	void DrawPixel(int x, int y, int color, int frameCount, int startFrame = -1);
	void DrawLine(int x, int y, int x2, int y2, int color, int frameCount, int startFrame = -1);
	void DrawRectangle(int x, int y, int width, int height, int color, bool fill, int frameCount, int startFrame = -1);
	void DrawString(int x, int y, string text, int color, int backColor, int frameCount, int startFrame = -1, int maxWidth = 0, bool overwritePixels = false);

	__forceinline void AddCommand(unique_ptr<DrawCommand> cmd)
	{
		auto lock = _commandLock.AcquireSafe();
		if(_commands.size() < DebugHud::MaxCommandCount) {
			_commands.push_back(std::move(cmd));
			_commandCount++;
		}
	}
};
