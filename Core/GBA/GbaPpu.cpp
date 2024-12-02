#include "pch.h"
#include "GBA/GbaPpu.h"
#include "GBA/GbaTypes.h"
#include "GBA/GbaConsole.h"
#include "GBA/GbaMemoryManager.h"
#include "GBA/GbaDmaController.h"
#include "GBA/Debugger/GbaPpuTools.h"
#include "Debugger/DebugTypes.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/BaseControlManager.h"
#include "Shared/RewindManager.h"
#include "Shared/MessageManager.h"
#include "Shared/RenderedFrame.h"
#include "Shared/Video/VideoDecoder.h"
#include "Shared/Video/VideoRenderer.h"
#include "Shared/EventType.h"
#include "Shared/NotificationManager.h"
#include "Utilities/BitUtilities.h"
#include "Utilities/Serializer.h"
#include "Utilities/StaticFor.h"

void GbaPpu::Init(Emulator* emu, GbaConsole* console, GbaMemoryManager* memoryManager)
{
	_emu = emu;
	_console = console;
	_memoryManager = memoryManager;

	_state = {};

	_paletteRam = (uint16_t*)_emu->GetMemory(MemoryType::GbaPaletteRam).Memory;
	_vram = (uint8_t*)_emu->GetMemory(MemoryType::GbaVideoRam).Memory;
	_vram16 = (uint16_t*)_emu->GetMemory(MemoryType::GbaVideoRam).Memory;
	_oam = (uint32_t*)_emu->GetMemory(MemoryType::GbaSpriteRam).Memory;

	_outputBuffers[0] = new uint16_t[GbaConstants::PixelCount];
	_outputBuffers[1] = new uint16_t[GbaConstants::PixelCount];
	memset(_outputBuffers[0], 0, GbaConstants::PixelCount * sizeof(uint16_t));
	memset(_outputBuffers[1], 0, GbaConstants::PixelCount * sizeof(uint16_t));
	_currentBuffer = _outputBuffers[0];

	_oamReadOutput = _oamOutputBuffers[0];
	_oamWriteOutput = _oamOutputBuffers[1];

	if(_emu->GetSettings()->GetGbaConfig().SkipBootScreen) {
		//BIOS leaves PPU registers in this state, some games expect this
		_state.Control = 0x80;
		_state.ForcedBlank = true;
		_state.Transform[0].Matrix[0] = 0x100;
		_state.Transform[0].Matrix[3] = 0x100;
		_state.Transform[1].Matrix[0] = 0x100;
		_state.Transform[1].Matrix[3] = 0x100;
	}

	//All layers are always active when no window is enabled
	for(int i = 0; i < 6; i++) {
		_state.WindowActiveLayers[4][i] = true;
	}
	
	StaticFor<0, 128>::Apply([=](auto i) {
		_colorMathFunc[i] = &GbaPpu::ProcessColorMath<(GbaPpuBlendEffect)(i >> 5), (bool)(i & 0x01), (bool)(i & 0x02), (bool)(i & 0x04), (bool)(i & 0x08), (bool)(i & 0x10)>;
	});
}

GbaPpu::~GbaPpu()
{
	delete[] _outputBuffers[0];
	delete[] _outputBuffers[1];
}

void GbaPpu::ProcessHBlank()
{
	if(_state.Scanline < 160) {
		RenderScanline();
		_console->GetDmaController()->TriggerDma(GbaDmaTrigger::HBlank);
	}

	for(int i = 0; i < 4; i++) {
		if(_state.BgLayers[i].EnableTimer && --_state.BgLayers[i].EnableTimer == 0) {
			//Exact timing hasn't been verified
			//The mGBA Suite test writes on H=1065 and expects the layer to be enabled 3 scanlines later (write on scanline 101, starts rendering on scanline 104, so just over 2 scanlines)
			//Spyro - Season of Ice writes on H=30, and expects the layer to be enabled 2 scanlines later
			//Doing this at the start of hblank allows both scenarios the work
			_state.BgLayers[i].Enabled = true;
		}
	}

	if(_state.HblankIrqEnabled) {
		_console->GetMemoryManager()->SetDelayedIrqSource(GbaIrqSource::LcdHblank, 4);
	}
}

void GbaPpu::ProcessEndOfScanline()
{
	ProcessSprites();
	ProcessWindow();

	if(!_skipRender && _emu->IsDebugging()) {
		DebugProcessMemoryAccessView();
	}

	memset(_memoryAccess, GbaPpuMemAccess::None, sizeof(_memoryAccess));

	_state.Cycle = 0;
	_state.Scanline++;

	//Reset renderer data for next scanline
	_lastRenderCycle = -1;
	_lastWindowCycle = -1;
	_oamLastCycle = -1;
	std::fill(_layerOutput[0], _layerOutput[0] + 240, GbaPixelData {});
	std::fill(_layerOutput[1], _layerOutput[1] + 240, GbaPixelData {});
	std::fill(_layerOutput[2], _layerOutput[2] + 240, GbaPixelData {});
	std::fill(_layerOutput[3], _layerOutput[3] + 240, GbaPixelData {});

	for(int i = 0; i < 2; i++) {
		if(_state.Transform[i].PendingUpdateX) {
			_state.Transform[i].LatchOriginX = (_state.Transform[i].OriginX << 4) >> 4; //sign extend
			_state.Transform[i].PendingUpdateX = false;
		}
		if(_state.Transform[i].PendingUpdateY) {
			_state.Transform[i].LatchOriginY = (_state.Transform[i].OriginY << 4) >> 4; //sign extend
			_state.Transform[i].PendingUpdateY = false;
		}
	}

	for(int i = 0; i < 4; i++) {
		//Unverified: Latch X scroll value at the start of each scanline
		//This fixes display issues in the Fire Emblem Sacred Stones menu
		_state.BgLayers[i].ScrollXLatch = _state.BgLayers[i].ScrollX;
	}

	if(_state.Scanline >= 2 && _state.Scanline < 162 && _triggerSpecialDma) {
		//"Video Capture Mode" dma, channel 3 only - auto-stops on scanline 161
		_console->GetDmaController()->TriggerDmaChannel(GbaDmaTrigger::Special, 3, _state.Scanline == 161);
	} else if(_state.Scanline == 162) {
		_triggerSpecialDma = _console->GetDmaController()->IsVideoCaptureDmaEnabled();
	}

	if(_state.Scanline == 160) {
		_oamScanline = 0;
		_state.ObjEnableTimer = 0;
		SendFrame();
		if(_state.VblankIrqEnabled) {
			_console->GetMemoryManager()->SetIrqSource(GbaIrqSource::LcdVblank);
		}
		_console->GetDmaController()->TriggerDma(GbaDmaTrigger::VBlank);
	} else if(_state.Scanline == 228) {
		_state.Scanline = 0;

		//Transform values latched at the start of the frame
		_state.Transform[0].LatchOriginX = (int32_t)_state.Transform[0].OriginX;
		_state.Transform[0].LatchOriginY = (int32_t)_state.Transform[0].OriginY;
		_state.Transform[1].LatchOriginX = (int32_t)_state.Transform[1].OriginX;
		_state.Transform[1].LatchOriginY = (int32_t)_state.Transform[1].OriginY;

		if(_emu->GetSettings()->GetGbaConfig().DisableSprites) {
			std::fill(_oamOutputBuffers[0], _oamOutputBuffers[0] + 240, GbaPixelData {});
			std::fill(_oamOutputBuffers[1], _oamOutputBuffers[1] + 240, GbaPixelData {});
		}

		_emu->ProcessEvent(EventType::StartFrame, CpuType::Gba);

		EmuSettings* settings = _emu->GetSettings();
		_skipRender = (
			!settings->GetGbaConfig().DisableFrameSkipping &&
			!_emu->GetRewindManager()->IsRewinding() &&
			!_emu->GetVideoRenderer()->IsRecording() &&
			(settings->GetEmulationSpeed() == 0 || settings->GetEmulationSpeed() > 150) &&
			_frameSkipTimer.GetElapsedMS() < 15
		);
		if(!_skipRender) {
			_currentBuffer = _currentBuffer == _outputBuffers[0] ? _outputBuffers[1] : _outputBuffers[0];
		}
	}

	if(_state.ScanlineIrqEnabled && _state.Scanline == _state.Lyc) {
		_console->GetMemoryManager()->SetIrqSource(GbaIrqSource::LcdScanlineMatch);
	}

	InitializeWindows();
}

