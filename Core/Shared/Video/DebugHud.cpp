#include "pch.h"
#include <algorithm>
#include "Shared/Video/DebugHud.h"
#include "Shared/Video/DrawCommand.h"
#include "Shared/Video/DrawLineCommand.h"
#include "Shared/Video/DrawPixelCommand.h"
#include "Shared/Video/DrawRectangleCommand.h"
#include "Shared/Video/DrawStringCommand.h"
#include "Shared/Video/DrawScreenBufferCommand.h"

DebugHud::DebugHud()
{
	_commandCount = 0;
}

DebugHud::~DebugHud()
{
	_commandLock.Acquire();
	_commandLock.Release();
}

void DebugHud::ClearScreen()
{
	auto lock = _commandLock.AcquireSafe();
	_commands.clear();
	_drawPixels.clear();
}

bool DebugHud::Draw(uint32_t* argbBuffer, FrameInfo frameInfo, OverscanDimensions overscan, uint32_t frameNumber, HudScaleFactors scaleFactors, bool clearAndUpdate)
{
	auto lock = _commandLock.AcquireSafe();

	bool isDirty = false;
	if(clearAndUpdate) {
		unordered_map<uint32_t, uint32_t> drawPixels;
		drawPixels.reserve(1000);
		for(unique_ptr<DrawCommand>& command : _commands) {
			command->Draw(&drawPixels, argbBuffer, frameInfo, overscan, frameNumber, scaleFactors);
		}

		isDirty = drawPixels.size() != _drawPixels.size();
		if(!isDirty) {
			for(auto keyValue : drawPixels) {
				auto match = _drawPixels.find(keyValue.first);
				if(match != _drawPixels.end()) {
					if(keyValue.second != match->second) {
						isDirty = true;
						break;
					}
				} else {
					isDirty = true;
					break;
				}
			}
		}

		if(isDirty) {
			memset(argbBuffer, 0, frameInfo.Height * frameInfo.Width * sizeof(uint32_t));
			for(auto keyValue : drawPixels) {
				argbBuffer[keyValue.first] = keyValue.second;
			}
			_drawPixels = drawPixels;
		}
	} else {
		isDirty = true;
		for(unique_ptr<DrawCommand>& command : _commands) {
			command->Draw(nullptr, argbBuffer, frameInfo, overscan, frameNumber, scaleFactors);
		}
	}

	_commands.erase(std::remove_if(_commands.begin(), _commands.end(), [](const unique_ptr<DrawCommand>& c) { return c->Expired(); }), _commands.end());
	_commandCount = (uint32_t)_commands.size();

	return isDirty;
}

void DebugHud::DrawPixel(int x, int y, int color, int frameCount, int startFrame)
{
	AddCommand(unique_ptr<DrawCommand>(new DrawPixelCommand(x, y, color, frameCount, startFrame)));
}

void DebugHud::DrawLine(int x, int y, int x2, int y2, int color, int frameCount, int startFrame)
{
	AddCommand(unique_ptr<DrawCommand>(new DrawLineCommand(x, y, x2, y2, color, frameCount, startFrame)));
}

void DebugHud::DrawRectangle(int x, int y, int width, int height, int color, bool fill, int frameCount, int startFrame)
{
	AddCommand(unique_ptr<DrawCommand>(new DrawRectangleCommand(x, y, width, height, color, fill, frameCount, startFrame)));
}

void DebugHud::DrawString(int x, int y, string text, int color, int backColor, int frameCount, int startFrame, int maxWidth, bool overwritePixels)
{
	AddCommand(unique_ptr<DrawCommand>(new DrawStringCommand(x, y, text, color, backColor, frameCount, startFrame, maxWidth, overwritePixels)));
}
