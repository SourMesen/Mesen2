#include "stdafx.h"
#include "PCE/PceVpc.h"
#include "PCE/PceVdc.h"
#include "PCE/PceVce.h"
#include "PCE/PcePsg.h"
#include "PCE/PceMemoryManager.h"
#include "PCE/PceControlManager.h"
#include "PCE/PceConsole.h"
#include "Shared/EmuSettings.h"
#include "Shared/RewindManager.h"
#include "Shared/Video/VideoDecoder.h"
#include "Shared/Video/VideoRenderer.h"
#include "Shared/NotificationManager.h"
#include "Utilities/Serializer.h"
#include "EventType.h"

PceVpc::PceVpc(Emulator* emu, PceConsole* console, PceVce* vce)
{
	_emu = emu;
	_console = console;
	_vce = vce;

	//Add an extra line to the buffer - this is used to store clock divider values for each row
	uint32_t bufferSize = PceConstants::MaxScreenWidth * (PceConstants::ScreenHeight + 1);
	_outBuffer[0] = new uint16_t[bufferSize];
	_outBuffer[1] = new uint16_t[bufferSize];
	_currentOutBuffer = _outBuffer[0];

	memset(_outBuffer[0], 0, bufferSize * sizeof(uint16_t));
	memset(_outBuffer[1], 0, bufferSize * sizeof(uint16_t));
}

PceVpc::~PceVpc()
{
	delete[] _outBuffer[0];
	delete[] _outBuffer[1];
}

void PceVpc::ConnectVdc(PceVdc* vdc1, PceVdc* vdc2)
{
	_vdc1 = vdc1;
	_vdc2 = vdc2;

	if(vdc2) {
		//Default power on values (SuperGrafx only)
		Write(0x08, 0x11);
		Write(0x09, 0x11);
	}
}

uint8_t PceVpc::Read(uint16_t addr)
{
	if(_vdc2) {
		//SuperGrafx enabled
		switch(addr & 0x1F) {
			case 0: case 1: case 2: case 3:
			case 4: case 5: case 6: case 7:
				return _vdc1->ReadRegister(addr);

			case 0x08: return _state.Priority1;
			case 0x09: return _state.Priority2;

			case 0x0A: return (_state.Window1 & 0xFF);
			case 0x0B: return (_state.Window1 >> 8);
			case 0x0C: return (_state.Window2 & 0xFF);
			case 0x0D: return (_state.Window2 >> 8);

			case 0x0E: return 0;
			case 0x0F: return 0;

			case 0x10: case 0x11: case 0x12: case 0x13:
			case 0x14: case 0x15: case 0x16: case 0x17:
				return _vdc2->ReadRegister(addr & 0x03);

			default:
				//18-1F are unused
				return 0xFF;
		}
	} else {
		//Regular PC Engine with a single VDC
		return _vdc1->ReadRegister(addr);
	}
}

void PceVpc::Write(uint16_t addr, uint8_t value)
{
	if(_vdc2) {
		//SuperGrafx enabled
		switch(addr & 0x1F) {
			case 0: case 1: case 2: case 3:
			case 4: case 5: case 6: case 7:
				_vdc1->WriteRegister(addr, value);
				break;

			case 0x08:
				SetPriorityConfig(PceVpcPixelWindow::Both, value & 0x0F);
				SetPriorityConfig(PceVpcPixelWindow::Window2, (value >> 4) & 0x0F);
				_state.Priority1 = value;
				break;

			case 0x09:
				SetPriorityConfig(PceVpcPixelWindow::Window1, value & 0x0F);
				SetPriorityConfig(PceVpcPixelWindow::NoWindow, (value >> 4) & 0x0F);
				_state.Priority2 = value;
				break;

			case 0x0A: _state.Window1 = (_state.Window1 & 0x300) | value; break;
			case 0x0B: _state.Window1 = (_state.Window1 & 0xFF) | ((value & 0x03) << 8); break;
			case 0x0C: _state.Window2 = (_state.Window2 & 0x300) | value; break;
			case 0x0D: _state.Window2 = (_state.Window2 & 0xFF) | ((value & 0x03) << 8); break;

			case 0x0E: _state.StToVdc2Mode = (value & 0x01) != 0; break;

			case 0x10: case 0x11: case 0x12: case 0x13:
			case 0x14: case 0x15: case 0x16: case 0x17:
				_vdc2->WriteRegister(addr & 0x03, value);
				break;

			default:
				//0F, 18-1F are unused
				break;
		}
	} else {
		//Regular PC Engine with a single VDC
		_vdc1->WriteRegister(addr, value);
	}
}

