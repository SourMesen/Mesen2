#pragma once
#include "pch.h"
#include "GBA/GbaTypes.h"
#include "Shared/Emulator.h"
#include "Utilities/Timer.h"
#include "Utilities/ISerializable.h"

class Emulator;
class GbaConsole;
class GbaMemoryManager;

struct GbaLayerRendererData
{
	uint16_t TilemapData;
	uint16_t TileData;
	uint16_t FetchAddr;

	uint16_t TileIndex;
	int16_t RenderX;

	int32_t TransformX;
	int32_t TransformY;
	uint32_t XPos;
	uint32_t YPos;

	uint8_t PaletteIndex;

	bool HoriMirror;
	bool VertMirror;

	uint8_t TileRow;
	uint8_t TileColumn;
	uint8_t MosaicColor;
};

struct GbaSpriteRendererData
{
	int16_t MatrixData[4];
	int32_t XValue;
	int32_t YValue;
	uint32_t XPos;
	uint32_t YPos;
	int16_t CenterX;
	int16_t CenterY;

	uint16_t SpriteX;
	uint16_t TileIndex;
	uint16_t Addr;
	uint8_t RenderX;
	uint8_t SpriteY;
	uint8_t YOffset;
	bool TransformEnabled;
	bool DoubleSize;
	bool HideSprite;

	GbaPpuObjMode Mode;
	bool Mosaic;
	bool Bpp8Mode;
	uint8_t Shape;
	bool HoriMirror;
	bool VertMirror;
	uint8_t TransformParamSelect;
	uint8_t Size;

	uint8_t Priority;
	uint8_t PaletteIndex;

	uint8_t Width;
	uint8_t Height;
};

struct GbaPixelData
{
	uint16_t Color = 0;
	uint8_t Priority = 0xFF;
	uint8_t Layer = 5;
};

class GbaPpu final : public ISerializable
{
private:
	static constexpr int SpriteLayerIndex = 4;
	static constexpr int BackdropLayerIndex = 5;
	static constexpr int EffectLayerIndex = 5;
	static constexpr uint16_t DirectColorFlag = 0x8000;
	static constexpr uint16_t SpriteBlendFlag = 0x4000;
	static constexpr uint16_t SpriteMosaicFlag = 0x2000;
	
	static constexpr uint8_t Window0 = 0;
	static constexpr uint8_t Window1 = 1;
	static constexpr uint8_t ObjWindow = 2;
	static constexpr uint8_t OutsideWindow = 3;
	static constexpr uint8_t NoWindow = 4;

	static constexpr uint16_t BlackColor = 0;
	static constexpr uint16_t WhiteColor = 0x7FFF;

	GbaPpuState _state = {};

	Emulator* _emu = nullptr;
	GbaConsole* _console = nullptr;
	GbaMemoryManager* _memoryManager = nullptr;

	uint8_t* _vram = nullptr;
	uint16_t* _vram16 = nullptr;
	uint32_t* _oam = nullptr;
	uint16_t* _paletteRam = nullptr;

	uint16_t* _outputBuffers[2] = {};
	uint16_t* _currentBuffer = nullptr;

	GbaPixelData* _oamWriteOutput = nullptr;
	GbaPixelData* _oamReadOutput = nullptr;

	GbaPixelData _oamOutputBuffers[2][GbaConstants::ScreenWidth] = {};
	GbaPixelData _layerOutput[4][GbaConstants::ScreenWidth] = {};
	
	uint8_t _oamWindow[GbaConstants::ScreenWidth] = {};
	uint8_t _activeWindow[256] = {};

	Timer _frameSkipTimer;
	bool _skipRender = false;
	
	bool _triggerSpecialDma = false;

	int16_t _lastWindowCycle = -1;
	bool _window0ActiveY = false;
	bool _window1ActiveY = false;
	bool _window0ActiveX = false;
	bool _window1ActiveX = false;
	uint8_t _windowChangePos[4] = {};

	int16_t _lastRenderCycle = -1;
	GbaLayerRendererData _layerData[4] = {};
	GbaSpriteRendererData _objData[2] = {};
	
	int16_t _evalOamIndex = -1;
	int16_t _loadOamTileCounter = 0;
	int16_t _oamLastCycle = -1;
	bool _oamHasWindowModeSprite = false;
	bool _loadOamAttr01 = false;
	bool _loadOamAttr2 = false;
	bool _isFirstOamTileLoad = false;
	uint8_t _loadObjMatrix = 0;
	uint8_t _oamScanline = 0;
	uint8_t _oamMosaicY = 0;