void GbaPpu::SendFrame()
{
	_emu->ProcessEvent(EventType::EndFrame, CpuType::Gba);
	_emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::PpuFrameDone);

	RenderedFrame frame(_currentBuffer, GbaConstants::ScreenWidth, GbaConstants::ScreenHeight, 1.0, _state.FrameCount, _console->GetControlManager()->GetPortStates());
	bool rewinding = _emu->GetRewindManager()->IsRewinding();
	_emu->GetVideoDecoder()->UpdateFrame(frame, rewinding, rewinding);

	_emu->ProcessEndOfFrame();
	_console->ProcessEndOfFrame();

	_state.FrameCount++;

	if(!_skipRender) {
		_frameSkipTimer.Reset();
	}
}

void GbaPpu::DebugSendFrame()
{
	RenderScanline(true);

	int lastDrawnPixel = std::clamp((_state.Cycle - 46) / 4, 0, 239);

	int offset = lastDrawnPixel + 1 + _state.Scanline * GbaConstants::ScreenWidth;
	int pixelsToClear = GbaConstants::ScreenWidth * GbaConstants::ScreenHeight - offset;
	if(pixelsToClear > 0) {
		memset(_currentBuffer + offset, 0, pixelsToClear * sizeof(uint16_t));
	}

	RenderedFrame frame(_currentBuffer, GbaConstants::ScreenWidth, GbaConstants::ScreenHeight, 1.0, _state.FrameCount);
	_emu->GetVideoDecoder()->UpdateFrame(frame, false, false);
}

void GbaPpu::RenderScanline(bool forceRender)
{
	if(_skipRender && !forceRender) {
		return;
	}

	ProcessSprites();
	ProcessWindow();

	if(_state.Scanline >= 160 || _lastRenderCycle >= 1006) {
		return;
	}

	if(_state.ForcedBlank) {
		uint16_t* rowStart = _currentBuffer + (_state.Scanline * GbaConstants::ScreenWidth);
		std::fill(rowStart, rowStart + GbaConstants::ScreenWidth, 0x7FFF);
		return;
	}
	
	uint8_t activeLayers = 0;
	switch(_state.BgMode) {
		case 0:
			RenderTilemap<0>();
			RenderTilemap<1>();
			RenderTilemap<2>();
			RenderTilemap<3>();
			activeLayers = _state.Control2 & 0x0F;
			break;

		case 1:
			RenderTilemap<0>();
			RenderTilemap<1>();
			RenderTransformTilemap<2>();
			activeLayers = _state.Control2 & 0x07;
			break;

		case 2:
			RenderTransformTilemap<2>();
			RenderTransformTilemap<3>();
			activeLayers = _state.Control2 & 0x0C;
			break;

		case 3: RenderBitmapMode<3>(); activeLayers = _state.Control2 & 0x04; break;
		case 4: RenderBitmapMode<4>(); activeLayers = _state.Control2 & 0x04; break;
		case 5: RenderBitmapMode<5>(); activeLayers = _state.Control2 & 0x04; break;

		default: break;
	}

	if(_state.Cycle >= 46) {
		bool windowEnabled = _state.Window0Enabled || _state.Window1Enabled || _state.ObjWindowEnabled;
		(this->*_colorMathFunc[((int)_state.BlendEffect << 5) | ((int)windowEnabled << 4) | activeLayers])();
	}

	_lastRenderCycle = _state.Cycle;
}

template<GbaPpuBlendEffect effect, bool bg0Enabled, bool bg1Enabled, bool bg2Enabled, bool bg3Enabled, bool windowEnabled>
void GbaPpu::ProcessColorMath()
{
	uint16_t* dst = _skipRender ? _skippedOutput : (_currentBuffer + (_state.Scanline * GbaConstants::ScreenWidth));
	uint8_t mainCoeff = std::min<uint8_t>(16, _state.BlendMainCoefficient);
	uint8_t subCoeff = std::min<uint8_t>(16, _state.BlendSubCoefficient);
	
	GbaPixelData main = {};
	GbaPixelData sub = {};
	uint8_t brightness = std::min<uint8_t>(16, _state.Brightness);
	uint16_t blendColor = effect == GbaPpuBlendEffect::IncreaseBrightness ? GbaPpu::WhiteColor : GbaPpu::BlackColor;

	uint8_t wnd = 0;

	int start = _lastRenderCycle < 46 ? 0 : std::max(0, ((_lastRenderCycle - 46) / 4) + 1);
	int end = std::min((_state.Cycle - 46) / 4, 239);

	GbaPixelData sprPixel = {};

	for(int x = start; x <= end; x++) {

		if constexpr(windowEnabled) {
			wnd = _activeWindow[x];
			if(_state.WindowActiveLayers[wnd][GbaPpu::SpriteLayerIndex]) {
				main = _oamReadOutput[x];
			} else {
				main = {};
			}
		} else {
			wnd = GbaPpu::NoWindow;
			main = _oamReadOutput[x];
		}
		
		if(!(main.Color & GbaPpu::SpriteMosaicFlag) || !(sprPixel.Color & GbaPpu::SpriteMosaicFlag) || x % (_state.ObjMosaicSizeX + 1) == 0) {
			sprPixel = main;
		} else {
			main = sprPixel;
		}

		sub = {};

		if constexpr(bg0Enabled) {
			ProcessLayerPixel<0, windowEnabled>(x, wnd, main, sub);
		}
		if constexpr(bg1Enabled) {
			ProcessLayerPixel<1, windowEnabled>(x, wnd, main, sub);
		}
		if constexpr(bg2Enabled) {
			ProcessLayerPixel<2, windowEnabled>(x, wnd, main, sub);
		}
		if constexpr(bg3Enabled) {
			ProcessLayerPixel<3, windowEnabled>(x, wnd, main, sub);
		}

		if((main.Color & (GbaPpu::SpriteBlendFlag | GbaPpu::DirectColorFlag)) == GbaPpu::SpriteBlendFlag && _state.BlendSub[sub.Layer]) {
			//Sprite transparency is applied before anything else
			BlendColors(dst, x, ReadColor<false>(x, main.Color), mainCoeff, ReadColor<true>(x, sub.Color), subCoeff);
		} else {
			if constexpr(effect == GbaPpuBlendEffect::None) {
				dst[x] = ReadColor<false>(x, main.Color);
			} else if constexpr(effect == GbaPpuBlendEffect::AlphaBlend) {
				if(_state.BlendSub[sub.Layer]) {
					if(!_state.BlendMain[main.Layer] || !_state.WindowActiveLayers[wnd][GbaPpu::EffectLayerIndex]) {
						dst[x] = ReadColor<false>(x, main.Color);
					} else {
						BlendColors(dst, x, ReadColor<false>(x, main.Color), mainCoeff, ReadColor<true>(x, sub.Color), subCoeff);
					}
				} else {
					dst[x] = ReadColor<false>(x, main.Color);
				}
			} else {
				if(brightness == 0 || !_state.BlendMain[main.Layer] || !_state.WindowActiveLayers[wnd][GbaPpu::EffectLayerIndex]) {
					dst[x] = ReadColor<false>(x, main.Color);
				} else {
					BlendColors(dst, x, ReadColor<false>(x, main.Color), 16 - brightness, blendColor, brightness);
				}
			}
		}
	}

	if(_state.GreenSwapEnabled) {
		for(int x = start & ~1; x + 1 <= end; x+=2) {
			uint16_t gLeft = dst[x] & 0x3E0;
			uint16_t gRight = dst[x+1] & 0x3E0;
			dst[x] = (dst[x] & ~0x3E0) | gRight;
			dst[x+1] = (dst[x+1] & ~0x3E0) | gLeft;
		}
	}
}

template<bool isSubColor>
uint16_t GbaPpu::ReadColor(int x, uint16_t addr)
{
	if(addr & GbaPpu::DirectColorFlag) {
		return addr & 0x7FFF;
	} else {
		_memoryAccess[(x << 2) + (isSubColor ? 48 : 46)] |= GbaPpuMemAccess::Palette;
		return _paletteRam[(addr >> 1) & 0x1FF] & 0x7FFF;
	}
}

void GbaPpu::BlendColors(uint16_t* dst, int x, uint16_t main, uint8_t aCoeff, uint16_t sub, uint8_t bCoeff)
{
	uint8_t aR = main & 0x1F;
	uint8_t aG = (main >> 5) & 0x1F;
	uint8_t aB = (main >> 10) & 0x1F;

	uint8_t bR = sub & 0x1F;
	uint8_t bG = (sub >> 5) & 0x1F;
	uint8_t bB = (sub >> 10) & 0x1F;

	uint32_t r = std::min(31, (aR * aCoeff + bR * bCoeff) >> 4);
	uint32_t g = std::min(31, (aG * aCoeff + bG * bCoeff) >> 4);
	uint32_t b = std::min(31, (aB * aCoeff + bB * bCoeff) >> 4);

	dst[x] = r | (g << 5) | (b << 10);
}

