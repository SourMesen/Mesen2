#include "pch.h"
#include "Shared/InputHud.h"
#include "Shared/Emulator.h"
#include "Shared/BaseControlManager.h"
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

		case 6:
			_hud->DrawLine(x + _xOffset, y + _yOffset, x + 2 + _xOffset, y + _yOffset, color[0], 1);
			_hud->DrawLine(x + _xOffset, 1 + y + _yOffset, x + _xOffset, y + _yOffset + 3, color[0], 1);
			_hud->DrawLine(x + _xOffset, 2 + y + _yOffset, x + 2 + _xOffset, 2 + y + _yOffset, color[0], 1);
			_hud->DrawPixel(x + 2 + _xOffset, 3 + y + _yOffset, color[0], 1);
			_hud->DrawLine(x + _xOffset, 4 + y + _yOffset, x + 2 + _xOffset, 4 + y + _yOffset, color[0], 1);
			break;

		case 7:
			_hud->DrawLine(x + _xOffset, y + _yOffset, x + 2 + _xOffset, y + _yOffset, color[0], 1);
			_hud->DrawLine(x + _xOffset + 2, y + _yOffset, x + _xOffset + 2, 4 + y + _yOffset, color[0], 1);
			break;

		case 8:
			_hud->DrawRectangle(x + _xOffset, y + _yOffset, 3, 5, color[0], false, 1);
			_hud->DrawPixel(x + _xOffset + 1, y + _yOffset + 2, color[0], 1);
			break;

		default:
			break;
	}
}

void InputHud::DrawMousePosition(MousePosition pos)
{
	if(pos.X >= 0 && pos.Y >= 0) {
		//These are drawn on the "debug"/"lua" HUD because its size always matches the console's output size
		//Drawing on _hud causes issues when the "fixed size" option is selected
		OverscanDimensions overscan = _emu->GetSettings()->GetOverscan();
		_emu->GetDebugHud()->DrawRectangle(pos.X - 1 - overscan.Left, pos.Y - 1 - overscan.Top, 3, 3, 0x00FF0000, true, 1);
		_emu->GetDebugHud()->DrawRectangle(pos.X - 1 - overscan.Left, pos.Y - 1 - overscan.Top, 3, 3, 0x00808080, false, 1);
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

void InputHud::DrawController(ControllerData& data, BaseControlManager* controlManager)
{
	shared_ptr<BaseControlDevice> controller = controlManager->CreateControllerDevice(data.Type, data.Port);
	if(!controller) {
		return;
	}

	controller->SetRawState(data.State);
	controller->DrawController(*this);
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

	_controllerIndex++;
}

void InputHud::DrawControllers(FrameInfo size, vector<ControllerData> controllerData)
{
	if(_emu->GetAudioPlayerHud()) {
		//Don't draw controllers when playing an audio file
		return;
	}

	shared_ptr<IConsole> console = _emu->GetConsole();
	if(!console) {
		return;
	}

	InputConfig& cfg = _emu->GetSettings()->GetInputConfig();
	
	bool hasVisiblePort = false;
	for(int i = 0; i < 8; i++) {
		hasVisiblePort |= cfg.DisplayInputPort[i];
	}

	if(!hasVisiblePort) {
		return;
	}

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
	
	_controllerIndex = 0;
	for(ControllerData& portData : controllerData) {
		DrawController(portData, console->GetControlManager());
	}
}
