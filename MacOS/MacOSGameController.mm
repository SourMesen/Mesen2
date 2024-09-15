#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <GameController/GameController.h>
#import <CoreHaptics/CoreHaptics.h>

#include <iostream>
#include <stdint.h>
#include <optional>

#include "MacOSGameController.h"
//The MacOS SDK defines a global function 'Debugger', colliding with Mesen's Debugger class
//Redefine it temporarily so the headers don't cause compilation errors due to this
#define Debugger MesenDebugger
#include "Shared/MessageManager.h"
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

	_haptics = nil;
	_player = nil;
	if([_controller haptics] != nil) {
		_haptics = [[_controller haptics] createEngineWithLocality:GCHapticsLocalityDefault];
		NSError* error = nil;
		[_haptics startAndReturnError:&error];
		if(error) {
			MessageManager::Log("[Input Device] Failed to initialize force feedback");
			_haptics = nil;
		} else {
			[_haptics retain];
		}
	}
}

MacOSGameController::~MacOSGameController()
{
	if(_haptics) {
		if(_player) {
			NSError* error = nil;
			[_player stopAtTime:0.0 error:&error];
			[_player release];
		}
		[_haptics stopWithCompletionHandler:^ void (NSError* error) {}];
		[_haptics release];
	}
	[_input setValueChangedHandler:nil];
	[_input release];
	[_controller release];
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
	NSError* error = nil;

	if(_haptics == nil) {
		return;
	}

	if(_player != nil) {
		[_player stopAtTime:0.0 error:&error];
		if(error) {
			MessageManager::Log("[Input Device] Failed to stop force feedback effect");
			return;
		}
		[_player release];
		_player = nil;
	}

	//If magnitude is zero, only stop current effect
	if(magnitude == 0) {
		return;
	}

	double strength = magnitude / (double) UINT16_MAX;
	CHHapticEventParameter* intensityPar = [[CHHapticEventParameter alloc] initWithParameterID:CHHapticEventParameterIDHapticIntensity value:strength];
	CHHapticEventParameter* sharpnessPar = [[CHHapticEventParameter alloc] initWithParameterID:CHHapticEventParameterIDHapticSharpness value:0.6];
	CHHapticEvent* event = [[CHHapticEvent alloc] initWithEventType:CHHapticEventTypeHapticContinuous parameters:@[intensityPar, sharpnessPar] relativeTime:0.0 duration:GCHapticDurationInfinite];

	CHHapticPattern* pattern = [[CHHapticPattern alloc] initWithEvents:@[event] parameters:@[] error:&error];
	[intensityPar release];
	[sharpnessPar release];
	[event release];
	if(error) {
		MessageManager::Log("[Input Device] Failed to create force feedback pattern");
		[pattern release];
		return;
	}

	_player = [_haptics createPlayerWithPattern:pattern error:&error];
	[pattern release];
	if(error) {
		MessageManager::Log("[Input Device] Failed to create force feedback effect");
		_player = nil;
		return;
	}
	[_player retain];

	[_player startAtTime:0.0 error:&error];
	if(error) {
		MessageManager::Log("[Input Device] Failed to start force feedback effect");
	}
}