void GbaPpu::InitializeWindows()
{
	//Windows are enabled/disabled when the scanline reaches the start/end scanlines
	//See window_midframe test - unsure about behavior when top==bottom
	if(_state.Scanline == _state.Window[0].TopY) {
		_window0ActiveY = _state.Window0Enabled;
	}
	if(_state.Scanline == _state.Window[0].BottomY) {
		_window0ActiveY = false;
	}

	if(_state.Scanline == _state.Window[1].TopY) {
		_window1ActiveY = _state.Window1Enabled;
	}
	if(_state.Scanline == _state.Window[1].BottomY) {
		_window1ActiveY = false;
	}

	memset(_activeWindow, GbaPpu::OutsideWindow, sizeof(_activeWindow));
}

void GbaPpu::ProcessWindow()
{
	//Using the same logic as for window Y above, enable/disable windows when
	//the current pixel matches the left/right X value for the window
	//Some games set left > right, essentially making the window
	//wrap around to the beginning of the next scanline.
	//(MM&B does this for speech bubbles)
	//TODOGBA is there any test rom for this (window x)?
	int x = (_lastWindowCycle + 1) / 4;
	int end = std::min(_state.Cycle / 4, 255) + 1;
	if(x >= end) {
		return;
	}
	
	if(_state.Window0Enabled || _state.Window1Enabled) {
		for(int i = -1; i < 4; i++) {
			uint16_t changePos = i < 0 ? 0 : _windowChangePos[i];
			uint16_t nextChange = i < 3 ? _windowChangePos[i + 1] : 256;
			if(nextChange == changePos || (changePos < x && nextChange < x)) {
				continue;
			}

			if(x == _state.Window[0].LeftX) {
				_window0ActiveX = _state.Window0Enabled;
			}
			if(x == _state.Window[0].RightX) {
				_window0ActiveX = false;
			}

			if(x == _state.Window[1].LeftX) {
				_window1ActiveX = _state.Window1Enabled;
			}
			if(x == _state.Window[1].RightX) {
				_window1ActiveX = false;
			}

			int length = std::min(nextChange - x, end - x);
			if(_window0ActiveX && _window0ActiveY) {
				memset(_activeWindow + x, GbaPpu::Window0, length);
			} else if(_window1ActiveX && _window1ActiveY) {
				memset(_activeWindow + x, GbaPpu::Window1, length);
			}

			x += length;
			if(x >= end) {
				break;
			}
		}
	} else {
		_window0ActiveX = false;
		_window1ActiveX = false;
	}

	_lastWindowCycle = _state.Cycle;
}

void GbaPpu::SetWindowX(uint8_t& regValue, uint8_t newValue)
{
	if(regValue != newValue) {
		regValue = newValue;
		UpdateWindowChangePoints();
	}
}

void GbaPpu::UpdateWindowChangePoints()
{
	//Generate a sorted list of the positions where the windows start/end
	//Used in ProcessWindow to process the changes in chunks
	_windowChangePos[0] = _state.Window[0].LeftX;
	_windowChangePos[1] = _state.Window[0].RightX;
	_windowChangePos[2] = _state.Window[1].LeftX;
	_windowChangePos[3] = _state.Window[1].RightX;
	std::sort(_windowChangePos, _windowChangePos + 4);
}

void GbaPpu::SetPixelData(GbaPixelData& pixel, uint16_t color, uint8_t priority, uint8_t layer)
{
	pixel.Color = color;
	pixel.Priority = priority;
	pixel.Layer = layer;
}

template<int i, bool mosaic, bool bpp8>
void GbaPpu::PushBgPixels()
{
	if(_layerData[i].HoriMirror) {
		PushBgPixels<i, mosaic, bpp8, true>();
	} else {
		PushBgPixels<i, mosaic, bpp8, false>();
	}
}

template<int i, bool mosaic, bool bpp8, bool mirror>
void GbaPpu::PushBgPixels()
{
	GbaLayerRendererData& data = _layerData[i];

	uint16_t tileData = _vram16[data.FetchAddr >> 1];
	if constexpr(mirror) {
		if constexpr(bpp8) {
			tileData = ((tileData & 0xFF00) >> 8) | ((tileData & 0x00FF) << 8);
		} else {
			tileData = (
				((tileData & 0xF000) >> 12) | 
				((tileData & 0x0F00) >> 4) |
				((tileData & 0x00F0) << 4) |
				((tileData & 0x000F) << 12)
			);
		}
	}

	int16_t renderX = data.RenderX;
	constexpr int len = bpp8 ? 2 : 4;
	for(int x = 0; x < len; x++) {
		if(renderX >= 0 && renderX < 240) {
			if constexpr(mosaic) {
				if(renderX % (_state.BgMosaicSizeX + 1) == 0) {
					data.MosaicColor = tileData & (bpp8 ? 0xFF : 0x0F);
				}
			} else {
				data.MosaicColor = tileData & (bpp8 ? 0xFF : 0x0F);
			}

			if(data.MosaicColor != 0) {
				SetPixelData(_layerOutput[i][renderX], (data.PaletteIndex * 32) + data.MosaicColor * 2, _state.BgLayers[i].Priority, i);
			}
		}
		tileData >>= bpp8 ? 8 : 4;
		renderX++;
	}

	data.FetchAddr += (mirror ? -2 : 2);
	data.RenderX = renderX;
};

template<int i>
void GbaPpu::RenderTilemap()
{
	if(!_state.BgLayers[i].Enabled || _emu->GetSettings()->GetGbaConfig().HideBgLayers[i]) {
		return;
	}

	if(_state.BgLayers[i].Mosaic) {
		if(_state.BgLayers[i].Bpp8Mode) {
			RenderTilemap<i, true, true>();
		} else {
			RenderTilemap<i, true, false>();
		}
	} else {
		if(_state.BgLayers[i].Bpp8Mode) {
			RenderTilemap<i, false, true>();
		} else {
			RenderTilemap<i, false, false>();
		}
	}
}

template<int i, bool mosaic, bool bpp8>
void GbaPpu::RenderTilemap()
{
	GbaBgConfig& layer = _state.BgLayers[i];

	uint16_t baseAddr = layer.TilemapAddr >> 1;
	uint16_t scanline = _state.Scanline;
	if constexpr(mosaic) {
		scanline = scanline - scanline % (_state.BgMosaicSizeY + 1);
	}
	uint16_t yPos = layer.ScrollY + scanline;

	if(layer.DoubleHeight && (yPos & 0x100)) {
		baseAddr += layer.DoubleWidth ? 0x800 : 0x400;
	}

	yPos &= 0xFF;

	if(_lastRenderCycle == -1) {
		_layerData[i].RenderX = -(layer.ScrollXLatch & 0x07);
	}

	//MessageManager::Log(std::to_string(_state.Scanline) + " render " + std::to_string(_lastRenderCycle+1) + " to " + std::to_string(_state.Cycle));
	int gap = (31 + i - (layer.ScrollXLatch & 0x07) * 4);
	int cycle = std::max(0, _lastRenderCycle + 1 - gap);
	int end = std::min<int>(_state.Cycle, 1005) - gap;

	for(; cycle <= end; cycle++) {
		//MessageManager::Log(std::to_string(_state.Scanline) + " fetch cycle " + std::to_string(fetchCycle));
		switch(cycle & 0x1F) {
			case 0: {
				//Fetch tilemap data
				uint16_t xPos = layer.ScrollXLatch + _layerData[i].RenderX;
				uint16_t addr = baseAddr;
				if(layer.DoubleWidth && (xPos & 0x100)) {
					addr += 0x400;
				}
				xPos &= 0xFF;
				
				uint16_t vramAddr = addr + yPos / 8 * 32 + xPos / 8;
				uint16_t tilemapData = _vram16[vramAddr];
				_memoryAccess[cycle + gap] |= GbaPpuMemAccess::Vram;

				_layerData[i].TileIndex = tilemapData & 0x3FF;
				_layerData[i].HoriMirror = tilemapData & (1 << 10);
				_layerData[i].VertMirror = tilemapData & (1 << 11);
				_layerData[i].PaletteIndex = bpp8 ? 0 : (tilemapData >> 12) & 0x0F;

				_layerData[i].TileRow = _layerData[i].VertMirror ? (~yPos & 0x07) : (yPos & 0x07);
				_layerData[i].TileColumn = _layerData[i].HoriMirror ? (bpp8 ? 6 : 2) : 0;
				_layerData[i].FetchAddr = layer.TilesetAddr + _layerData[i].TileIndex * (bpp8 ? 64 : 32) + _layerData[i].TileRow * (bpp8 ? 8 : 4) + _layerData[i].TileColumn;

				//MessageManager::Log(std::to_string(_state.Scanline) + "," + std::to_string(cycle) + " fetch tilemap");
				cycle += 3;
				break;
			}

			case 4: {
				//Fetch tile data (4bpp & 8bpp)
				PushBgPixels<i, mosaic, bpp8>();
				_memoryAccess[cycle + gap] |= GbaPpuMemAccess::Vram;

				//MessageManager::Log(std::to_string(_state.Scanline) + "," + std::to_string(cycle) + " fetch tile data 0");
				cycle += bpp8 ? 7 : 15;
				break;
			}

			case 12: {
				//Fetch tile data (8bpp only)
				if constexpr(bpp8) {
					PushBgPixels<i, mosaic, bpp8>();
					_memoryAccess[cycle + gap] |= GbaPpuMemAccess::Vram;
				}
				cycle += 7;
				break;
			}

			case 20: {
				//Fetch tile data (4bpp & 8bpp)
				PushBgPixels<i, mosaic, bpp8>();
				_memoryAccess[cycle + gap] |= GbaPpuMemAccess::Vram;

				//MessageManager::Log(std::to_string(_state.Scanline) + "," + std::to_string(cycle) + " fetch tile data 2");
				cycle += bpp8 ? 7 : (11 - i);
				break;
			}
						
			case 28: {
				//Fetch tile data (8bpp only)
				if constexpr(bpp8) {
					PushBgPixels<i, mosaic, bpp8>();
					_memoryAccess[cycle + gap] |= GbaPpuMemAccess::Vram;
				}
				cycle += 3 - i;
				break;
			}
		}
	}
}

