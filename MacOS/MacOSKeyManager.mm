#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <GameController/GameController.h>

#include <algorithm>
#include "MacOSKeyManager.h"
#include "MacOSGameController.h"
//The MacOS SDK defines a global function 'Debugger', colliding with Mesen's Debugger class
//Redefine it temporarily so the headers don't cause compilation errors due to this
#define Debugger MesenDebugger
#include "Shared/MessageManager.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/KeyDefinitions.h"
#include "Shared/SettingTypes.h"
#undef Debugger

MacOSKeyManager::MacOSKeyManager(Emulator* emu)
{
	_emu = emu;

	ResetKeyState();

	_keyDefinitions = KeyDefinition::GetSharedKeyDefinitions();

	vector<string> buttonNames = {
		"A", "B", "X", "Y", "L1", "R1", "Start", "Select",
		"Up", "Down", "Left", "Right", "L2", "R2", "L3", "R3",
		"X+", "X-", "Y+", "Y-", "X2+", "X2-", "Y2+", "Y2-",
		"X", "Y", "X2", "Y2"
	};

	for(int i = 0; i < 20; i++) {
		for(int j = 0; j < (int)buttonNames.size(); j++) {
			_keyDefinitions.push_back({ "Pad" + std::to_string(i + 1) + " " + buttonNames[j], (uint32_t)(MacOSKeyManager::BaseGamepadIndex + i * 0x100 + j) });
		}
	}

	for(KeyDefinition &keyDef : _keyDefinitions) {
		_keyNames[keyDef.keyCode] = keyDef.name;
		_keyCodes[keyDef.name] = keyDef.keyCode;
	}

	_disableAllKeys = false;

	NSEventMask eventMask = NSEventMaskKeyDown | NSEventMaskKeyUp | NSEventMaskFlagsChanged;

	_eventMonitor = [NSEvent addLocalMonitorForEventsMatchingMask:eventMask handler:^ NSEvent* (NSEvent* event) {
		if(_emu->GetSettings()->CheckFlag(EmulationFlags::InBackground)) {
			//Allow UI to handle key-events when main window is not in focus
			return event;
		}

		if([event type] == NSEventTypeKeyDown && ([event modifierFlags] & NSEventModifierFlagCommand) != 0) {
			//Pass through command-based keydown events so cmd+Q etc still works
			return event;
		}

		if([event type] == NSEventTypeFlagsChanged) {
			HandleModifiers((uint32_t) [event modifierFlags]);
		} else {
			uint16_t mappedKeyCode = [event keyCode] >= 128 ? 0 : _keyCodeMap[[event keyCode]];
			_keyState[mappedKeyCode] = ([event type] == NSEventTypeKeyDown);
		}

		return nil;
	}];

	_connectObserver = [[NSNotificationCenter defaultCenter] addObserverForName:GCControllerDidConnectNotification object:nil queue:nil usingBlock:^ void (NSNotification* notification) {
		GCController* controller = (GCController*) [notification object];

		if([controller extendedGamepad] == nil) {
			MessageManager::Log(std::string("[Input] Device ignored (Does not support extended gamepad) - Name: ") + [[controller vendorName] UTF8String]);
		} else {
			_controllers.push_back(std::shared_ptr<MacOSGameController>(new MacOSGameController(_emu, controller)));
			MessageManager::Log(std::string("[Input Connected] Name: ") + [[controller vendorName] UTF8String]);
		}
	}];

	_disconnectObserver = [[NSNotificationCenter defaultCenter] addObserverForName:GCControllerDidDisconnectNotification object:nil queue:nil usingBlock:^ void (NSNotification* notification) {
		GCController* controller = (GCController*) [notification object];

		int indexToRemove = -1;
		for(int i = 0; i < _controllers.size(); i++) {
			if(_controllers[i]->IsGameController(controller)) {
				indexToRemove = i;
				break;
			}
		}

		if(indexToRemove >= 0) {
			_controllers.erase(_controllers.begin() + indexToRemove);
			MessageManager::Log("[Input Device] Disconnected");
		}
	}];
}

MacOSKeyManager::~MacOSKeyManager()
{
	[NSEvent removeMonitor:(id) _eventMonitor];
	[[NSNotificationCenter defaultCenter] removeObserver:(id) _connectObserver];
	[[NSNotificationCenter defaultCenter] removeObserver:(id) _disconnectObserver];
}