	uint8_t _memoryAccess[308 * 4] = {};
	
	typedef void(GbaPpu::*Func)();
	Func _colorMathFunc[128] = {};

	uint16_t _skippedOutput[240];

	template<int i, bool windowEnabled> __forceinline void ProcessLayerPixel(int x, uint8_t wnd, GbaPixelData& main, GbaPixelData& sub)
	{
		if constexpr(windowEnabled) {
			if(!_state.WindowActiveLayers[wnd][i]) {
				return;
			}
		}
		
		if(_layerOutput[i][x].Priority < main.Priority) {
			sub = main;
			main = _layerOutput[i][x];
		} else if(_layerOutput[i][x].Priority < sub.Priority) {
			sub = _layerOutput[i][x];
		}
	}

	template<GbaPpuBlendEffect effect, bool bg0Enabled, bool bg1Enabled, bool bg2Enabled, bool bg3Enabled, bool windowEnabled> void ProcessColorMath();

	void BlendColors(uint16_t* dst, int x, uint16_t a, uint8_t aCoeff, uint16_t b, uint8_t bCoeff);
	template<bool isSubColor> uint16_t ReadColor(int x, uint16_t addr);

	void InitializeWindows();
	void ProcessWindow();
	void SetWindowX(uint8_t& regValue, uint8_t newValue);
	void UpdateWindowChangePoints();
	void SetWindowActiveLayers(int window, uint8_t cfg);

	void SendFrame();
	
	template<int i, bool mosaic, bool bpp8> __forceinline void PushBgPixels();
	template<int i, bool mosaic, bool bpp8, bool mirror> __forceinline void PushBgPixels();

	template<int layer> void RenderTilemap();
	template<int layer, bool mosaic, bool bpp8> void RenderTilemap();

	template<int layer> void RenderTransformTilemap();
	template<int mode> void RenderBitmapMode();
	
	__forceinline void SetPixelData(GbaPixelData& pixel, uint16_t color, uint8_t priority, uint8_t layer);

	void ProcessSprites();
	
	__noinline void InitSpriteEvaluation();
	void AddVisibleSprite(uint32_t sprData);
	void LoadSpriteAttr2();
	void LoadSpriteTransformMatrix();

	template<bool transformEnabled, bool blockFirst16k> __forceinline void RenderSprite(GbaSpriteRendererData& spr);
	template<bool blockFirst16k> void RenderSprites();
	template<bool blockFirst16k> __forceinline uint8_t ReadSpriteVram(uint32_t addr);

	template<int bit> void SetTransformOrigin(uint8_t i, uint8_t value, bool setY);
	void SetLayerEnabled(int layer, bool enabled);

	void ProcessEndOfScanline();
	void ProcessHBlank();

	void DebugProcessMemoryAccessView();

public:
	void Init(Emulator* emu, GbaConsole* console, GbaMemoryManager* memoryManager);
	~GbaPpu();

	__forceinline void Exec()
	{
		_state.Cycle++;

		if(_state.Cycle == 308*4) {
			ProcessEndOfScanline();
		} else if(_state.Cycle == 1006) {
			ProcessHBlank();
		}

		_emu->ProcessPpuCycle<CpuType::Gba>();
	}

	bool IsAccessingMemory(uint8_t memType)
	{
		return _memoryAccess[_state.Cycle] & memType;
	}

	void RenderScanline(bool forceRender = false);
	
	void DebugSendFrame();

	void WriteRegister(uint32_t addr, uint8_t value);
	uint8_t ReadRegister(uint32_t addr);

	uint16_t* GetScreenBuffer() { return _currentBuffer; }
	uint16_t* GetPreviousScreenBuffer() { return _currentBuffer == _outputBuffers[0] ? _outputBuffers[1] : _outputBuffers[0]; }

	GbaPpuState& GetState() { return _state; }
	uint32_t GetFrameCount() { return _state.FrameCount; }
	int32_t GetScanline() { return _state.Scanline; }
	uint32_t GetCycle() { return _state.Cycle; }
	bool IsBitmapMode() { return _state.BgMode >= 3; }
	
	void Serialize(Serializer& s) override;
};