template<int i>
void GbaPpu::RenderTransformTilemap()
{
	GbaBgConfig& layer = _state.BgLayers[i];
	GbaTransformConfig& cfg = _state.Transform[i - 2];
	if(!layer.Enabled || _emu->GetSettings()->GetGbaConfig().HideBgLayers[i]) {
		return;
	}

	uint16_t screenSize = 128 << layer.ScreenSize;

	if(_lastRenderCycle == -1) {
		_layerData[i].TransformX = (cfg.LatchOriginX << 4) >> 4; //sign extend
		_layerData[i].TransformY = (cfg.LatchOriginY << 4) >> 4;
		_layerData[i].RenderX = 0;
	}
	
	//MessageManager::Log(std::to_string(_state.Scanline) + " render " + std::to_string(_lastRenderCycle+1) + " to " + std::to_string(_state.Cycle));
	constexpr int gap = (37 - i * 2);
	int cycle = std::max(0, _lastRenderCycle + 1 - gap);
	int end = std::min<int>(_state.Cycle, 1005) - gap;

	for(; cycle <= end; cycle++) {
		switch(cycle & 0x03) {
			case 0: {
				//Fetch tilemap data
				uint32_t wrapMask = layer.WrapAround ? (screenSize - 1) : 0xFFFFF;
				uint16_t columnCount = screenSize >> 3;

				if(!layer.Mosaic || _layerData[i].RenderX % (_state.BgMosaicSizeX + 1) == 0) {
					//Ignore decimal point value (bottom 8 bits), apply wraparound behavior
					//This produces the x,y coordinate (on the tilemap) that needs to be drawn
					_layerData[i].XPos = (_layerData[i].TransformX >> 8) & wrapMask;
					_layerData[i].YPos = (_layerData[i].TransformY >> 8) & wrapMask;
				}

				uint16_t vramAddr = layer.TilemapAddr + (_layerData[i].YPos >> 3) * columnCount + (_layerData[i].XPos >> 3);
				_layerData[i].TileIndex = _vram[vramAddr];
				_memoryAccess[cycle + gap] |= GbaPpuMemAccess::Vram;

				_layerData[i].TileRow = _layerData[i].YPos & 0x07;
				_layerData[i].TileColumn = _layerData[i].XPos & 0x07;

				//Update x/y values for next dot
				_layerData[i].TransformX += cfg.Matrix[0];
				_layerData[i].TransformY += cfg.Matrix[2];
				//MessageManager::Log(std::to_string(_state.Scanline) + "," + std::to_string(cycle) + " fetch tilemap");
				break;
			}

			case 1: {
				//Fetch pixel
				_memoryAccess[cycle + gap] |= GbaPpuMemAccess::Vram;
				if(_layerData[i].RenderX < 240) {
					uint8_t color = _vram[(layer.TilesetAddr + _layerData[i].TileIndex * 64 + _layerData[i].TileRow * 8 + _layerData[i].TileColumn) & 0xFFFF];
					if(color != 0 && _layerData[i].XPos < screenSize && _layerData[i].YPos < screenSize) {
						SetPixelData(_layerOutput[i][_layerData[i].RenderX], color * 2, layer.Priority, i);
					}
				}
				_layerData[i].RenderX++;
				//MessageManager::Log(std::to_string(_state.Scanline) + "," + std::to_string(cycle) + " fetch pixel");
				cycle += 2;
				break;
			}
		}
	}

	if(_state.Cycle == 1006) {
		//Update x/y values for next scanline
		if(!layer.Mosaic) {
			cfg.LatchOriginX += cfg.Matrix[1];
			cfg.LatchOriginY += cfg.Matrix[3];
		} else if(_state.Scanline % (_state.BgMosaicSizeY + 1) == _state.BgMosaicSizeY) {
			cfg.LatchOriginX += cfg.Matrix[1] * (_state.BgMosaicSizeY + 1);
			cfg.LatchOriginY += cfg.Matrix[3] * (_state.BgMosaicSizeY + 1);
		}
	}
}

template<int mode>
void GbaPpu::RenderBitmapMode()
{
	GbaBgConfig& layer = _state.BgLayers[2];
	if(!layer.Enabled || _emu->GetSettings()->GetGbaConfig().HideBgLayers[2]) {
		return;
	}

	GbaTransformConfig& cfg = _state.Transform[0];

	if(_lastRenderCycle == -1) {
		_layerData[2].TransformX = (cfg.LatchOriginX << 4) >> 4; //sign extend
		_layerData[2].TransformY = (cfg.LatchOriginY << 4) >> 4;
		_layerData[2].RenderX = 0;
	}

	uint16_t screenWidth = mode == 5 ? 160 : 240;
	uint16_t screenHeight = mode == 5 ? 128 : 160;

	uint32_t base = (_state.DisplayFrameSelect && (mode == 4 || mode == 5)) ? 0xA000 : 0;

	constexpr int gap = 34;
	int cycle = std::max(0, _lastRenderCycle + 1 - gap);
	int end = std::min<int>(_state.Cycle, 1005) - gap;

	for(; cycle <= end; cycle++) {
		if(!(cycle & 0x03)) {
			if(!layer.Mosaic || _layerData[2].RenderX % (_state.BgMosaicSizeX + 1) == 0) {
				//Ignore decimal point value (bottom 8 bits), apply wraparound behavior
				//This produces the x,y coordinate (on the tilemap) that needs to be drawn
				_layerData[2].XPos = (_layerData[2].TransformX >> 8);
				_layerData[2].YPos = (_layerData[2].TransformY >> 8);
			}

			_memoryAccess[cycle+gap] |= GbaPpuMemAccess::Vram;

			if(_layerData[2].YPos < screenHeight && _layerData[2].XPos < screenWidth) {
				uint32_t addr = _layerData[2].YPos * screenWidth + _layerData[2].XPos;
				if constexpr(mode == 3 || mode == 5) {
					SetPixelData(_layerOutput[2][_layerData[2].RenderX], _vram16[(base >> 1) + addr] | GbaPpu::DirectColorFlag, layer.Priority, 2);
				} else if constexpr(mode == 4) {
					uint8_t color = _vram[base + addr];
					if(color != 0) {
						SetPixelData(_layerOutput[2][_layerData[2].RenderX], color * 2, layer.Priority, 2);
					}
				}
			}

			//Update x/y values for next dot
			_layerData[2].TransformX += cfg.Matrix[0];
			_layerData[2].TransformY += cfg.Matrix[2];
			_layerData[2].RenderX++;
			cycle += 2;
		}
	}

	if(_state.Cycle == 1006) {
		//Update x/y values for next scanline
		if(!layer.Mosaic) {
			cfg.LatchOriginX += cfg.Matrix[1];
			cfg.LatchOriginY += cfg.Matrix[3];
		} else if(_state.Scanline % (_state.BgMosaicSizeY + 1) == _state.BgMosaicSizeY) {
			cfg.LatchOriginX += cfg.Matrix[1] * (_state.BgMosaicSizeY + 1);
			cfg.LatchOriginY += cfg.Matrix[3] * (_state.BgMosaicSizeY + 1);
		}
	}
}

