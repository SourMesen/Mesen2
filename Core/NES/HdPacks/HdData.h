#pragma once
#include "pch.h"
#include "NES/NesConstants.h"
#include "Shared/MessageManager.h"
#include "Utilities/PNGHelper.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/SimpleLock.h"
#include "Utilities/Timer.h"

class BaseHdNesPack;

struct HdTileKey
{
	static constexpr int32_t NoTile = -1;

	//Code depends on these 2 fields being one after the other
	uint32_t PaletteColors = 0;
	uint8_t TileData[16] = {};

	int32_t TileIndex = 0;
	bool IsChrRamTile = false;

	HdTileKey GetKey(bool defaultKey)
	{
		if(defaultKey) {
			HdTileKey copy = *this;
			copy.PaletteColors = 0xFFFFFFFF;
			return copy;
		} else {
			return *this;
		}
	}

	uint32_t GetHashCode() const
	{
		if(IsChrRamTile) {
			return CalculateHash((uint8_t*)&PaletteColors, sizeof(PaletteColors) + sizeof(TileData));
		} else {
			return (uint32_t)TileIndex ^ PaletteColors;
		}
	}

	size_t operator() (const HdTileKey &tile) const {
		return tile.GetHashCode();
	}

	bool operator==(const HdTileKey &other) const
	{
		if(IsChrRamTile) {
			return memcmp((uint8_t*)&PaletteColors, (uint8_t*)&other.PaletteColors, sizeof(PaletteColors) + sizeof(TileData)) == 0;
		} else {
			return TileIndex == other.TileIndex && PaletteColors == other.PaletteColors;
		}
	}

	uint32_t CalculateHash(const uint8_t* key, size_t len) const
	{
		uint32_t result = 0;
		for(size_t i = 0; i < len; i += 4) {
			uint32_t chunk;
			memcpy(&chunk, key, sizeof(uint32_t));

			result += chunk;
			result = (result << 2) | (result >> 30);
			key += 4;
		}
		return result;
	}

	bool IsSpriteTile()
	{
		return (PaletteColors & 0xFF000000) == 0xFF000000;
	}
};

namespace std {
	template <> struct hash<HdTileKey>
	{
		size_t operator()(const HdTileKey& x) const
		{
			return x.GetHashCode();
		}
	};
}

struct HdPpuTileInfo : public HdTileKey
{
	uint8_t OffsetX = 0;
	uint8_t OffsetY = 0;
	bool HorizontalMirroring = false;
	bool VerticalMirroring = false;
	bool BackgroundPriority = false;
	
	uint8_t BgColorIndex = 0;
	uint8_t SpriteColorIndex = 0;
	uint8_t BgColor = 0;
	uint8_t SpriteColor = 0;
	uint8_t PpuBackgroundColor = 0;
	uint8_t PaletteOffset = 0;
};

struct HdPpuPixelInfo
{
	HdPpuTileInfo Tile = {};
	HdPpuTileInfo Sprite[4] = {};
	
	uint16_t TmpVideoRamAddr = 0;
	uint8_t XScroll = 0;
	uint8_t EmphasisBits = 0;
	bool Grayscale = false;
	uint8_t SpriteCount = 0;
};

struct HdScreenInfo
{
	HdPpuPixelInfo* ScreenTiles;
	unordered_map<uint32_t, uint8_t> WatchedAddressValues;
	uint32_t FrameNumber = 0;

	HdScreenInfo(const HdScreenInfo& that) = delete;

	HdScreenInfo(bool isChrRamGame)
	{
		ScreenTiles = new HdPpuPixelInfo[NesConstants::ScreenPixelCount];

		for(int i = 0; i < NesConstants::ScreenPixelCount; i++) {
			ScreenTiles[i].Tile.BackgroundPriority = false;
			ScreenTiles[i].Tile.IsChrRamTile = isChrRamGame;
			ScreenTiles[i].Tile.HorizontalMirroring = false;
			ScreenTiles[i].Tile.VerticalMirroring = false;

			for(int j = 0; j < 4; j++) {
				ScreenTiles[i].Sprite[j].IsChrRamTile = isChrRamGame;
			}
		}
	}

	~HdScreenInfo()
	{
		delete[] ScreenTiles;
	}
};

enum class HdPackConditionType
{
	HMirror,
	VMirror,
	BgPriority,
	FrameRange,
	MemoryCheck,
	MemoryCheckConstant,
	TileNearby,
	TileAtPos,
	SpriteAtPos,
	SpriteNearby,
	SpritePalette,
	PositionCheckX,
	PositionCheckY,
	OriginPositionCheckX,
	OriginPositionCheckY,
};

struct HdPackCondition
{
protected:
	HdScreenInfo* _screenInfo = nullptr;
	BaseHdNesPack* _hdPack = nullptr;

public:
	string Name;

	virtual HdPackConditionType GetConditionType() = 0;
	virtual string GetConditionName() = 0;
	virtual bool IsExcludedFromFile() { return Name.size() > 0 && Name[0] == '!'; }
	virtual string ToString() = 0;