void MacOSKeyManager::HandleModifiers(uint32_t flags)
{
	_keyState[116] = (flags & NX_DEVICELSHIFTKEYMASK) != 0; //Left shift
	_keyState[117] = (flags & NX_DEVICERSHIFTKEYMASK) != 0; //Right shift
	_keyState[118] = (flags & NX_DEVICELCTLKEYMASK) != 0; //Left ctrl
	_keyState[119] = (flags & NX_DEVICERCTLKEYMASK) != 0; //Right ctrl
	_keyState[120] = (flags & NX_DEVICELALTKEYMASK) != 0; //Left alt/option
	_keyState[121] = (flags & NX_DEVICERALTKEYMASK) != 0; //Right alt/option
	_keyState[70] = (flags & NX_DEVICELCMDKEYMASK) != 0; //Left cmd
	_keyState[71] = (flags & NX_DEVICERCMDKEYMASK) != 0; //Right cmd
}

void MacOSKeyManager::RefreshState()
{
	//TODO: NOT IMPLEMENTED YET
	//Only needed to detect poll controller input
}

bool MacOSKeyManager::IsKeyPressed(uint16_t key)
{
	if(_disableAllKeys || key == 0) {
		return false;
	}

	if(key >= MacOSKeyManager::BaseGamepadIndex) {
		uint8_t gamepadPort = (key - MacOSKeyManager::BaseGamepadIndex) / 0x100;
		uint8_t gamepadButton = (key - MacOSKeyManager::BaseGamepadIndex) % 0x100;
		if(_controllers.size() > gamepadPort) {
			return _controllers[gamepadPort]->IsButtonPressed(gamepadButton);
		}
	} else if(key < 0x205) {
		return _keyState[key] != 0;
	}
	return false;
}

optional<int16_t> MacOSKeyManager::GetAxisPosition(uint16_t key)
{
	if(key >= MacOSKeyManager::BaseGamepadIndex) {
		uint8_t port = (key - MacOSKeyManager::BaseGamepadIndex) / 0x100;
		uint8_t button = (key - MacOSKeyManager::BaseGamepadIndex) % 0x100;
		return _controllers[port]->GetAxisPosition(button);
	}
	return std::nullopt;
}

bool MacOSKeyManager::IsMouseButtonPressed(MouseButton button)
{
	return _keyState[MacOSKeyManager::BaseMouseButtonIndex + (int)button];
}

vector<uint16_t> MacOSKeyManager::GetPressedKeys()
{
	vector<uint16_t> pressedKeys;
	for(size_t i = 0; i < _controllers.size(); i++) {
		for(int j = 0; j < 24; j++) {
			if(_controllers[i]->IsButtonPressed(j)) {
				pressedKeys.push_back(MacOSKeyManager::BaseGamepadIndex + i * 0x100 + j);
			}
		}
	}

	for(int i = 0; i < 0x205; i++) {
		if(_keyState[i]) {
			pressedKeys.push_back(i);
		}
	}
	return pressedKeys;
}

string MacOSKeyManager::GetKeyName(uint16_t key)
{
	auto keyDef = _keyNames.find(key);
	if(keyDef != _keyNames.end()) {
		return keyDef->second;
	}
	return "";
}

uint16_t MacOSKeyManager::GetKeyCode(string keyName)
{
	auto keyDef = _keyCodes.find(keyName);
	if(keyDef != _keyCodes.end()) {
		return keyDef->second;
	}
	return 0;
}

void MacOSKeyManager::UpdateDevices()
{
	//TODO: NOT IMPLEMENTED YET
	//Only needed to detect newly plugged in devices
}

bool MacOSKeyManager::SetKeyState(uint16_t scanCode, bool state)
{
	if(scanCode < 0x205 && _keyState[scanCode] != state) {
		_keyState[scanCode] = state;
		return true;
	}
	return false;
}

void MacOSKeyManager::ResetKeyState()
{
	memset(_keyState, 0, sizeof(_keyState));
}

void MacOSKeyManager::SetDisabled(bool disabled)
{
	_disableAllKeys = disabled;
}

void MacOSKeyManager::SetForceFeedback(uint16_t magnitude)
{
	for(auto& controller : _controllers) {
		controller->SetForceFeedback(magnitude);
	}
}
