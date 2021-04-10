#pragma once
#include "Core/Shared/Interfaces/IMessageManager.h"
#include "Core/Shared/SettingTypes.h"
#include "Utilities/Timer.h"

class Emulator;

class BaseRenderer : public IMessageManager
{
private:
	list<shared_ptr<ToastInfo>> _toasts;

	Timer _fpsTimer;
	uint32_t _lastFrameCount = 0;
	uint32_t _lastRenderedFrameCount = 0;
	uint32_t _currentFPS = 0;
	uint32_t _currentRenderedFPS = 0;

	void RemoveOldToasts();
	std::wstring WrapText(string utf8Text, float maxLineWidth, uint32_t &lineCount);
	virtual float MeasureString(std::wstring text) = 0;
	virtual bool ContainsCharacter(wchar_t character) = 0;

protected:
	shared_ptr<Emulator> _emu;

	uint32_t _screenWidth = 0;
	uint32_t _screenHeight = 0;
	uint32_t _renderedFrameCount = 0;	

	BaseRenderer(shared_ptr<Emulator> emu, bool registerAsMessageManager);
	virtual ~BaseRenderer();

	bool IsMessageShown();	

	void DisplayMessage(string title, string message);
	void DrawToasts();
	
	void DrawToast(shared_ptr<ToastInfo> toast, int &lastHeight);
	void DrawString(std::string message, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t opacity = 255);	
	virtual void DrawString(std::wstring message, int x, int y, uint8_t r = 255, uint8_t g = 255, uint8_t b = 255, uint8_t opacity = 255) = 0;

	void ShowFpsCounter(int lineNumber);
	void ShowLagCounter(int lineNumber);
	void ShowFrameCounter(int lineNumber);
	void ShowGameTimer(int lineNumber);
	void DrawCounters();
};