	virtual ~HdPackCondition() { }

	void Initialize(HdScreenInfo* screenInfo, BaseHdNesPack* hdPack)
	{
		_screenInfo = screenInfo;
		_hdPack = hdPack;
		_resultCache = -1;
	}

	bool CheckCondition(int x, int y, HdPpuTileInfo* tile)
	{
		if(_resultCache >= 0) {
			return (bool)_resultCache;
		}

		bool result = InternalCheckCondition(x, y, tile);
		if(Name[0] == '!') {
			result = !result;
		}

		if(_useCache) {
			_resultCache = result ? 1 : 0;
		}
		return result;
		
	}

protected:
	int8_t _resultCache = -1;
	bool _useCache = false;

	virtual bool InternalCheckCondition(int x, int y, HdPpuTileInfo* tile) = 0;
};

struct HdPackBitmapInfo
{
private:
	bool _initDone = false;
	SimpleLock _lock;

public:
	string PngName;
	vector<uint8_t> FileData;
	vector<uint32_t> PixelData;
	uint32_t Width;
	uint32_t Height;

	void Init()
	{
		if(_initDone) {
			return;
		}

		auto lock = _lock.AcquireSafe();
		if(_initDone) {
			return;
		}

		//Timer tmr;
		if(PNGHelper::ReadPNG(FileData, PixelData, Width, Height)) {
			//MessageManager::Log("[HDPack] PNG file loaded: " + PngName + " (" + std::to_string(tmr.GetElapsedMS()) + ")");
			PremultiplyAlpha();
		} else {
			MessageManager::Log("[HDPack] PNG file " + PngName + " is invalid.");
		}
		FileData = {};
		_initDone = true;
	}

	void PremultiplyAlpha()
	{
		for(size_t i = 0; i < PixelData.size(); i++) {
			if(PixelData[i] < 0xFF000000) {
				uint8_t* output = (uint8_t*)(PixelData.data() + i);
				uint8_t alpha = output[3] + 1;
				output[0] = (uint8_t)((alpha * output[0]) >> 8);
				output[1] = (uint8_t)((alpha * output[1]) >> 8);
				output[2] = (uint8_t)((alpha * output[2]) >> 8);
			}
		}
	}
};

struct HdPackTileInfo : public HdTileKey
{
private:
	bool _needInit = true;

public:
	uint32_t X;
	uint32_t Y;
	uint32_t Width;
	uint32_t Height;
	uint32_t BitmapIndex;
	HdPackBitmapInfo* Bitmap;
	int Brightness;
	bool DefaultTile;
	bool Blank;
	bool HasTransparentPixels;
	bool TransparencyRequired;
	bool IsFullyTransparent;
	vector<uint32_t> HdTileData;
	uint32_t ChrBankId;

	vector<HdPackCondition*> Conditions;
	bool ForceDisableCache;

	bool MatchesCondition(int x, int y, HdPpuTileInfo* tile)
	{
		for(HdPackCondition* condition : Conditions) {
			if(!condition->CheckCondition(x, y, tile)) {
				return false;
			}
		}
		return true;
	}

	vector<uint32_t> ToRgb(uint32_t* palette)
	{
		vector<uint32_t> rgbBuffer;
		for(uint8_t i = 0; i < 8; i++) {
			uint8_t lowByte = TileData[i];
			uint8_t highByte = TileData[i + 8];
			for(uint8_t j = 0; j < 8; j++) {
				uint8_t color = ((lowByte >> (7 - j)) & 0x01) | (((highByte >> (7 - j)) & 0x01) << 1);
				uint32_t rgbColor;
				if(IsSpriteTile() || TransparencyRequired) {
					rgbColor = color == 0 ? 0x00FFFFFF : palette[(PaletteColors >> ((3 - color) * 8)) & 0x3F];
				} else {
					rgbColor = palette[(PaletteColors >> ((3 - color) * 8)) & 0x3F];
				}
				rgbBuffer.push_back(rgbColor);
			}
		}

		return rgbBuffer;
	}

	void UpdateFlags()
	{
		Blank = true;
		HasTransparentPixels = false;
		IsFullyTransparent = true;
		for(size_t i = 0; i < HdTileData.size(); i++) {
			if(HdTileData[i] != HdTileData[0]) {
				Blank = false;
			}
			if((HdTileData[i] & 0xFF000000) != 0xFF000000) {
				HasTransparentPixels = true;
			}
			if(HdTileData[i] & 0xFF000000) {
				IsFullyTransparent = false;
			}
		}
	}

	__forceinline bool NeedInit()
	{
		return _needInit;
	}

