#include "stdafx.h"
#include "Shared/InputHud.h"
#include "Shared/Emulator.h"
#include "Shared/BaseControlManager.h"
#include "Shared/Interfaces/IControlManager.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/EmuSettings.h"
#include "Shared/Video/DebugHud.h"

static constexpr int color[2] = { 0x00111111, 0x00FFFFFF };

InputHud::InputHud(Emulator* emu, DebugHud* hud)
{
	_emu = emu;
	_hud = hud;
}

void InputHud::DrawButton(int x, int y, int width, int height, bool pressed)
{
	_hud->DrawRectangle(_xOffset + x, _yOffset + y, width, height, color[pressed], true, 1);
}

void InputHud::DrawNumber(int number, int x, int y)
{
	switch(number) {
		case 1:
			_hud->DrawLine(x+1 + _xOffset, y + _yOffset, x+1 + _xOffset, 4 + y + _yOffset, color[0], 1);
			_hud->DrawLine(x + _xOffset, 4 + y + _yOffset, x+2 + _xOffset, 4 + y + _yOffset, color[0], 1);
			_hud->DrawPixel(x + _xOffset, 1 + y + _yOffset, color[0], 1);
			break;

		case 2:
			_hud->DrawLine(x + _xOffset, y + _yOffset, x+2 + _xOffset, y + _yOffset, color[0], 1);
			_hud->DrawPixel(x+2 + _xOffset, 1 + y + _yOffset, color[0], 1);
			_hud->DrawLine(x + _xOffset, 2 + y + _yOffset, x+2 + _xOffset, 2 + y + _yOffset, color[0], 1);
			_hud->DrawPixel(x + _xOffset, 3 + y + _yOffset, color[0], 1);
			_hud->DrawLine(x + _xOffset, 4 + y + _yOffset, x+2 + _xOffset, 4 + y + _yOffset, color[0], 1);
			break;

		case 3:
			_hud->DrawLine(x + _xOffset, y + _yOffset, x+2 + _xOffset, y + _yOffset, color[0], 1);
			_hud->DrawPixel(x+2 + _xOffset, 1 + y + _yOffset, color[0], 1);
			_hud->DrawLine(x + _xOffset, 2 + y + _yOffset, x+2 + _xOffset, 2 + y + _yOffset, color[0], 1);
			_hud->DrawPixel(x+2 + _xOffset, 3 + y + _yOffset, color[0], 1);
			_hud->DrawLine(x + _xOffset, 4 + y + _yOffset, x+2 + _xOffset, 4 + y + _yOffset, color[0], 1);
			break;

		case 4:
			_hud->DrawLine(x + _xOffset, y + _yOffset, x + _xOffset, 2 + y + _yOffset, color[0], 1);
			_hud->DrawLine(x+2 + _xOffset, y + _yOffset, x+2 + _xOffset, 4 + y + _yOffset, color[0], 1);
			_hud->DrawLine(x + _xOffset, 2 + y + _yOffset, x+2 + _xOffset, 2 + y + _yOffset, color[0], 1);
			break;

		case 5:
			_hud->DrawLine(x + _xOffset, y + _yOffset, x+2 + _xOffset, y + _yOffset, color[0], 1);
			_hud->DrawPixel(x + _xOffset, 1 + y + _yOffset, color[0], 1);
			_hud->DrawLine(x + _xOffset, 2 + y + _yOffset, x+2 + _xOffset, 2 + y + _yOffset, color[0], 1);
			_hud->DrawPixel(x+2 + _xOffset, 3 + y + _yOffset, color[0], 1);
			_hud->DrawLine(x + _xOffset, 4 + y + _yOffset, x+2 + _xOffset, 4 + y + _yOffset, color[0], 1);
			break;

		default:
			break;
	}
}

void InputHud::DrawOutline(int width, int height)
{
	InputConfig& cfg = _emu->GetSettings()->GetInputConfig();

	switch(cfg.DisplayInputPosition) {
		default:
		case InputDisplayPosition::TopLeft:
			break;

		case InputDisplayPosition::TopRight:
			_xOffset -= width + 1;
			break;

		case InputDisplayPosition::BottomLeft:
			_yOffset -= height + 1;
			break;

		case InputDisplayPosition::BottomRight:
			_yOffset -= height + 1;
			_xOffset -= width + 1;
			break;
	}

	_hud->DrawRectangle(_xOffset, _yOffset, width, height, 0x80CCCCCC, true, 1);
	_hud->DrawRectangle(_xOffset, _yOffset, width, height, color[0], false, 1);
	
	_outlineWidth = width;
	_outlineHeight = height;
}

void InputHud::DrawController(ControllerType controllerType, int port, ControlDeviceState state)
{
	shared_ptr<BaseControlDevice> controller = ((BaseControlManager*)_emu->GetControlManager())->CreateControllerDevice(controllerType, port);
	if(!controller) {
		return;
	}

	controller->SetRawState(state);
	controller->DrawController(*this);
	EndDrawController();
}

void InputHud::EndDrawController()
{
	if(_outlineHeight > 0 && _outlineWidth > 0) {
		InputConfig& cfg = _emu->GetSettings()->GetInputConfig();

		switch(cfg.DisplayInputPosition) {
			default:
			case InputDisplayPosition::TopLeft:
				if(cfg.DisplayInputHorizontally) {
					_xOffset += _outlineWidth + 1;
				} else {
					_yOffset += _outlineHeight + 1;
				}
				break;

			case InputDisplayPosition::TopRight:
				if(!cfg.DisplayInputHorizontally) {
					_xOffset += _outlineWidth + 1;
					_yOffset += _outlineHeight + 1;
				}
				break;

			case InputDisplayPosition::BottomLeft:
				if(cfg.DisplayInputHorizontally) {
					_xOffset += _outlineWidth + 1;
					_yOffset += _outlineHeight + 1;
				}
				break;

			case InputDisplayPosition::BottomRight:
				if(cfg.DisplayInputHorizontally) {
					_yOffset += _outlineHeight + 1;
				} else {
					_xOffset += _outlineWidth + 1;
				}
				break;
		}

		_outlineWidth = 0;
		_outlineHeight = 0;
	}
}

void InputHud::DrawControllers(FrameInfo size, vector<ControllerData> controllerData)
{
	InputConfig& cfg = _emu->GetSettings()->GetInputConfig();

	switch(cfg.DisplayInputPosition) {
		default:
		case InputDisplayPosition::TopLeft:
			_xOffset = 2;
			_yOffset = 2;
			break;
		case InputDisplayPosition::TopRight:
			_xOffset = size.Width - 1;
			_yOffset = 2;
			break;
		case InputDisplayPosition::BottomLeft:
			_xOffset = 2;
			_yOffset = size.Height - 1;
			break;
		case InputDisplayPosition::BottomRight:
			_xOffset = size.Width - 1;
			_yOffset = size.Height - 1;
			break;
	}
	
	for(int i = 0; i < (int)controllerData.size(); i++) {
		if(controllerData[i].Type != ControllerType::None) {
			DrawController(controllerData[i].Type, i, controllerData[i].State);
		}
	}
}