//TODOGBA behavior for shape == 3? current behaves like shape == 0
static constexpr uint8_t _sprSize[4][4][2] = {
	{ { 8, 8 }, { 16, 8 }, { 8, 16 }, { 8, 8 } },
	{ { 16, 16 }, { 32, 8 }, { 8, 32 }, { 16, 16 } },
	{ { 32, 32 }, { 32, 16 }, { 16, 32 }, { 32, 32 } },
	{ { 64, 64 }, { 64, 32 }, { 32, 64 }, { 64, 64 } }
};

void GbaPpu::ProcessSprites()
{
	if(!_emu->GetSettings()->GetGbaConfig().DisableSprites && _state.ObjLayerEnabled && (_state.Scanline <= 159 || _state.Scanline == 227)) {
		if(_state.BgMode >= 3) {
			RenderSprites<true>();
		} else {
			RenderSprites<false>();
		}
	}
}

void GbaPpu::InitSpriteEvaluation()
{
	_oamScanline = _state.Scanline == 227 ? 0 : (_state.Scanline + 1);
	if(_oamScanline == 0) {
		_oamMosaicY = 0;
	} else {
		if(_oamMosaicY == _state.ObjMosaicSizeY) {
			_oamMosaicY = 0;
		} else {
			_oamMosaicY = (_oamMosaicY + 1) & 0x0F;
		}
	}
	_loadOamAttr01 = true;
	_loadOamAttr2 = false;
	_isFirstOamTileLoad = false;
	_loadOamTileCounter = 0;
	_loadObjMatrix = 0;
	_evalOamIndex = -1;

	//Update window data
	if(_oamHasWindowModeSprite) {
		for(int x = 0; x < 240; x++) {
			if(_oamWindow[x] == GbaPpu::ObjWindow && _activeWindow[x] == GbaPpu::OutsideWindow) {
				_activeWindow[x] = _oamWindow[x];
			}
		}
	}
	
	_oamHasWindowModeSprite = false;

	std::swap(_oamWriteOutput, _oamReadOutput);
	std::fill(_oamWriteOutput, _oamWriteOutput + 240, GbaPixelData {});
	memset(_oamWindow, GbaPpu::BackdropLayerIndex, sizeof(_oamWindow));

	if(_state.ObjEnableTimer) {
		_state.ObjEnableTimer--;
	}
}

void GbaPpu::AddVisibleSprite(uint32_t sprData)
{
	GbaSpriteRendererData& spr = _objData[0];

	spr.Width = _sprSize[spr.Size][spr.Shape][0];
	spr.Mosaic = sprData & 0x1000;
	if(spr.Mosaic) {
		spr.YOffset = std::max(0, (int)spr.YOffset - (int)_oamMosaicY);
	}

	spr.Bpp8Mode = sprData & 0x2000;
	spr.SpriteX = (sprData >> 16) & 0x1FF;
	spr.HoriMirror = sprData & 0x10000000;
	spr.VertMirror = sprData & 0x20000000;
	spr.TransformParamSelect = (sprData >> 25) & 0x1F;
	spr.RenderX = 0;

	_loadOamAttr01 = false;
	_loadOamAttr2 = true;

	if(spr.SpriteX >= 240) {
		//Skip hidden pixels that wrap around to the right side of the coordinate space
		int gap = -((int)spr.SpriteX - 512);
		spr.RenderX += spr.TransformEnabled ? gap : (gap & ~0x01);
		if(spr.RenderX >= (spr.Width << (uint8_t)spr.DoubleSize)) {
			//Skip this sprite entirely (it's completely hidden)
			_loadOamAttr01 = true;
			_loadOamAttr2 = false;
		}
	}
}

void GbaPpu::LoadSpriteAttr2()
{
	GbaSpriteRendererData& spr = _objData[0];
	uint32_t sprData = _oam[spr.Addr + 1];
	spr.TileIndex = sprData & 0x3FF;
	spr.Priority = (sprData >> 10) & 0x03;
	spr.PaletteIndex = spr.Bpp8Mode ? 0 : ((sprData >> 12) & 0x0F);

	_loadOamAttr2 = false;
	if(spr.TransformEnabled) {
		_loadObjMatrix = 4;
	} else {
		_loadOamAttr01 = true;
		_isFirstOamTileLoad = true;
		_loadOamTileCounter = (spr.Width - spr.RenderX) / 2;
		_objData[1] = spr;
	}
}

void GbaPpu::LoadSpriteTransformMatrix()
{
	GbaSpriteRendererData& spr = _objData[0];
	_loadObjMatrix--;

	uint8_t i = 3 - _loadObjMatrix;
	uint16_t transformAddr = (spr.TransformParamSelect << 3) + 1 + (i * 2);
	spr.MatrixData[i] = _oam[transformAddr] >> 16;

	if(_loadObjMatrix == 0) {
		spr.CenterX = spr.Width / 2;
		spr.CenterY = spr.Height / 2;

		int32_t originX = -(spr.CenterX << (uint8_t)spr.DoubleSize);
		int32_t originY = -(spr.CenterY << (uint8_t)spr.DoubleSize);

		spr.XValue = originX * spr.MatrixData[0] + (originY + spr.YOffset) * spr.MatrixData[1];
		spr.YValue = originX * spr.MatrixData[2] + (originY + spr.YOffset) * spr.MatrixData[3];
		spr.XValue += spr.MatrixData[0] * spr.RenderX;
		spr.YValue += spr.MatrixData[2] * spr.RenderX;
		_loadOamTileCounter = (spr.Width << (uint8_t)spr.DoubleSize) - spr.RenderX;
		_loadOamAttr01 = true;
		_isFirstOamTileLoad = true;
		_objData[1] = spr;
	}
}

template<bool blockFirst16k>
void GbaPpu::RenderSprites()
{
	if(_oamScanline >= 160) {
		return;
	}

	uint16_t ppuCycle = _state.Cycle >= 308 * 4 ? (308 * 4) - 1 : _state.Cycle;
	if(_state.AllowHblankOamAccess && ppuCycle > 996) {
		//Evaluation stops just before hblank when this is enabled
		ppuCycle = 996;
	}
	int cycle = _oamLastCycle + 1;

	if(cycle < 40 && (_state.AllowHblankOamAccess || _evalOamIndex >= 128 || _state.ObjEnableTimer > 0)) {
		//Evaluation in hblank is disabled, jump to cycle 40 (eval start)
		cycle = 40;
	} else if(cycle >= 40 && _state.ObjEnableTimer > 0) {
		return;
	}

	if(cycle & 0x01) {
		cycle++;
	}
	GbaSpriteRendererData& spr = _objData[0];
	for(; cycle <= ppuCycle; cycle+=2) {
		if(cycle == 40) {
			//start oam evaluation/fetching
			InitSpriteEvaluation();
			if(_oamScanline == 160 || _state.ObjEnableTimer > 0) {
				//Scanline 159 stops processing sprites after cycle 39
				break;
			}
		}

		bool allowLoadAttr01 = _loadOamTileCounter <= 1 || _isFirstOamTileLoad;

		if(_loadOamTileCounter) {
			if(_isFirstOamTileLoad && _objData[1].TransformEnabled) {
				_isFirstOamTileLoad = false;
			} else {
				//load+draw pixels
				_isFirstOamTileLoad = false;
				_loadOamTileCounter--;
				_memoryAccess[cycle] |= GbaPpuMemAccess::VramObj;

				//Last cycle (39) doesn't actually draw, but cycle 38 does read from VRAM anyway
				if(cycle != 38) {
					if(_objData[1].TransformEnabled) {
						RenderSprite<true, blockFirst16k>(_objData[1]);
					} else {
						RenderSprite<false, blockFirst16k>(_objData[1]);
					}
				}

				if(_evalOamIndex >= 128 && _loadOamTileCounter == 0) {
					//Finished loading last sprite
					if(cycle < 40) {
						//Jump to the start of the next evaluation cycle (cycle 40)
						cycle = 38;
						continue;
					} else {
						break;
					}
				}
			}
		}

		if(_loadOamAttr01 && allowLoadAttr01) {
			//Load first 4 bytes of OAM attributes, evaluate if sprite should appear on this scanline
			_evalOamIndex++;
			if(_evalOamIndex >= 128) {
				if(_loadOamTileCounter == 0) {
					//Finished loading last sprite
					if(cycle < 40) {
						//Jump to the start of the next evaluation cycle (cycle 40)
						cycle = 38;
						continue;
					} else {
						break;
					}
				}
				_loadOamAttr01 = false;
				continue;
			}
			
			_memoryAccess[cycle] |= GbaPpuMemAccess::Oam;

			uint16_t addr = _evalOamIndex << 1;
			uint32_t sprData = _oam[addr];
			spr.TransformEnabled = sprData & 0x0100;
			spr.DoubleSize = sprData & 0x0200;
			spr.HideSprite = !spr.TransformEnabled && spr.DoubleSize;
			spr.Mode = (GbaPpuObjMode)((sprData >> 10) & 0x03);
			if(!spr.HideSprite && spr.Mode != GbaPpuObjMode::Invalid) {
				spr.SpriteY = sprData & 0xFF;
				spr.Shape = (sprData >> 14) & 0x03;
				spr.Size = (sprData >> 30) & 0x03;
				spr.Height = _sprSize[spr.Size][spr.Shape][1];
				spr.YOffset = _oamScanline - spr.SpriteY;

				uint8_t sprHeight = (spr.Height << (uint8_t)spr.DoubleSize);
				if(spr.YOffset < sprHeight && _oamScanline < (uint8_t)(spr.SpriteY + sprHeight)) {
					//sprite is visible on this scanline
					spr.Addr = addr;
					AddVisibleSprite(sprData);
				}
			}
		} else if(_loadOamAttr2 && _loadOamTileCounter == 0) {
			//Load last 2 bytes of OAM attributes
			_memoryAccess[cycle] |= GbaPpuMemAccess::Oam;
			LoadSpriteAttr2();
		} else if(_loadObjMatrix) {
			//Load all 4 16-bit transform parameters (when transform flag is enabled)
			_memoryAccess[cycle] |= GbaPpuMemAccess::Oam;
			LoadSpriteTransformMatrix();
		}
	}

	_oamLastCycle = _state.Cycle;
}

