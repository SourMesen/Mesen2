#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#include "MacOSMouseManager.h"

double MacOSMouseManager::_relativeX = 0.0;
double MacOSMouseManager::_relativeY = 0.0;
bool MacOSMouseManager::_mouseCaptured = false;
bool MacOSMouseManager::_cursorHidden = false;

MouseState MacOSMouseManager::GetMouseState()
{
	MouseState state = {};
	if(_mouseCaptured) {
		state.XPosition = _relativeX;
		state.YPosition = _relativeY;
	} else {
		NSPoint location = [NSEvent mouseLocation];
		state.XPosition = location.x;
		state.YPosition = CGDisplayPixelsHigh(kCGDirectMainDisplay) - location.y;
	}
	NSUInteger buttons = [NSEvent pressedMouseButtons];
	state.LeftButton = (buttons & 1) != 0;
	state.RightButton = (buttons & 2) != 0;
	state.MiddleButton = (buttons & 4) != 0;
	state.Button4 = (buttons & 8) != 0;
	state.Button5 = (buttons & 16) != 0;
	return state;
}

void MacOSMouseManager::SetMouseCaptured(bool enabled)
{
	if(enabled) {
		NSPoint location = [NSEvent mouseLocation];
		CGAssociateMouseAndMouseCursorPosition(NO);
		_relativeX = location.x;
		_relativeY = CGDisplayPixelsHigh(kCGDirectMainDisplay) - location.y;
	} else {
		CGAssociateMouseAndMouseCursorPosition(YES);
	}
	_mouseCaptured = enabled;
}

void MacOSMouseManager::SetMousePosition(double x, double y)
{
	_relativeX = x;
	_relativeY = y;
}

void MacOSMouseManager::SetRelativeMovement(double x, double y)
{
	_relativeX += x;
	_relativeY += y;
}

void MacOSMouseManager::SetCursor(CursorIcon cursor) {
	if(cursor == CursorIcon::Hidden && !_cursorHidden) {
		[NSCursor hide];
		_cursorHidden = true;
	}
	if(cursor != CursorIcon::Hidden && _cursorHidden) {
		[NSCursor unhide];
		_cursorHidden = false;
	}
	switch(cursor) {
		case CursorIcon::Hidden: break; //Already handled above
		case CursorIcon::Arrow: [[NSCursor arrowCursor] set]; break;
		case CursorIcon::Cross: [[NSCursor crosshairCursor] set]; break;
	}
}

double MacOSMouseManager::GetPixelScale()
{
	//On MacOS, Avalonia seems to have a scaling-mismatch between getting/converting points and getting element sizes
	//points seem to be given in DPI-aware pixels (as GetMouseState returns as well), but sizes in actual screen-pixels
	//The result of this function is used in the UI to correct for this with mouse-related usage
	NSScreen* screen = [NSScreen mainScreen];
	if(screen == nil) {
		return 1.0;
	}
	return [screen backingScaleFactor];
}