void PceVpc::StVdcWrite(uint16_t addr, uint8_t value)
{
	if(_state.StToVdc2Mode) {
		_vdc2->WriteRegister(addr, value);
	} else {
		_vdc1->WriteRegister(addr, value);
	}
}

void PceVpc::DrawScanline()
{
	if(_vdc2) {
		_vdc2->DrawScanline();
	}
	_vdc1->DrawScanline();
}

void PceVpc::ProcessStartFrame()
{
	if(!_skipRender) {
		_frameSkipTimer.Reset();
	}

	if(_emu->IsRunAheadFrame()) {
		_skipRender = true;
	} else {
		_skipRender = (
			!_emu->GetSettings()->GetVideoConfig().DisableFrameSkipping &&
			!_emu->GetRewindManager()->IsRewinding() &&
			!_emu->GetVideoRenderer()->IsRecording() &&
			(_emu->GetSettings()->GetEmulationSpeed() == 0 || _emu->GetSettings()->GetEmulationSpeed() > 150) &&
			_frameSkipTimer.GetElapsedMS() < 10
		);
	}
}

void PceVpc::ProcessScanlineStart(PceVdc* vdc, uint16_t scanline)
{
	if(vdc == _vdc2 || _skipRender) {
		return;
	}

	if(scanline >= 14 && scanline < 256) {
		uint16_t row = scanline - 14;
		if(row == 0) {
			_currentOutBuffer = _currentOutBuffer == _outBuffer[0] ? _outBuffer[1] : _outBuffer[0];
		}
		
		//Store clock dividers for each row at the end of the buffer
		_currentOutBuffer[PceConstants::MaxScreenWidth * PceConstants::ScreenHeight + row] = _vce->GetClockDivider();
	}
}

void PceVpc::ProcessScanlineEnd(PceVdc* vdc, uint16_t scanline, uint16_t* rowBuffer)
{
	if(vdc == _vdc2 || _skipRender) {
		return;
	}

	uint32_t offset = (scanline - 14) * PceConstants::MaxScreenWidth;
	if(_vdc2) {
		//Supergrafx mode, merge outputs
		uint16_t* rowBufferVdc2 = _vdc2->GetRowBuffer();
		
		uint32_t pixelCount = PceConstants::ClockPerScanline / _vce->GetClockDivider();
		for(uint32_t i = 0; i < pixelCount; i++) {
			PceVpcPriorityMode prio = (PceVpcPriorityMode)((i < _state.Window1) | ((i < _state.Window2) << 1));
			PceVpcPriorityConfig& cfg = _state.WindowCfg[(int)prio];
			uint8_t enabledLayers = (uint8_t)cfg.Vdc1Enabled | ((uint8_t)cfg.Vdc2Enabled << 1);
			uint16_t color;
			switch(enabledLayers) {
				default:
				case 0: color = 0; break;
				case 1: color = rowBuffer[i]; break;
				case 2: color = rowBufferVdc2[i]; break;

				case 3:
				{
					bool isSpriteVdc1 = (rowBuffer[i] & PceVpc::SpritePixelFlag) != 0;
					bool isSpriteVdc2 = (rowBufferVdc2[i] & PceVpc::SpritePixelFlag) != 0;
					bool isTransparentVdc1 = (rowBuffer[i] & PceVpc::TransparentPixelFlag) != 0;

					switch(cfg.PriorityMode) {
						default:
						case PceVpcPriorityMode::Default:
							color = isTransparentVdc1 ? rowBufferVdc2[i] : rowBuffer[i];
							break;

						case PceVpcPriorityMode::Vdc2SpritesAboveVdc1Bg:
							if(isTransparentVdc1 || (isSpriteVdc2 && !isSpriteVdc1)) {
								//VDC1 transparent, show VDC2, or
								//VDC2 is a sprite and VDC1 is not a sprite, show VDC2
								color = rowBufferVdc2[i];
							} else {
								color = rowBuffer[i];
							}
							break;

						case PceVpcPriorityMode::Vdc1SpritesBelowVdc2Bg:
							if(isTransparentVdc1 || (isSpriteVdc1 && !isSpriteVdc2)) {
								//VDC1 transparent, show VDC2, or
								//VDC1 is a sprite, show VDC2
								color = rowBufferVdc2[i];
							} else {
								color = rowBuffer[i];
							}
							break;
					}
					break;
				}
			}

			_currentOutBuffer[offset + i] = color;
		}
	} else {
		//PC Engine, display VDC1's data
		memcpy(_currentOutBuffer + offset, rowBuffer, PceConstants::ClockPerScanline / _vce->GetClockDivider() * sizeof(uint16_t));
	}
}

