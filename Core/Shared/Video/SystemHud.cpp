#include "stdafx.h"
#include "Shared/Video/SystemHud.h"
#include "Shared/Video/DebugHud.h"
#include "Shared/MessageManager.h"
#include "Shared/Video/DrawStringCommand.h"
#include "Shared/Interfaces/IMessageManager.h"

SystemHud::SystemHud(Emulator* emu)
{
	_emu = emu;
	_hud = emu->GetDebugHud();
	MessageManager::RegisterMessageManager(this);
}

SystemHud::~SystemHud()
{
	MessageManager::UnregisterMessageManager(this);
}

void SystemHud::Draw(FrameInfo info, OverscanDimensions overscan)
{
	_screenWidth = info.Width - _overscan.Left - _overscan.Right;
	_screenHeight = info.Height - _overscan.Top - _overscan.Bottom;
	_overscan = overscan;
	DrawCounters();
	DrawMessages();
}

void SystemHud::DrawMessage(MessageInfo &msg, int& lastHeight)
{
	//Get opacity for fade in/out effect
	uint8_t opacity = (uint8_t)(msg.GetOpacity() * 255);
	int textLeftMargin = 4;

	string text = "[" + msg.GetTitle() + "] " + msg.GetMessage();

	int maxWidth = _screenWidth - textLeftMargin;
	TextSize size = DrawStringCommand::MeasureString(text, maxWidth);
	lastHeight += size.Y;
	DrawString(text, textLeftMargin, _screenHeight - lastHeight, opacity);
}

void SystemHud::DrawString(string text, int x, int y, uint8_t opacity)
{
	int maxWidth = _screenWidth - x;
	x += _overscan.Left;
	y += _overscan.Top;
	opacity = 255 - opacity;
	for(int i = -1; i <= 1; i++) {
		for(int j = -1; j <= 1; j++) {
			_hud->DrawString(x + i, y + j, text, 0 | (opacity << 24), 0xFF000000, 1, -1, maxWidth);
		}
	}
	_hud->DrawString(x, y, text, 0xFFFFFF | (opacity << 24), 0xFF000000, 1, -1, maxWidth);
}

void SystemHud::ShowFpsCounter(int lineNumber)
{
	int yPos = 13 + 10 * lineNumber;
	if(_fpsTimer.GetElapsedMS() > 1000) {
		//Update fps every sec
		uint32_t frameCount = _emu->GetFrameCount();
		if(_lastFrameCount > frameCount) {
			_currentFPS = 0;
		} else {
			_currentFPS = (int)(std::round((double)(frameCount - _lastFrameCount) / (_fpsTimer.GetElapsedMS() / 1000)));
			_currentRenderedFPS = (int)(std::round((double)(_renderedFrameCount - _lastRenderedFrameCount) / (_fpsTimer.GetElapsedMS() / 1000)));
		}
		_lastFrameCount = frameCount;
		_lastRenderedFrameCount = _renderedFrameCount;
		_fpsTimer.Reset();
	}

	if(_currentFPS > 5000) {
		_currentFPS = 0;
	}
	if(_currentRenderedFPS > 5000) {
		_currentRenderedFPS = 0;
	}

	string fpsString = string("FPS: ") + std::to_string(_currentFPS); // +" / " + std::to_string(_currentRenderedFPS);
	uint32_t length = DrawStringCommand::MeasureString(fpsString).X;
	DrawString(fpsString, _screenWidth - 8 - length, yPos);
}

void SystemHud::ShowGameTimer(int lineNumber)
{
	int yPos = 13 + 10 * lineNumber;
	uint32_t frameCount = _emu->GetFrameCount();
	double frameRate = _emu->GetFps();
	uint32_t seconds = (uint32_t)(frameCount / frameRate) % 60;
	uint32_t minutes = (uint32_t)(frameCount / frameRate / 60) % 60;
	uint32_t hours = (uint32_t)(frameCount / frameRate / 3600);

	std::stringstream ss;
	ss << std::setw(2) << std::setfill('0') << hours << ":";
	ss << std::setw(2) << std::setfill('0') << minutes << ":";
	ss << std::setw(2) << std::setfill('0') << seconds;

	string text = ss.str();
	uint32_t length = DrawStringCommand::MeasureString(text).X;
	DrawString(ss.str(), _screenWidth - 8 - length, yPos);
}

void SystemHud::ShowFrameCounter(int lineNumber)
{
	int yPos = 13 + 10 * lineNumber;
	uint32_t frameCount = _emu->GetFrameCount();

	string frameCounter = MessageManager::Localize("Frame") + ": " + std::to_string(frameCount);
	uint32_t length = DrawStringCommand::MeasureString(frameCounter).X;
	DrawString(frameCounter, _screenWidth - 8 - length, yPos);
}

void SystemHud::DrawCounters()
{
	int lineNumber = 0;
	PreferencesConfig cfg = _emu->GetSettings()->GetPreferences();

	if(cfg.ShowFps) {
		ShowFpsCounter(lineNumber++);
	}
	if(cfg.ShowGameTimer) {
		ShowGameTimer(lineNumber++);
	}
	if(cfg.ShowFrameCounter) {
		ShowFrameCounter(lineNumber++);
	}

	_renderedFrameCount++;
}

void SystemHud::DisplayMessage(string title, string message)
{
	auto lock = _msgLock.AcquireSafe();
	_messages.push_front(std::make_unique<MessageInfo>(title, message, 3000));
}

void SystemHud::DrawMessages()
{
	auto lock = _msgLock.AcquireSafe();

	_messages.remove_if([](unique_ptr<MessageInfo>& msg) { return msg->IsExpired(); });

	int counter = 0;
	int lastHeight = 3;
	for(unique_ptr<MessageInfo>& msg : _messages) {
		if(counter < 4) {
			DrawMessage(*msg.get(), lastHeight);
		} else {
			break;
		}
		counter++;
	}
}
