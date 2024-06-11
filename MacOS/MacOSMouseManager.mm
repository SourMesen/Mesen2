#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#include "MacOSMouseManager.h"

MacOSMouseManager::MacOSMouseManager()
{
	_relativeX = 0.0;
	_relativeY = 0.0;
	_mouseCaptured = false;
	_cursorHidden = false;

	NSEventMask eventMask = NSEventMaskMouseMoved | NSEventMaskLeftMouseDragged | NSEventMaskRightMouseDragged | NSEventMaskOtherMouseDragged;

	_eventMonitor = [NSEvent addLocalMonitorForEventsMatchingMask:eventMask handler:^ NSEvent* (NSEvent* event) {
		//When mouse is captured on MacOS, absolute position is frozen and only deltaX/Y gives movement data
		SetRelativeMovement([event deltaX], [event deltaY]);
		return event;
	}];
}

MacOSMouseManager::~MacOSMouseManager()
{
	[NSEvent removeMonitor:(id) _eventMonitor];
}

SystemMouseState MacOSMouseManager::GetSystemMouseState(void* rendererHandle)
{
	SystemMouseState state = {};
	if(_mouseCaptured) {
		state.XPosition = (int32_t) _relativeX;
		state.YPosition = (int32_t) _relativeY;
	} else {
		NSPoint location = [NSEvent mouseLocation];
		state.XPosition = (int32_t) location.x;
		state.YPosition = (int32_t) (CGDisplayPixelsHigh(kCGDirectMainDisplay) - location.y);
	}
	NSUInteger buttons = [NSEvent pressedMouseButtons];
	state.LeftButton = (buttons & 1) != 0;
	state.RightButton = (buttons & 2) != 0;
	state.MiddleButton = (buttons & 4) != 0;
	state.Button4 = (buttons & 8) != 0;
	state.Button5 = (buttons & 16) != 0;
	return state;
}

bool MacOSMouseManager::CaptureMouse(int32_t x, int32_t y, int32_t width, int32_t height, void* rendererHandle)
{
	NSPoint location = [NSEvent mouseLocation];
	CGAssociateMouseAndMouseCursorPosition(NO);
	_relativeX = location.x;
	_relativeY = CGDisplayPixelsHigh(kCGDirectMainDisplay) - location.y;
	_mouseCaptured = true;
	return true;
}

void MacOSMouseManager::ReleaseMouse()
{
	CGAssociateMouseAndMouseCursorPosition(YES);
	_mouseCaptured = false;
}

void MacOSMouseManager::SetSystemMousePosition(int32_t x, int32_t y)
{
	_relativeX = (double) x;
	_relativeY = (double) y;
}

void MacOSMouseManager::SetRelativeMovement(double x, double y)
{
	_relativeX += x;
	_relativeY += y;
}

void MacOSMouseManager::SetCursorImage(CursorImage cursor)
{
	if(cursor == CursorImage::Hidden && !_cursorHidden) {
		[NSCursor hide];
		_cursorHidden = true;
	}
	if(cursor != CursorImage::Hidden && _cursorHidden) {
		[NSCursor unhide];
		_cursorHidden = false;
	}
	switch(cursor) {
		case CursorImage::Hidden: break; //Already handled above
		case CursorImage::Arrow: [[NSCursor arrowCursor] set]; break;
		case CursorImage::Cross: [[NSCursor crosshairCursor] set]; break;
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
