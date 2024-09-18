#pragma once

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <GameController/GameController.h>
#import <CoreHaptics/CoreHaptics.h>

#include <optional>

class Emulator;

class MacOSGameController
{
private:
	Emulator* _emu;

	GCController* _controller;
	GCExtendedGamepad* _input;
	CHHapticEngine* _haptics;
	id<CHHapticPatternPlayer> _player;

	bool _buttonState[24] = {};
	int16_t _axisState[4] = {};

	void HandleDpad(GCControllerDirectionPad* dpad);
	void HandleThumbstick(GCControllerDirectionPad* stick, int stickNumber);

public:
	MacOSGameController(Emulator* emu, GCController* controller);
	~MacOSGameController();

	bool IsGameController(GCController* controller);

	bool IsButtonPressed(int buttonNumber);
	std::optional<int16_t> GetAxisPosition(int axis);

	void SetForceFeedback(uint16_t magnitude);
};
