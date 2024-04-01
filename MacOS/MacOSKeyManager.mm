#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#include <algorithm>
#include "MacOSKeyManager.h"
//The MacOS SDK defines a global function 'Debugger', colliding with Mesen's Debugger class
//Redefine it temporarily so the headers don't cause compilation errors due to this
#define Debugger MesenDebugger
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/KeyDefinitions.h"
#include "Shared/SettingTypes.h"
#include "MacOS/MacOSMouseManager.h"
#undef Debugger

MacOSKeyManager::MacOSKeyManager(Emulator* emu)
{
	_emu = emu;

	ResetKeyState();

	_keyDefinitions = KeyDefinition::GetSharedKeyDefinitions();

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
}

MacOSKeyManager::~MacOSKeyManager()
{
	[NSEvent removeMonitor:(id) _eventMonitor];
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

	if(key < 0x205) {
		return _keyState[key] != 0;
	}
	return false;
}

bool MacOSKeyManager::IsMouseButtonPressed(MouseButton button)
{
	return _keyState[MacOSKeyManager::BaseMouseButtonIndex + (int)button];
}

vector<uint16_t> MacOSKeyManager::GetPressedKeys()
{
	vector<uint16_t> pressedKeys;

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
