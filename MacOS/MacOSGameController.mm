#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <GameController/GameController.h>

#include <iostream>
#include <stdint.h>
#include <optional>

#include "MacOSGameController.h"
//The MacOS SDK defines a global function 'Debugger', colliding with Mesen's Debugger class
//Redefine it temporarily so the headers don't cause compilation errors due to this
#define Debugger MesenDebugger
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#undef Debugger

MacOSGameController::MacOSGameController(Emulator* emu, GCController* controller)
{
	_emu = emu;
	_controller = [controller retain];
	_input = [[_controller extendedGamepad] retain];

	[_input setValueChangedHandler:^ void (GCExtendedGamepad* input, GCControllerElement* element) {
		if([input buttonA] == element) _buttonState[0] = [((GCControllerButtonInput*) element) isPressed];
		if([input buttonB] == element) _buttonState[1] = [((GCControllerButtonInput*) element) isPressed];
		if([input buttonX] == element) _buttonState[2] = [((GCControllerButtonInput*) element) isPressed];
		if([input buttonY] == element) _buttonState[3] = [((GCControllerButtonInput*) element) isPressed];
		if([input leftShoulder] == element) _buttonState[4] = [((GCControllerButtonInput*) element) isPressed];
		if([input rightShoulder] == element) _buttonState[5] = [((GCControllerButtonInput*) element) isPressed];
		if([input buttonMenu] == element) _buttonState[6] = [((GCControllerButtonInput*) element) isPressed];
		if([input buttonOptions] == element) _buttonState[7] = [((GCControllerButtonInput*) element) isPressed];
		if([input dpad] == element) HandleDpad((GCControllerDirectionPad*) element);
		if([input leftTrigger] == element) _buttonState[12] = [((GCControllerButtonInput*) element) isPressed];
		if([input rightTrigger] == element) _buttonState[13] = [((GCControllerButtonInput*) element) isPressed];
		if([input leftThumbstickButton] == element) _buttonState[14] = [((GCControllerButtonInput*) element) isPressed];
		if([input rightThumbstickButton] == element) _buttonState[15] = [((GCControllerButtonInput*) element) isPressed];
		if([input leftThumbstick] == element) HandleThumbstick((GCControllerDirectionPad*) element, 0);
		if([input rightThumbstick] == element) HandleThumbstick((GCControllerDirectionPad*) element, 1);
	}];
}

void MacOSGameController::HandleDpad(GCControllerDirectionPad* dpad)
{
	_buttonState[8] = [[dpad up] isPressed];
	_buttonState[9] = [[dpad down] isPressed];
	_buttonState[10] = [[dpad left] isPressed];
	_buttonState[11] = [[dpad right] isPressed];
}

void MacOSGameController::HandleThumbstick(GCControllerDirectionPad* stick, int stickNumber)
{
	double deadZoneRatio = _emu->GetSettings()->GetControllerDeadzoneRatio() * 0.4;

	float xAxis = [[stick xAxis] value];
	float yAxis = [[stick yAxis] value];
	_buttonState[(stickNumber * 4) + 16] = xAxis > deadZoneRatio;
	_buttonState[(stickNumber * 4) + 17] = xAxis < -deadZoneRatio;
	_buttonState[(stickNumber * 4) + 18] = yAxis > deadZoneRatio;
	_buttonState[(stickNumber * 4) + 19] = yAxis < -deadZoneRatio;
	_axisState[(stickNumber * 2) + 0] = INT16_MAX * xAxis;
	_axisState[(stickNumber * 2) + 1] = INT16_MAX * yAxis;
}

MacOSGameController::~MacOSGameController()
{
	[_input setValueChangedHandler:nil];
	[_input release];
	[_controller release];
}

bool MacOSGameController::IsGameController(GCController* controller)
{
	return _controller == controller;
}

bool MacOSGameController::IsButtonPressed(int buttonNumber)
{
	if(buttonNumber < 0 || buttonNumber >= 24) {
		return false;
	}
	return _buttonState[buttonNumber];
}

std::optional<int16_t> MacOSGameController::GetAxisPosition(int axis)
{
	axis -= 24;
	if(axis < 0 || axis >= 4) {
		return std::nullopt;
	}
	return _axisState[axis];
}

void MacOSGameController::SetForceFeedback(uint16_t magnitude)
{
	return;
}