template<bool blockFirst16k>
uint8_t GbaPpu::ReadSpriteVram(uint32_t addr)
{
	if constexpr(blockFirst16k) {
		//When in BG modes 3-5, the first 16kb of sprite tile vram ($10000-$13FFF) can't be accessed
		//by sprites (because the BG layer uses it), reading it returns 0
		return addr < 0x4000 ? 0 : _vram[0x10000 | (addr & 0x7FFF)];
	} else {
		return _vram[0x10000 | (addr & 0x7FFF)];
	}
}

template<bool transformEnabled, bool blockFirst16k>
void GbaPpu::RenderSprite(GbaSpriteRendererData& spr)
{
	for(int i = 0; i < (transformEnabled ? 1 : 2); i++) {
		if constexpr(transformEnabled) {
			spr.XPos = (spr.XValue >> 8) + spr.CenterX;
			spr.YPos = (spr.YValue >> 8) + spr.CenterY;
			spr.XValue += spr.MatrixData[0];
			spr.YValue += spr.MatrixData[2];
		} else {
			spr.XPos = spr.HoriMirror ? (spr.Width - spr.RenderX - 1) : spr.RenderX;
			spr.YPos = spr.VertMirror ? (spr.Height - spr.YOffset - 1) : spr.YOffset;
		}

		uint16_t drawPos = (spr.SpriteX + spr.RenderX) & 0x1FF;
		spr.RenderX++;

		if(drawPos >= 240 || spr.XPos >= spr.Width || spr.YPos >= spr.Height) {
			continue;
		}

		bool isHigherPriority = _oamWriteOutput[drawPos].Priority > spr.Priority;
		if(spr.Mode == GbaPpuObjMode::Window || isHigherPriority) {
			uint8_t tileRow = spr.YPos / 8;
			uint8_t tileColumn = spr.XPos / 8;
			uint16_t index;
			if(_state.ObjVramMappingOneDimension) {
				uint8_t tilesPerRow = (spr.Width / 8) * (spr.Bpp8Mode ? 2 : 1);
				index = _objData[0].TileIndex + tileRow * tilesPerRow + (spr.Bpp8Mode ? tileColumn * 2 : tileColumn);
			} else {
				index = (_objData[0].TileIndex & ~0x1F) + tileRow * 0x20 + (((_objData[0].TileIndex & 0x1F) + (spr.Bpp8Mode ? tileColumn * 2 : tileColumn)) & 0x1F);
			}
			uint8_t color = 0;

			if(spr.Bpp8Mode) {
				color = ReadSpriteVram<blockFirst16k>(index * 32 + (spr.YPos & 0x07) * 8 + (spr.XPos & 0x07));
			} else {
				color = ReadSpriteVram<blockFirst16k>(index * 32 + (spr.YPos & 0x07) * 4 + ((spr.XPos & 0x07) >> 1));
				if(spr.XPos & 0x01) {
					color >>= 4;
				} else {
					color &= 0x0F;
				}
			}

			if(color != 0) {
				if(spr.Mode == GbaPpuObjMode::Window) {
					_oamHasWindowModeSprite = true;
					_oamWindow[drawPos] = GbaPpu::ObjWindow;
				} else {
					uint16_t colorIndex = 0x200 + (spr.PaletteIndex * 32) + color * 2;
					if(spr.Mode == GbaPpuObjMode::Blending) {
						colorIndex |= GbaPpu::SpriteBlendFlag;
					}
					SetPixelData(_oamWriteOutput[drawPos], colorIndex, spr.Priority, GbaPpu::SpriteLayerIndex);
				}
			} else if(isHigherPriority && _oamWriteOutput[drawPos].Priority != 0xFF) {
				//If a sprite pixel already exists and another sprite with higher priority with a
				//transparent pixel is displayed here, the priority is updated to match the transparent
				//pixel's priority (Golden Sun requires this)
				_oamWriteOutput[drawPos].Priority = spr.Priority;
			}

			if(spr.Mosaic) {
				_oamWriteOutput[drawPos].Color |= GbaPpu::SpriteMosaicFlag;
			}
		}
	}
}

void GbaPpu::SetWindowActiveLayers(int window, uint8_t cfg)
{
	for(int i = 0; i < 6; i++) {
		_state.WindowActiveLayers[window][i] = cfg & (1 << i);
	}
}

template<int bit>
void GbaPpu::SetTransformOrigin(uint8_t i, uint8_t value, bool setY)
{
	if(setY) {
		BitUtilities::SetBits<bit>(_state.Transform[i].OriginY, value);
		_state.Transform[i].PendingUpdateY = true;
	} else {
		BitUtilities::SetBits<bit>(_state.Transform[i].OriginX, value);
		_state.Transform[i].PendingUpdateX = true;
	}
}

void GbaPpu::SetLayerEnabled(int layer, bool enabled)
{
	if(_state.BgLayers[layer].Enabled || !enabled) {
		_state.BgLayers[layer].Enabled = enabled;
		_state.BgLayers[layer].EnableTimer = 0;
	} else if(_state.BgLayers[layer].EnableTimer == 0) {
		_state.BgLayers[layer].EnableTimer = 2;
	}
}

