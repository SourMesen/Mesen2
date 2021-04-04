#include "stdafx.h"
#include "InputHud.h"
#include "BaseControlDevice.h"
#include "SnesController.h"
#include "Multitap.h"
#include "SnesMouse.h"
#include "SuperScope.h"
#include "Console.h"
#include "EmuSettings.h"
#include "DebugHud.h"
#include "ControlManager.h"

static constexpr int color[2] = { 0x00111111, 0x00FFFFFF };

InputHud::InputHud(Emulator* emu)
{
	_emu = emu;
}

void InputHud::DrawController(int port, ControlDeviceState state, int x, int y, int frameNumber)
{
	//TODO
	/*SnesController controller(_console, 0, KeyMappingSet());
	controller.SetRawState(state);

	shared_ptr<DebugHud> hud = _console->GetDebugHud();
	hud->DrawRectangle(0 + x, 0 + y, 35, 14, 0x80CCCCCC, true, 1, frameNumber);
	hud->DrawRectangle(0 + x, 0 + y, 35, 14, color[0], false, 1, frameNumber);
	hud->DrawRectangle(5 + x, 3 + y, 3, 3, color[controller.IsPressed(SnesController::Buttons::Up)], true, 1, frameNumber);
	hud->DrawRectangle(5 + x, 9 + y, 3, 3, color[controller.IsPressed(SnesController::Buttons::Down)], true, 1, frameNumber);
	hud->DrawRectangle(2 + x, 6 + y, 3, 3, color[controller.IsPressed(SnesController::Buttons::Left)], true, 1, frameNumber);
	hud->DrawRectangle(8 + x, 6 + y, 3, 3, color[controller.IsPressed(SnesController::Buttons::Right)], true, 1, frameNumber);
	hud->DrawRectangle(5 + x, 6 + y, 3, 3, color[0], true, 1, frameNumber);

	hud->DrawRectangle(27 + x, 3 + y, 3, 3, color[controller.IsPressed(SnesController::Buttons::X)], true, 1, frameNumber);
	hud->DrawRectangle(27 + x, 9 + y, 3, 3, color[controller.IsPressed(SnesController::Buttons::B)], true, 1, frameNumber);
	hud->DrawRectangle(30 + x, 6 + y, 3, 3, color[controller.IsPressed(SnesController::Buttons::A)], true, 1, frameNumber);
	hud->DrawRectangle(24 + x, 6 + y, 3, 3, color[controller.IsPressed(SnesController::Buttons::Y)], true, 1, frameNumber);

	hud->DrawRectangle(4 + x, 0 + y, 5, 2, color[controller.IsPressed(SnesController::Buttons::L)], true, 1, frameNumber);
	hud->DrawRectangle(26 + x, 0 + y, 5, 2, color[controller.IsPressed(SnesController::Buttons::R)], true, 1, frameNumber);

	hud->DrawRectangle(13 + x, 9 + y, 4, 2, color[controller.IsPressed(SnesController::Buttons::Select)], true, 1, frameNumber);
	hud->DrawRectangle(18 + x, 9 + y, 4, 2, color[controller.IsPressed(SnesController::Buttons::Start)], true, 1, frameNumber);

	switch(port) {
		case 0:
			//1
			hud->DrawLine(17 + x, 2 + y, 17 + x, 6 + y, color[0], 1, frameNumber);
			hud->DrawLine(16 + x, 6 + y, 18 + x, 6 + y, color[0], 1, frameNumber);
			hud->DrawPixel(16 + x, 3 + y, color[0], 1, frameNumber);
			break;

		case 1:
			//2
			hud->DrawLine(16 + x, 2 + y, 18 + x, 2 + y, color[0], 1, frameNumber);
			hud->DrawPixel(18 + x, 3 + y, color[0], 1, frameNumber);
			hud->DrawLine(16 + x, 4 + y, 18 + x, 4 + y, color[0], 1, frameNumber);
			hud->DrawPixel(16 + x, 5 + y, color[0], 1, frameNumber);
			hud->DrawLine(16 + x, 6 + y, 18 + x, 6 + y, color[0], 1, frameNumber);
			break;

		case 2:
			//3
			hud->DrawLine(16 + x, 2 + y, 18 + x, 2 + y, color[0], 1, frameNumber);
			hud->DrawPixel(18 + x, 3 + y, color[0], 1, frameNumber);
			hud->DrawLine(16 + x, 4 + y, 18 + x, 4 + y, color[0], 1, frameNumber);
			hud->DrawPixel(18 + x, 5 + y, color[0], 1, frameNumber);
			hud->DrawLine(16 + x, 6 + y, 18 + x, 6 + y, color[0], 1, frameNumber);
			break;

		case 3:
			//4
			hud->DrawLine(16 + x, 2 + y, 16 + x, 4 + y, color[0], 1, frameNumber);
			hud->DrawLine(18 + x, 2 + y, 18 + x, 6 + y, color[0], 1, frameNumber);
			hud->DrawLine(16 + x, 4 + y, 18 + x, 4 + y, color[0], 1, frameNumber);
			break;

		case 4:
			//5
			hud->DrawLine(16 + x, 2 + y, 18 + x, 2 + y, color[0], 1, frameNumber);
			hud->DrawPixel(16 + x, 3 + y, color[0], 1, frameNumber);
			hud->DrawLine(16 + x, 4 + y, 18 + x, 4 + y, color[0], 1, frameNumber);
			hud->DrawPixel(18 + x, 5 + y, color[0], 1, frameNumber);
			hud->DrawLine(16 + x, 6 + y, 18 + x, 6 + y, color[0], 1, frameNumber);
			break;
	}*/
}