	__noinline void Init()
	{
		_needInit = false;
		Bitmap->Init();

		uint32_t bitmapOffset = Y * Bitmap->Width + X;
		uint32_t* pngData = Bitmap->PixelData.data();

		HdTileData.resize(Width * Height);
		if(Bitmap->PixelData.size() >= bitmapOffset + ((Height - 1) * Bitmap->Width) + Width) {
			for(uint32_t y = 0; y < Height; y++) {
				memcpy(HdTileData.data() + (y * Width), pngData + bitmapOffset, Width * sizeof(uint32_t));
				bitmapOffset += Bitmap->Width;
			}
		}

		UpdateFlags();
	}

	string ToString(int pngIndex)
	{
		stringstream out;

		if(Conditions.size() > 0) {
			out << "[";
			for(size_t i = 0; i < Conditions.size(); i++) {
				if(i > 0) {
					out << "&";
				}
				out << Conditions[i]->Name;
			}
			out << "]";
		}

		if(IsChrRamTile) {
			out << "<tile>" << pngIndex << ",";

			for(int i = 0; i < 16; i++) {
				out << HexUtilities::ToHex(TileData[i]);
			}
			out << "," <<
				HexUtilities::ToHex(PaletteColors, true) << "," <<
				X << "," <<
				Y << "," <<
				(double)Brightness / 255 << "," <<
				(DefaultTile ? "Y" : "N") << "," <<
				ChrBankId << "," <<
				TileIndex;
		} else {
			out << "<tile>" <<
				pngIndex << "," <<
				HexUtilities::ToHex(TileIndex) << "," <<
				HexUtilities::ToHex(PaletteColors, true) << "," <<
				X << "," <<
				Y << "," <<
				(double)Brightness / 255 << "," <<
				(DefaultTile ? "Y" : "N");
		}

		return out.str();
	}
};

enum class HdPackBlendMode
{
	Alpha,
	Add,
	Subtract
};

struct HdBackgroundInfo
{
	HdPackBitmapInfo* Data;
	int Brightness;
	vector<HdPackCondition*> Conditions;
	float HorizontalScrollRatio;
	float VerticalScrollRatio;
	uint8_t Priority;

	uint32_t Left;
	uint32_t Top;

	HdPackBlendMode BlendMode;

	uint32_t* data()
	{
		return Data->PixelData.data();
	}

	string ToString()
	{
		stringstream out;

		if(Conditions.size() > 0) {
			out << "[";
			for(size_t i = 0; i < Conditions.size(); i++) {
				if(i > 0) {
					out << "&";
				}
				out << Conditions[i]->Name;
			}
			out << "]";
		}
		
		out << "<background>";
		out << Data->PngName << ",";
		out << (Brightness / 255.0);

		return out.str();
	}
};

struct HdPackAdditionalSpriteInfo
{
	HdTileKey OriginalTile;
	HdTileKey AdditionalTile;

	int32_t OffsetX;
	int32_t OffsetY;
	bool IgnorePalette;
};

struct FallbackTileInfo
{
	int32_t TileIndex;
	int32_t FallbackTileIndex;
};

struct BgmTrackInfo
{
	string Filename;
	uint32_t LoopPosition = 0;
};

struct HdPackData
{
private:
	bool _cancelLoad = false;

public:
	static constexpr int BgLayerCount = 40;

	vector<HdBackgroundInfo> BackgroundsByPriority[HdPackData::BgLayerCount];
	vector<unique_ptr<HdPackBitmapInfo>> BackgroundFileData;
	vector<unique_ptr<HdPackBitmapInfo>> ImageFileData;
	vector<unique_ptr<HdPackTileInfo>> Tiles;
	vector<unique_ptr<HdPackCondition>> Conditions;
	vector<HdPackAdditionalSpriteInfo> AdditionalSprites;
	vector<FallbackTileInfo> FallbackTiles;
	unordered_set<uint32_t> WatchedMemoryAddresses;
	unordered_map<HdTileKey, vector<HdPackTileInfo*>> TileByKey;
	unordered_map<string, string> PatchesByHash;
	unordered_map<int, BgmTrackInfo> BgmFilesById;
	unordered_map<int, string> SfxFilesById;
	vector<uint32_t> Palette;

	bool HasOverscanConfig = false;
	OverscanDimensions Overscan;

	uint32_t Scale = 1;
	uint32_t Version = 0;
	uint32_t OptionFlags = 0;

	HdPackData() { }
	~HdPackData() { }

	HdPackData(const HdPackData&) = delete;
	HdPackData& operator=(const HdPackData&) = delete;

	void LoadAsync()
	{
		for(auto& bitmap : BackgroundFileData) {
			bitmap->Init();
			if(_cancelLoad) {
				return;
			}
		}
		for(auto& bitmap : ImageFileData) {
			bitmap->Init();
			if(_cancelLoad) {
				return;
			}
		}
	}

	void CancelLoad()
	{
		_cancelLoad = true;
	}
};

enum class HdPackOptions
{
	None = 0,
	NoSpriteLimit = 1,
	AlternateRegisterRange = 2,
	DisableCache = 8,
	DontRenderOriginalTiles = 16,
	AutomaticFallbackTiles = 32
};