void GbaPpu::WriteRegister(uint32_t addr, uint8_t value)
{
	if(_lastRenderCycle != _state.Cycle && (_state.Scanline < 160 || _state.Scanline == 227)) {
		if(_state.Cycle < 1006 || addr <= 0x01 || addr == 0x4D) {
			//Only run renderer during active rendering (< 1006), or if the write could affect sprites
			RenderScanline(true);
		}
	}

	switch(addr) {
		case 0x00:
			_state.Control = value & ~0x08;
			_state.BgMode = value & 0x07;
			_state.DisplayFrameSelect = value & 0x10;
			_state.AllowHblankOamAccess = value & 0x20;
			_state.ObjVramMappingOneDimension = value & 0x40;
			_state.ForcedBlank = value & 0x80;
			break;

		case 0x01: {
			_state.Control2 = value;
			SetLayerEnabled(0, value & 0x01);
			SetLayerEnabled(1, value & 0x02);
			SetLayerEnabled(2, value & 0x04);
			SetLayerEnabled(3, value & 0x08);
			bool objEnabled = value & 0x10;
			if(_state.ObjLayerEnabled && !objEnabled) {
				//Clear sprite row buffers when OBJ layer is disabled
				//TODOGBA what does hardware do if sprites are re-enabled on the same or next scanline?
				std::fill(_oamOutputBuffers[0], _oamOutputBuffers[0] + 240, GbaPixelData {});
				std::fill(_oamOutputBuffers[1], _oamOutputBuffers[1] + 240, GbaPixelData {});
			} else if(!_state.ObjLayerEnabled && objEnabled) {
				_state.ObjEnableTimer = _state.Scanline >= 160 ? 0 : 3;
			}
			_state.ObjLayerEnabled = objEnabled;
			_state.Window0Enabled = value & 0x20;
			_state.Window1Enabled = value & 0x40;
			_state.ObjWindowEnabled = value & 0x80;
			break;
		}

		case 0x02: _state.GreenSwapEnabled = value & 0x01; break;
		case 0x03: break;

		case 0x04:
			_state.DispStat = value & 0x38;
			_state.VblankIrqEnabled = value & 0x08;
			_state.HblankIrqEnabled = value & 0x10;
			_state.ScanlineIrqEnabled = value & 0x20;
			break;
		
		case 0x05:
			if(_state.Lyc != value) {
				_state.Lyc = value;

				//If LYC is changed to the current scanline and LYC IRQs are enabled, trigger it
				//(writing when LYC is already set to the current scanline doesn't trigger an irq - doing this breaks audio in Minish Cap)
				if(_state.ScanlineIrqEnabled && _state.Scanline == _state.Lyc) {
					_console->GetMemoryManager()->SetIrqSource(GbaIrqSource::LcdScanlineMatch);
				}
			}
			break;

		case 0x08: case 0x0A: case 0x0C: case 0x0E: {
			GbaBgConfig& cfg = _state.BgLayers[(addr & 0x06) >> 1];
			BitUtilities::SetBits<0>(cfg.Control, value);
			cfg.Priority = value & 0x03;
			cfg.TilesetAddr = ((value >> 2) & 0x03) * 0x4000;
			//Bits 4-5 unused
			cfg.Mosaic = value & 0x40;
			cfg.Bpp8Mode = value & 0x80;
			break;
		}

		case 0x09: case 0x0B: case 0x0D: case 0x0F: {
			uint8_t layer = (addr & 0x06) >> 1;
			GbaBgConfig& cfg = _state.BgLayers[layer];
			if(layer < 2) {
				//unused bit
				value &= ~0x20;
			}

			BitUtilities::SetBits<8>(cfg.Control, value);
			cfg.TilemapAddr = (value & 0x1F) * 0x800;
			cfg.WrapAround = value & 0x20;
			cfg.DoubleWidth = value & 0x40;
			cfg.DoubleHeight = value & 0x80;
			cfg.ScreenSize = (value >> 6) & 0x03;
			break;
		}

		case 0x10: case 0x14: case 0x18: case 0x1C:
			BitUtilities::SetBits<0>(_state.BgLayers[(addr & 0x0C) >> 2].ScrollX, value);
			break;

		case 0x11: case 0x15: case 0x19: case 0x1D:
			BitUtilities::SetBits<8>(_state.BgLayers[(addr & 0x0C) >> 2].ScrollX, value);
			break;

		case 0x12: case 0x16: case 0x1A: case 0x1E:
			BitUtilities::SetBits<0>(_state.BgLayers[(addr & 0x0C) >> 2].ScrollY, value);
			break;

		case 0x13: case 0x17: case 0x1B: case 0x1F:
			BitUtilities::SetBits<8>(_state.BgLayers[(addr & 0x0C) >> 2].ScrollY, value);
			break;

		case 0x20: case 0x22: case 0x24: case 0x26:
		case 0x30: case 0x32: case 0x34: case 0x36:
			BitUtilities::SetBits<0>(_state.Transform[(addr & 0x10) >> 4].Matrix[(addr & 0x06) >> 1], value);
			break;

		case 0x21: case 0x23: case 0x25: case 0x27:
		case 0x31: case 0x33: case 0x35: case 0x37:
			BitUtilities::SetBits<8>(_state.Transform[(addr & 0x10) >> 4].Matrix[(addr & 0x06) >> 1], value);
			break;
		
		case 0x28: case 0x38: SetTransformOrigin<0>((addr & 0x10) >> 4, value, false); break;
		case 0x29: case 0x39: SetTransformOrigin<8>((addr & 0x10) >> 4, value, false);  break;
		case 0x2A: case 0x3A: SetTransformOrigin<16>((addr & 0x10) >> 4, value, false);  break;
		case 0x2B: case 0x3B: SetTransformOrigin<24>((addr & 0x10) >> 4, value & 0x0F, false); break;

		case 0x2C: case 0x3C: SetTransformOrigin<0>((addr & 0x10) >> 4, value, true);  break;
		case 0x2D: case 0x3D: SetTransformOrigin<8>((addr & 0x10) >> 4, value, true);  break;
		case 0x2E: case 0x3E: SetTransformOrigin<16>((addr & 0x10) >> 4, value, true);  break;
		case 0x2F: case 0x3F: SetTransformOrigin<24>((addr & 0x10) >> 4, value & 0x0F, true);  break;

		case 0x40: SetWindowX(_state.Window[0].RightX, value); break;
		case 0x41: SetWindowX(_state.Window[0].LeftX, value); break;
		case 0x42: SetWindowX(_state.Window[1].RightX, value); break;
		case 0x43: SetWindowX(_state.Window[1].LeftX, value); break;
		
		case 0x44: _state.Window[0].BottomY = value; break;
		case 0x45: _state.Window[0].TopY = value; break;
		case 0x46: _state.Window[1].BottomY = value; break;
		case 0x47: _state.Window[1].TopY = value; break;

		case 0x48: _state.Window0Control = value & 0x3F; SetWindowActiveLayers(0, value & 0x3F); break;
		case 0x49: _state.Window1Control = value & 0x3F; SetWindowActiveLayers(1, value & 0x3F); break;
		case 0x4A: _state.OutWindowControl = value & 0x3F; SetWindowActiveLayers(3, value & 0x3F);  break;
		case 0x4B: _state.ObjWindowControl = value & 0x3F; SetWindowActiveLayers(2, value & 0x3F);  break;

		case 0x4C:
			_state.BgMosaicSizeX = value & 0x0F;
			_state.BgMosaicSizeY = (value >> 4) & 0x0F;
			break;

		case 0x4D:
			_state.ObjMosaicSizeX = value & 0x0F;
			_state.ObjMosaicSizeY = (value >> 4) & 0x0F;
			break;

		case 0x50:
			_state.BlendMainControl = value;
			_state.BlendMain[0] = value & 0x01; //BG0
			_state.BlendMain[1] = value & 0x02; //BG1
			_state.BlendMain[2] = value & 0x04; //BG2
			_state.BlendMain[3] = value & 0x08; //BG3
			_state.BlendMain[GbaPpu::SpriteLayerIndex] = value & 0x10;
			_state.BlendMain[GbaPpu::BackdropLayerIndex] = value & 0x20;
			_state.BlendEffect = (GbaPpuBlendEffect)((value >> 6) & 0x03);
			break;

		case 0x51:
			_state.BlendSubControl = value & 0x3F;
			_state.BlendSub[0] = value & 0x01;
			_state.BlendSub[1] = value & 0x02;
			_state.BlendSub[2] = value & 0x04;
			_state.BlendSub[3] = value & 0x08;
			_state.BlendSub[GbaPpu::SpriteLayerIndex] = value & 0x10;
			_state.BlendSub[GbaPpu::BackdropLayerIndex] = value & 0x20;
			break;

		case 0x52: _state.BlendMainCoefficient = value & 0x1F; break;
		case 0x53: _state.BlendSubCoefficient = value & 0x1F; break;
		case 0x54: _state.Brightness = value & 0x1F; break;
		
		default:
			LogDebug("Write unimplemented LCD register: " + HexUtilities::ToHex32(addr) + " = " + HexUtilities::ToHex(value));
			break;
	}
}

uint8_t GbaPpu::ReadRegister(uint32_t addr)
{
	switch(addr) {
		case 0x00: return _state.Control;
		case 0x01: return _state.Control2;

		case 0x02: return (uint8_t)_state.GreenSwapEnabled;
		case 0x03: return 0;

		case 0x04:
			return (
				(_state.Scanline >= 160 && _state.Scanline != 227 ? 0x01 : 0) |
				((_state.Cycle > 1007 || _state.Cycle == 0) ? 0x02 : 0) |
				(_state.Scanline == _state.Lyc ? 0x04 : 0) |
				_state.DispStat
			);

		case 0x05: return _state.Lyc;

		case 0x06: return _state.Scanline;
		case 0x07: return 0;

		case 0x08: case 0x0A: case 0x0C: case 0x0E:
			return (uint8_t)_state.BgLayers[(addr & 0x06) >> 1].Control;

		case 0x09: case 0x0B: case 0x0D: case 0x0F:
			return (uint8_t)(_state.BgLayers[(addr & 0x06) >> 1].Control >> 8);

		case 0x48: return _state.Window0Control;
		case 0x49: return _state.Window1Control;
		case 0x4A: return _state.OutWindowControl;
		case 0x4B: return _state.ObjWindowControl;

		case 0x50: return _state.BlendMainControl;
		case 0x51: return _state.BlendSubControl;
		case 0x52: return _state.BlendMainCoefficient;
		case 0x53: return _state.BlendSubCoefficient;

		default:
			LogDebug("Read unimplemented LCD register: " + HexUtilities::ToHex32(addr));
			break;
	}

	return _memoryManager->GetOpenBus(addr);
}