void InputHud::DrawControllers(OverscanDimensions overscan, int frameNumber)
{
	//TODO
	/*
	vector<ControllerData> controllerData = _console->GetControlManager()->GetPortStates();
	InputConfig cfg = _console->GetSettings()->GetInputConfig();

	int xStart;
	int yStart;
	int xOffset = cfg.DisplayInputHorizontally ? 38 : 0;
	int yOffset = cfg.DisplayInputHorizontally ? 0 : 16;

	switch(cfg.DisplayInputPosition) {
		default:
		case InputDisplayPosition::TopLeft:
			xStart = overscan.Left + 3;
			yStart = overscan.Top + 3;
			break;
		case InputDisplayPosition::TopRight:
			xStart = 256 - overscan.Right - 38;
			yStart = overscan.Top + 3;
			xOffset = -xOffset;
			break;
		case InputDisplayPosition::BottomLeft:
			xStart = overscan.Left + 3;
			yStart = 240 - overscan.Bottom - 18;
			yOffset = -yOffset;
			break;
		case InputDisplayPosition::BottomRight:
			xStart = 256 - overscan.Right - 38;
			yStart = 240 - overscan.Bottom - 18;
			xOffset = -xOffset;
			yOffset = -yOffset;
			break;
	}

	for(int i = 0; i < (int)controllerData.size(); i++) {
		if(controllerData[i].Type == ControllerType::SnesController) {
			if(cfg.DisplayInputPort[i]) {
				DrawController(i, controllerData[i].State, xStart, yStart, frameNumber);
				xStart += xOffset;
				yStart += yOffset;
			}
		} else if(controllerData[i].Type == ControllerType::Multitap) {
			uint64_t rawData = 0;
			for(int j = (int)controllerData[i].State.State.size() - 1; j >= 0; j--) {
				rawData <<= 8;
				rawData |= controllerData[i].State.State[j];
			}

			ControlDeviceState controllers[4] = {};
			for(int j = 0; j < 4; j++) {
				controllers[j].State.push_back(rawData & 0xFF);
				controllers[j].State.push_back((rawData >> 8) & 0x0F);
				rawData >>= 12;
			}

			if(cfg.DisplayInputPort[i]) {
				DrawController(i, controllers[0], xStart, yStart, frameNumber);
				xStart += xOffset;
				yStart += yOffset;
			}

			for(int j = 1; j < 4; j++) {
				if(cfg.DisplayInputPort[j + 1]) {
					DrawController(j + 1, controllers[j], xStart, yStart, frameNumber);
					xStart += xOffset;
					yStart += yOffset;
				}
			}
		} else if(controllerData[i].Type == ControllerType::SuperScope) {
			if(cfg.DisplayInputPort[i]) {
				SuperScope scope(_console, 0, KeyMappingSet());
				scope.SetRawState(controllerData[i].State);
				MousePosition pos = scope.GetCoordinates();

				shared_ptr<DebugHud> hud = _console->GetDebugHud();
				hud->DrawRectangle(pos.X - 1, pos.Y - 1, 3, 3, 0x00111111, true, 1, frameNumber);
				hud->DrawRectangle(pos.X - 1, pos.Y - 1, 3, 3, 0x80CCCCCC, false, 1, frameNumber);
			}
		} else if(controllerData[i].Type == ControllerType::SnesMouse) {
			if(cfg.DisplayInputPort[i]) {
				SnesMouse mouse(_console, 0);
				mouse.SetRawState(controllerData[i].State);

				shared_ptr<DebugHud> hud = _console->GetDebugHud();
				hud->DrawRectangle(xStart + 12, yStart, 11, 14, 0x00AAAAAA, true, 1, frameNumber);
				hud->DrawRectangle(xStart + 12, yStart, 11, 14, color[0], false, 1, frameNumber);
				hud->DrawRectangle(xStart + 13, yStart + 1, 4, 5, color[mouse.IsPressed(SnesMouse::Buttons::Left)], true, 1, frameNumber);
				hud->DrawRectangle(xStart + 18, yStart + 1, 4, 5, color[mouse.IsPressed(SnesMouse::Buttons::Right)], true, 1, frameNumber);

				xStart += xOffset;
				yStart += yOffset;
			}
		}
	}*/
}