void PceVpc::SendFrame(PceVdc* vdc)
{
	if(vdc == _vdc2) {
		return;
	}

	_emu->ProcessEvent(EventType::EndFrame);
	_emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::PpuFrameDone, _currentOutBuffer);

	bool forRewind = _emu->GetRewindManager()->IsRewinding();

	if(!_skipRender) {
		if(_console->GetRomFormat() == RomFormat::PceHes) {
			RenderedFrame frame(_currentOutBuffer, 256, 240, 1.0, _vdc1->GetState().FrameCount, _console->GetControlManager()->GetPortStates());
			_emu->GetVideoDecoder()->UpdateFrame(frame, forRewind, forRewind);
		} else {
			RenderedFrame frame(_currentOutBuffer, PceConstants::InternalOutputWidth, PceConstants::InternalOutputHeight, 1.0 / PceConstants::InternalResMultipler, _vdc1->GetState().FrameCount, _console->GetControlManager()->GetPortStates());
			_emu->GetVideoDecoder()->UpdateFrame(frame, forRewind, forRewind);
		}
	}

	_console->GetPsg()->Run();
	_emu->ProcessEndOfFrame();

	_console->GetControlManager()->UpdateInputState();
	_console->GetControlManager()->UpdateControlDevices();
}

void PceVpc::DebugSendFrame()
{
	uint16_t scanline = _vdc1->GetScanline();
	if(scanline >= 14 && scanline < 256) {
		DrawScanline();
		ProcessScanlineEnd(_vdc1, scanline, _vdc1->GetRowBuffer());
	
		uint32_t lastPixel = _vdc1->GetHClock() / _vce->GetClockDivider();
		int offset = std::max(0, (int)(lastPixel + (scanline - 14) * PceConstants::MaxScreenWidth));
		int pixelsToClear = PceConstants::MaxScreenWidth * PceConstants::ScreenHeight - offset;
		if(pixelsToClear > 0) {
			memset(_currentOutBuffer + offset, 0, pixelsToClear * sizeof(uint16_t));
		}
	}

	RenderedFrame frame(_currentOutBuffer, PceConstants::InternalOutputWidth, PceConstants::InternalOutputHeight, 0.25, _vdc1->GetState().FrameCount);
	_emu->GetVideoDecoder()->UpdateFrame(frame, false, false);
}

void PceVpc::UpdateIrqState()
{
	if(_hasIrqVdc1 || _hasIrqVdc2) {
		_console->GetMemoryManager()->SetIrqSource(PceIrqSource::Irq1);
	} else {
		_console->GetMemoryManager()->ClearIrqSource(PceIrqSource::Irq1);
	}
}

void PceVpc::SetIrq(PceVdc* vdc)
{
	if(vdc == _vdc1) {
		_hasIrqVdc1 = true;
	} else {
		_hasIrqVdc2 = true;
	}
	UpdateIrqState();
}

void PceVpc::ClearIrq(PceVdc* vdc)
{
	if(vdc == _vdc1) {
		_hasIrqVdc1 = false;
	} else {
		_hasIrqVdc2 = false;
	}
	UpdateIrqState();
}

void PceVpc::SetPriorityConfig(PceVpcPixelWindow wnd, uint8_t value)
{
	PceVpcPriorityConfig& cfg = _state.WindowCfg[(int)wnd];
	cfg.Vdc1Enabled = (value & 0x01) != 0;
	cfg.Vdc2Enabled = (value & 0x02) != 0;
	switch((value >> 2) & 0x03) {
		case 0: cfg.PriorityMode = PceVpcPriorityMode::Default; break;
		case 1: cfg.PriorityMode = PceVpcPriorityMode::Vdc2SpritesAboveVdc1Bg; break;
		case 2: cfg.PriorityMode = PceVpcPriorityMode::Vdc1SpritesBelowVdc2Bg; break;
		case 3: cfg.PriorityMode = PceVpcPriorityMode::Default; break;
	}
}

void PceVpc::Serialize(Serializer& s)
{
	SV(_hasIrqVdc1);
	SV(_hasIrqVdc2);
	SV(_state.Priority1);
	SV(_state.Priority2);
	SV(_state.StToVdc2Mode);
	SV(_state.Window1);
	SV(_state.Window2);
	for(int i = 0; i < 4; i++) {
		SVI(_state.WindowCfg[i].PriorityMode);
		SVI(_state.WindowCfg[i].Vdc1Enabled);
		SVI(_state.WindowCfg[i].Vdc2Enabled);
	}
}