void GbaPpu::DebugProcessMemoryAccessView()
{
	//Store memory access buffer in ppu tools to display in tilemap viewer
	GbaPpuTools* ppuTools = ((GbaPpuTools*)_emu->InternalGetDebugger()->GetPpuTools(CpuType::Gba));
	ppuTools->SetMemoryAccessData(_state.Scanline, _memoryAccess);
}

void GbaPpu::Serialize(Serializer& s)
{
	SV(_state.FrameCount);
	SV(_state.Cycle);
	SV(_state.Scanline);

	SV(_state.Control);
	SV(_state.BgMode);
	SV(_state.DisplayFrameSelect);
	SV(_state.AllowHblankOamAccess);
	SV(_state.ObjVramMappingOneDimension);
	SV(_state.ForcedBlank);
	SV(_state.GreenSwapEnabled);

	SV(_state.Control2);
	SV(_state.ObjEnableTimer);
	SV(_state.ObjLayerEnabled);
	SV(_state.Window0Enabled);
	SV(_state.Window1Enabled);
	SV(_state.ObjWindowEnabled);

	SV(_state.DispStat);
	SV(_state.VblankIrqEnabled);
	SV(_state.HblankIrqEnabled);
	SV(_state.ScanlineIrqEnabled);
	SV(_state.Lyc);

	SV(_state.BlendMainControl);
	SVArray(_state.BlendMain, 6);
	SV(_state.BlendSubControl);
	SVArray(_state.BlendSub, 6);
	SV(_state.BlendEffect);
	SV(_state.BlendMainCoefficient);
	SV(_state.BlendSubCoefficient);
	SV(_state.Brightness);

	for(int i = 0; i < 4; i++) {
		SVI(_state.BgLayers[i].Control);
		SVI(_state.BgLayers[i].TilemapAddr);
		SVI(_state.BgLayers[i].TilesetAddr);
		SVI(_state.BgLayers[i].ScrollX);
		SVI(_state.BgLayers[i].ScrollXLatch);
		SVI(_state.BgLayers[i].ScrollY);
		SVI(_state.BgLayers[i].ScreenSize);
		SVI(_state.BgLayers[i].DoubleWidth);
		SVI(_state.BgLayers[i].DoubleHeight);
		SVI(_state.BgLayers[i].Priority);
		SVI(_state.BgLayers[i].Mosaic);
		SVI(_state.BgLayers[i].WrapAround);
		SVI(_state.BgLayers[i].Bpp8Mode);
		SVI(_state.BgLayers[i].Enabled);
		SVI(_state.BgLayers[i].EnableTimer);
	}
	
	for(int i = 0; i < 2; i++) {
		SVI(_state.Transform[i].OriginX);
		SVI(_state.Transform[i].OriginY);
		SVI(_state.Transform[i].LatchOriginX);
		SVI(_state.Transform[i].LatchOriginY);
		SVI(_state.Transform[i].PendingUpdateX);
		SVI(_state.Transform[i].PendingUpdateY);
		SVI(_state.Transform[i].Matrix[0]);
		SVI(_state.Transform[i].Matrix[1]);
		SVI(_state.Transform[i].Matrix[2]);
		SVI(_state.Transform[i].Matrix[3]);

		SVI(_state.Window[i].LeftX);
		SVI(_state.Window[i].RightX);
		SVI(_state.Window[i].TopY);
		SVI(_state.Window[i].BottomY);
	}

	SV(_state.BgMosaicSizeX);
	SV(_state.BgMosaicSizeY);
	SV(_state.ObjMosaicSizeX);
	SV(_state.ObjMosaicSizeY);

	SV(_state.Window0Control);
	SV(_state.Window1Control);
	SV(_state.ObjWindowControl);
	SV(_state.OutWindowControl);

	if(s.GetFormat() != SerializeFormat::Map) {
		for(int i = 0; i < 5; i++) {
			SVI(_state.WindowActiveLayers[i][0]);
			SVI(_state.WindowActiveLayers[i][1]);
			SVI(_state.WindowActiveLayers[i][2]);
			SVI(_state.WindowActiveLayers[i][3]);
			SVI(_state.WindowActiveLayers[i][4]);
			SVI(_state.WindowActiveLayers[i][5]);
		}

		//Convert data to plain arrays to improve serialization performance
		GbaPixelData* src[6] = { _oamOutputBuffers[0], _oamOutputBuffers[1], _layerOutput[0], _layerOutput[1], _layerOutput[2], _layerOutput[3] };
		string names[6] = { "oamOutputBuffers[0]", "oamOutputBuffers[1]", "layerOutput[0]", "layerOutput[1]", "layerOutput[2]", "layerOutput[3]" };
		for(int i = 0; i < 6; i++) {
			GbaPixelData* data = src[i];
			uint16_t color[GbaConstants::ScreenWidth];
			uint8_t layer[GbaConstants::ScreenWidth];
			uint8_t priority[GbaConstants::ScreenWidth];
			if(s.IsSaving()) {
				for(int j = 0; j < GbaConstants::ScreenWidth; j++) {
					color[j] = data[j].Color;
					layer[j] = data[j].Layer;
					priority[j] = data[j].Priority;
				}
			}
			s.StreamArray(color, GbaConstants::ScreenWidth, (names[i] + "_color").c_str());
			s.StreamArray(layer, GbaConstants::ScreenWidth, (names[i] + "_layer").c_str());
			s.StreamArray(priority, GbaConstants::ScreenWidth, (names[i] + "_priority").c_str());
			if(!s.IsSaving()) {
				for(int j = 0; j < GbaConstants::ScreenWidth; j++) {
					data[j].Color = color[j];
					data[j].Layer = layer[j];
					data[j].Priority = priority[j];
				}
			}
		}

		SVArray(_oamWindow, GbaConstants::ScreenWidth);
		SVArray(_activeWindow, GbaConstants::ScreenWidth);
		SVArray(_memoryAccess, 308 * 4);

		SV(_triggerSpecialDma);

		SV(_lastWindowCycle);
		SV(_window0ActiveY);
		SV(_window1ActiveY);
		SV(_window0ActiveX);
		SV(_window1ActiveX);

		SV(_lastRenderCycle);
		SV(_evalOamIndex);
		SV(_loadOamTileCounter);
		SV(_oamHasWindowModeSprite);
		SV(_loadOamAttr01);
		SV(_loadOamAttr2);
		SV(_isFirstOamTileLoad);
		SV(_loadObjMatrix);
		SV(_oamScanline);
		SV(_oamMosaicY);

		for(int i = 0; i < 4; i++) {
			SVI(_layerData[i].TilemapData);
			SVI(_layerData[i].TileData);
			SVI(_layerData[i].FetchAddr);
			SVI(_layerData[i].TileIndex);
			SVI(_layerData[i].RenderX);
			SVI(_layerData[i].TransformX);
			SVI(_layerData[i].TransformY);
			SVI(_layerData[i].XPos);
			SVI(_layerData[i].YPos);
			SVI(_layerData[i].PaletteIndex);
			SVI(_layerData[i].HoriMirror);
			SVI(_layerData[i].VertMirror);
			SVI(_layerData[i].TileRow);
			SVI(_layerData[i].TileColumn);
			SVI(_layerData[i].MosaicColor);
		}

		for(int i = 0; i < 2; i++) {
			SVI(_objData[i].MatrixData[0]);
			SVI(_objData[i].MatrixData[1]);
			SVI(_objData[i].MatrixData[2]);
			SVI(_objData[i].MatrixData[3]);
			SVI(_objData[i].XValue);
			SVI(_objData[i].YValue);
			SVI(_objData[i].XPos);
			SVI(_objData[i].YPos);
			SVI(_objData[i].CenterX);
			SVI(_objData[i].CenterY);

			SVI(_objData[i].SpriteX);
			SVI(_objData[i].TileIndex);
			SVI(_objData[i].Addr);
			SVI(_objData[i].RenderX);
			SVI(_objData[i].SpriteY);
			SVI(_objData[i].YOffset);
			SVI(_objData[i].TransformEnabled);
			SVI(_objData[i].DoubleSize);
			SVI(_objData[i].HideSprite);

			SVI(_objData[i].Mode);
			SVI(_objData[i].Mosaic);
			SVI(_objData[i].Bpp8Mode);
			SVI(_objData[i].Shape);
			SVI(_objData[i].HoriMirror);
			SVI(_objData[i].VertMirror);
			SVI(_objData[i].TransformParamSelect);
			SVI(_objData[i].Size);

			SVI(_objData[i].Priority);
			SVI(_objData[i].PaletteIndex);

			SVI(_objData[i].Width);
			SVI(_objData[i].Height);
		}
	}

	if(!s.IsSaving()) {
		UpdateWindowChangePoints();
	}
}
