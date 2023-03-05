#include "pch.h"
#include <algorithm>
#include <unordered_map>
#include "NES/HdPacks/HdPackLoader.h"
#include "NES/HdPacks/HdPackConditions.h"
#include "NES/HdPacks/HdNesPack.h"
#include "NES/NesConsole.h"
#include "Shared/MessageManager.h"
#include "Utilities/ZipReader.h"
#include "Utilities/FolderUtilities.h"
#include "Utilities/StringUtilities.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/PNGHelper.h"
#include "Utilities/FastString.h"

#define checkConstraint(x, y) if(!(x)) { MessageManager::Log(y); return; }

HdPackLoader::HdPackLoader()
{
}

bool HdPackLoader::InitializeLoader(VirtualFile &romFile, HdPackData *data)
{
	_data = data;

	string romName = FolderUtilities::GetFilename(romFile.GetFileName(), false);
	string hdPackFolder = FolderUtilities::GetHdPackFolder();
	string zipName = romName + ".hdn";
	string definitionPath = FolderUtilities::CombinePath(romName, "hires.txt");

	string legacyPath = FolderUtilities::CombinePath(hdPackFolder, definitionPath);
	if(ifstream(legacyPath)) {
		_loadFromZip = false;
		_hdPackFolder = FolderUtilities::GetFolderName(legacyPath);
		return true;
	} else {
		vector<string> hdnPackages = FolderUtilities::GetFilesInFolder(romFile.GetFolderPath(), { ".hdn" }, false);
		vector<string> more = FolderUtilities::GetFilesInFolder(hdPackFolder, { ".hdn", ".zip" }, false);
		hdnPackages.insert(hdnPackages.end(), more.begin(), more.end());

		if(hdnPackages.size() == 0) {
			//Prevent calculating rom SHA-1 when no potential HD pack file exists
			return false;
		}

		string sha1Hash = romFile.GetSha1Hash();
		for(string path : hdnPackages) {
			_reader.LoadArchive(path);

			vector<uint8_t> hdDefinition;
			if(_reader.ExtractFile("hires.txt", hdDefinition)) {
				if(FolderUtilities::GetFilename(path, false) == romName) {
					_loadFromZip = true;
					_hdPackFolder = path;
					return true;
				} else {
					for(string line : StringUtilities::Split(string(hdDefinition.data(), hdDefinition.data() + hdDefinition.size()), '\n')) {
						std::transform(line.begin(), line.end(), line.begin(), ::tolower);
						if(line.find("<supportedrom>") != string::npos && line.find(sha1Hash) != string::npos) {
							_loadFromZip = true;
							_hdPackFolder = path;
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}

bool HdPackLoader::LoadHdNesPack(string definitionFile, HdPackData &outData)
{
	HdPackLoader loader;
	if(ifstream(definitionFile)) {
		loader._data = &outData;
		loader._loadFromZip = false;
		loader._hdPackFolder = FolderUtilities::GetFolderName(definitionFile);
		return loader.LoadPack();
	}
	return false;
}

bool HdPackLoader::LoadHdNesPack(VirtualFile &romFile, HdPackData &outData)
{
	HdPackLoader loader;
	if(loader.InitializeLoader(romFile, &outData)) {
		return loader.LoadPack();
	}
	return false;
}

bool HdPackLoader::CheckFile(string filename)
{
	if(_loadFromZip) {
		return _reader.CheckFile(filename);
	} else {
		ifstream file(FolderUtilities::CombinePath(_hdPackFolder, filename), ios::in | ios::binary);
		if(file.good()) {
			return true;
		}
	}

	return false;
}

bool HdPackLoader::LoadFile(string filename, vector<uint8_t> &fileData)
{
	fileData.clear();

	if(_loadFromZip) {
		if(_reader.ExtractFile(filename, fileData)) {
			return true;
		}
	} else {
		ifstream file(FolderUtilities::CombinePath(_hdPackFolder, filename), ios::in | ios::binary);
		if(file.good()) {
			file.seekg(0, ios::end);
			uint32_t fileSize = (uint32_t)file.tellg();
			file.seekg(0, ios::beg);

			fileData.resize(fileSize);
			file.read((char*)fileData.data(), fileSize);
			
			return true;
		}
	}

	return false;
}

bool HdPackLoader::LoadPack()
{
	string lineContent;
	try {
		vector<uint8_t> hdDefinition;
		if(!LoadFile("hires.txt", hdDefinition)) {
			return false;
		}

		InitializeGlobalConditions();

		size_t len = hdDefinition.size();
		size_t pos = 0;
		while(pos < len) {
			lineContent.clear();

			size_t start = pos;
			for(; pos < len; pos++) {
				if(hdDefinition[pos] == '\n') {
					pos++;
					break;
				}
			}

			lineContent.insert(0, (char*)hdDefinition.data() + start, pos < len ? (pos - start - 1) : (len - start));

			if(lineContent.empty()) {
				continue;
			}

			if(lineContent[lineContent.size() - 1] == '\r') {
				lineContent = lineContent.substr(0, lineContent.size() - 1);
			}

			vector<HdPackCondition*> conditions;
			if(lineContent.substr(0, 1) == "[") {
				size_t endOfCondition = lineContent.find_first_of(']', 1);
				if(endOfCondition == string::npos) {
					MessageManager::Log("[HDPack] Invalid condition tag: " + lineContent);
					continue;
				}
				conditions = ParseConditionString(lineContent.substr(1, endOfCondition - 1));
				lineContent = lineContent.substr(endOfCondition + 1);
			}

			vector<string> tokens;
			if(lineContent.substr(0, 6) == "<tile>") {
				tokens = StringUtilities::Split(lineContent.substr(6), ',');
				ProcessTileTag(tokens, conditions);
			} else if(lineContent.substr(0, 12) == "<background>") {
				tokens = StringUtilities::Split(lineContent.substr(12), ',');
				ProcessBackgroundTag(tokens, conditions);
			} else if(lineContent.substr(0, 11) == "<condition>") {
				tokens = StringUtilities::Split(lineContent.substr(11), ',');
				ProcessConditionTag(tokens, false);
				ProcessConditionTag(tokens, true);
			} else if(lineContent.substr(0, 5) == "<img>") {
				lineContent = lineContent.substr(5);
				if(!ProcessImgTag(lineContent)) {
					return false;
				}
			} else if(lineContent.substr(0, 10) == "<addition>") {
				tokens = StringUtilities::Split(lineContent.substr(10), ',');
				ProcessAdditionTag(tokens);
			} else if(lineContent.substr(0, 5) == "<bgm>") {
				tokens = StringUtilities::Split(lineContent.substr(5), ',');
				ProcessBgmTag(tokens);
			} else if(lineContent.substr(0, 5) == "<sfx>") {
				tokens = StringUtilities::Split(lineContent.substr(5), ',');
				ProcessSfxTag(tokens);
			} else if(lineContent.substr(0, 5) == "<ver>") {
				_data->Version = stoi(lineContent.substr(5));
				if(_data->Version > BaseHdNesPack::CurrentVersion) {
					MessageManager::Log("[HDPack] This HD Pack was built with a more recent version of Mesen - update Mesen to the latest version and try again.");
					return false;
				}
			} else if(lineContent.substr(0, 7) == "<scale>") {
				lineContent = lineContent.substr(7);
				_data->Scale = std::stoi(lineContent);
				if(_data->Scale > 10) {
					MessageManager::Log("[HDPack] Scale ratios higher than 10 are not supported.");
					return false;
				}
			} else if(lineContent.substr(0, 10) == "<overscan>") {
				tokens = StringUtilities::Split(lineContent.substr(10), ',');
				ProcessOverscanTag(tokens);
			} else if(lineContent.substr(0, 7) == "<patch>") {
				tokens = StringUtilities::Split(lineContent.substr(7), ',');
				ProcessPatchTag(tokens);
			} else if(lineContent.substr(0, 9) == "<options>") {
				tokens = StringUtilities::Split(lineContent.substr(9), ',');
				ProcessOptionTag(tokens);
			}
		}

		LoadCustomPalette();
		InitializeHdPack();

		return true;
	} catch(std::exception &ex) {
		MessageManager::Log(string("[HDPack] Error loading HDPack: ") + ex.what() + " on line: " + lineContent);
		return false;
	}
}

bool HdPackLoader::ProcessImgTag(string src)
{
	_data->ImageFileData.push_back(unique_ptr<HdPackBitmapInfo>(new HdPackBitmapInfo()));
	HdPackBitmapInfo& bitmapInfo = *_data->ImageFileData.back().get();

	if(!LoadFile(src, bitmapInfo.FileData)) {
		_data->ImageFileData.pop_back();
		MessageManager::Log("[HDPack] Error loading HDPack: PNG file " + src + " could not be read.");
		return false;
	}
	bitmapInfo.PngName = src;
	return true;
}

template<typename T>
void HdPackLoader::AddGlobalCondition(string name) {
	T* cond = new T();
	cond->Name = name;
	_data->Conditions.push_back(unique_ptr<HdPackCondition>(cond));
	_conditionsByName[name] = cond;

	name = "!" + name;
	cond = new T();
	cond->Name = name;
	_data->Conditions.push_back(unique_ptr<HdPackCondition>(cond));
	_conditionsByName[name] = cond;
}

void HdPackLoader::InitializeGlobalConditions()
{
	AddGlobalCondition<HdPackHorizontalMirroringCondition>("hmirror");
	AddGlobalCondition<HdPackVerticalMirroringCondition>("vmirror");
	AddGlobalCondition<HdPackBgPriorityCondition>("bgpriority");
	
	if(_data->Version >= 107) {
		AddGlobalCondition<HdPackSpritePaletteCondition<0>>("sppalette0");
		AddGlobalCondition<HdPackSpritePaletteCondition<1>>("sppalette1");
		AddGlobalCondition<HdPackSpritePaletteCondition<2>>("sppalette2");
		AddGlobalCondition<HdPackSpritePaletteCondition<3>>("sppalette3");
	}
}

void HdPackLoader::ReadTileData(HdTileKey& key, string& tileData, string& palData)
{
	if(tileData.size() >= 32) {
		//CHR RAM tile, read the tile data
		for(int i = 0; i < 16; i++) {
			key.TileData[i] = HexUtilities::FromHex(tileData.substr(i * 2, 2));
		}
		key.TileIndex = -1;
		key.IsChrRamTile = true;
	} else {
		if(_data->Version <= 102) {
			key.TileIndex = std::stoi(tileData);
		} else {
			key.TileIndex = HexUtilities::FromHex(tileData);
		}
		key.IsChrRamTile = false;
	}

	key.PaletteColors = HexUtilities::FromHex(palData);
}

void HdPackLoader::ProcessOverscanTag(vector<string> &tokens)
{
	OverscanDimensions overscan;
	overscan.Top = std::stoi(tokens[0]);
	overscan.Right = std::stoi(tokens[1]);
	overscan.Bottom = std::stoi(tokens[2]);
	overscan.Left = std::stoi(tokens[3]);
	_data->HasOverscanConfig = true;
	_data->Overscan = overscan;
}

void HdPackLoader::ProcessPatchTag(vector<string> &tokens)
{
	checkConstraint(tokens.size() >= 2, "[HDPack] Patch tag requires more parameters");
	checkConstraint(tokens[1].size() == 40, string("[HDPack] Invalid SHA1 hash for patch (" + tokens[0] + "): " + tokens[1]));

	vector<uint8_t> fileData;
	if(!LoadFile(tokens[0], fileData)) {
		MessageManager::Log(string("[HDPack] Patch file not found: " + tokens[1]));
		return;
	}

	std::transform(tokens[1].begin(), tokens[1].end(), tokens[1].begin(), ::toupper);
	if(_loadFromZip) {
		_data->PatchesByHash[tokens[1]] = VirtualFile(_hdPackFolder, tokens[0]);
	} else {
		_data->PatchesByHash[tokens[1]] = FolderUtilities::CombinePath(_hdPackFolder, tokens[0]);
	}
}

void HdPackLoader::ProcessTileTag(vector<string> &tokens, vector<HdPackCondition*> conditions)
{
	HdPackTileInfo *tileInfo = new HdPackTileInfo();
	size_t index = 0;
	if(_data->Version < 100) {
		tileInfo->TileIndex = std::stoi(tokens[index++]);
		tileInfo->BitmapIndex = std::stoi(tokens[index++]);
		tileInfo->PaletteColors = std::stoi(tokens[index + 2]) | (std::stoi(tokens[index + 1]) << 8) | (std::stoi(tokens[index]) << 16);
		index += 3;
	} else {
		tileInfo->BitmapIndex = std::stoi(tokens[index++]);

		string tileData = tokens[index++];
		string palData = tokens[index++];
		ReadTileData(*tileInfo, tileData, palData);
	}
	tileInfo->X = std::stoi(tokens[index++]);
	tileInfo->Y = std::stoi(tokens[index++]);
	tileInfo->Conditions = conditions;
	tileInfo->ForceDisableCache = false;
	for(HdPackCondition* condition : conditions) {
		HdPackConditionType type = condition->GetConditionType();
		switch(type){
			case HdPackConditionType::SpriteNearby: tileInfo->ForceDisableCache = true; break;
			case HdPackConditionType::TileNearby:
				HdPackTileNearbyCondition* tileNearby = (HdPackTileNearbyCondition*)condition;
				if(tileNearby->TileX % 8 > 0 || tileNearby->TileY % 8 > 0) {
					tileInfo->ForceDisableCache = true;
				}
				break;
		}
	}

	if(_data->Version >= 105) {
		tileInfo->Brightness = (int)(std::stof(tokens[index++]) * 255);
	} else if(_data->Version > 0) {
		tileInfo->Brightness = (uint8_t)(std::stof(tokens[index++]) * 255);
	} else {
		tileInfo->Brightness = 255;
	}
	tileInfo->DefaultTile = (tokens[index++] == "Y");

	//For CHR ROM tiles, the ID is just the bank number in chr rom (4k banks)
	tileInfo->ChrBankId = tileInfo->TileIndex / 256;

	if(_data->Version < 100) {
		if(tokens.size() >= 24) {
			//CHR RAM tile, read the tile data
			for(int i = 0; i < 16; i++) {
				tileInfo->TileData[i] = std::stoi(tokens[index++]);
			}
			tileInfo->IsChrRamTile = true;
		} else {
			tileInfo->IsChrRamTile = false;
		}
	} else {
		if(tileInfo->IsChrRamTile && tokens.size() > index) {
			tileInfo->ChrBankId = std::stoul(tokens[index++]);
		}
		if(tileInfo->IsChrRamTile && tokens.size() > index) {
			tileInfo->TileIndex = std::stoi(tokens[index++]);
		}
	}

	checkConstraint(tileInfo->BitmapIndex < _data->ImageFileData.size(), "[HDPack] Invalid bitmap index: " + std::to_string(tileInfo->BitmapIndex));

	tileInfo->Bitmap = _data->ImageFileData[tileInfo->BitmapIndex].get();
	tileInfo->Width = 8 * _data->Scale;
	tileInfo->Height = 8 * _data->Scale;

	_data->Tiles.push_back(unique_ptr<HdPackTileInfo>(tileInfo));
}

void HdPackLoader::ProcessOptionTag(vector<string> &tokens)
{
	for(string token : tokens) {
		if(token == "disableSpriteLimit") {
			_data->OptionFlags |= (int)HdPackOptions::NoSpriteLimit;
		} else if(token == "alternateRegisterRange") {
			_data->OptionFlags |= (int)HdPackOptions::AlternateRegisterRange;
		} else if(token == "disableCache") {
			_data->OptionFlags |= (int)HdPackOptions::DisableCache;
		} else if(token == "disableOriginalTiles") {
			_data->OptionFlags |= (int)HdPackOptions::DontRenderOriginalTiles;
		} else {
			MessageManager::Log("[HDPack] Invalid option: " + token);
		}
	}
}

void HdPackLoader::ProcessConditionTag(vector<string> &tokens, bool createInvertedCondition)
{
	checkConstraint(tokens.size() >= 4, "[HDPack] Condition tag should contain at least 4 parameters");
	checkConstraint(tokens[0].size() > 0, "[HDPack] Condition name may not be empty");
	checkConstraint(tokens[0].find_last_of('!') == string::npos, "[HDPack] Condition name may not contain '!' characters");

	unique_ptr<HdPackCondition> condition;
	
	if(tokens[1] == "tileAtPosition") {
		condition.reset(new HdPackTileAtPositionCondition());
	} else if(tokens[1] == "tileNearby") {
		condition.reset(new HdPackTileNearbyCondition());
	} else if(tokens[1] == "spriteAtPosition") {
		condition.reset(new HdPackSpriteAtPositionCondition());
	} else if(tokens[1] == "spriteNearby") {
		condition.reset(new HdPackSpriteNearbyCondition());
	} else if(tokens[1] == "memoryCheck" || tokens[1] == "ppuMemoryCheck") {
		condition.reset(new HdPackMemoryCheckCondition());
	} else if(tokens[1] == "memoryCheckConstant" || tokens[1] == "ppuMemoryCheckConstant") {
		condition.reset(new HdPackMemoryCheckConstantCondition());
	} else if(tokens[1] == "frameRange") {
		condition.reset(new HdPackFrameRangeCondition());
	} else {
		MessageManager::Log("[HDPack] Invalid condition type: " + tokens[1]);
		return;
	}

	tokens[0].erase(tokens[0].find_last_not_of(" \n\r\t") + 1);
	condition->Name = tokens[0];

	if(createInvertedCondition) {
		condition->Name = "!" + condition->Name;
	}

	int index = 2;
	switch(condition->GetConditionType()) {
		case HdPackConditionType::TileNearby:
		case HdPackConditionType::TileAtPos:
		case HdPackConditionType::SpriteNearby:
		case HdPackConditionType::SpriteAtPos: {
			checkConstraint(tokens.size() >= 6, "[HDPack] Condition tag should contain at least 6 parameters");

			int x = std::stoi(tokens[index++]);
			int y = std::stoi(tokens[index++]);
			string token = tokens[index++];
			int32_t tileIndex = -1;
			string tileData;
			if(token.size() == 32) {
				tileData = token;
			} else {
				if(_data->Version < 104) {
					tileIndex = std::stoi(token);
				} else {
					//Tile indexes are stored as hex starting from version 104+
					tileIndex = HexUtilities::FromHex(token);
				}
			}
			uint32_t palette = HexUtilities::FromHex(tokens[index++]);

			((HdPackBaseTileCondition*)condition.get())->Initialize(x, y, palette, tileIndex, tileData);
			break;
		}

		case HdPackConditionType::MemoryCheck:
		case HdPackConditionType::MemoryCheckConstant: {
			checkConstraint(_data->Version >= 101, "[HDPack] This feature requires version 101+ of HD Packs");
			checkConstraint(tokens.size() >= 5, "[HDPack] Condition tag should contain at least 5 parameters");
		
			bool usePpuMemory = tokens[1].substr(0, 3) == "ppu";
			uint32_t operandA = HexUtilities::FromHex(tokens[index++]);

			if(usePpuMemory) {
				checkConstraint(operandA <= 0x3FFF, "[HDPack] Out of range memoryCheck operand");
				operandA |= HdPackBaseMemoryCondition::PpuMemoryMarker;
			} else {
				checkConstraint(operandA <= 0xFFFF, "[HDPack] Out of range memoryCheck operand");
			}

			HdPackConditionOperator op;
			string opString = tokens[index++];
			if(opString == "==") {
				op = HdPackConditionOperator::Equal;
			} else if(opString == "!=") {
				op = HdPackConditionOperator::NotEqual;
			} else if(opString == ">") {
				op = HdPackConditionOperator::GreaterThan;
			} else if(opString == "<") {
				op = HdPackConditionOperator::LowerThan;
			} else if(opString == "<=") {
				op = HdPackConditionOperator::LowerThanOrEqual;
			} else if(opString == ">=") {
				op = HdPackConditionOperator::GreaterThanOrEqual;
			} else {
				checkConstraint(false, "[HDPack] Invalid operator.");
			}

			uint32_t operandB = HexUtilities::FromHex(tokens[index++]);
			uint32_t mask = 0xFF;
			if(tokens.size() > 5 && _data->Version >= 103) {
				checkConstraint(operandB <= 0xFF, "[HDPack] Out of range memoryCheck mask");
				mask = HexUtilities::FromHex(tokens[index++]);
			}

			switch(condition->GetConditionType()) {
				case HdPackConditionType::MemoryCheck:
					if(usePpuMemory) {
						checkConstraint(operandB <= 0x3FFF, "[HDPack] Out of range memoryCheck operand");
						operandB |= HdPackBaseMemoryCondition::PpuMemoryMarker;
					} else {
						checkConstraint(operandB <= 0xFFFF, "[HDPack] Out of range memoryCheck operand");
					}
					_data->WatchedMemoryAddresses.emplace(operandB);
					break;

				case HdPackConditionType::MemoryCheckConstant:
					checkConstraint(operandB <= 0xFF, "[HDPack] Out of range memoryCheckConstant operand");
					break;
			}

			_data->WatchedMemoryAddresses.emplace(operandA);
			((HdPackBaseMemoryCondition*)condition.get())->Initialize(operandA, op, operandB, (uint8_t)mask);
			break;
		}

		case HdPackConditionType::FrameRange: {
			checkConstraint(_data->Version >= 101, "[HDPack] This feature requires version 101+ of HD Packs");
			checkConstraint(tokens.size() >= 4, "[HDPack] Condition tag should contain at least 4 parameters");

			int32_t operandA;
			int32_t operandB;
			if(_data->Version == 101) {
				operandA = HexUtilities::FromHex(tokens[index++]);
				operandB = HexUtilities::FromHex(tokens[index++]);
			} else {
				//Version 102+
				operandA = std::stoi(tokens[index++]);
				operandB = std::stoi(tokens[index++]);
			}

			checkConstraint(operandA >= 0 && operandA <= 0xFFFF, "[HDPack] Out of range frameRange operand");
			checkConstraint(operandB >= 0 && operandB <= 0xFFFF, "[HDPack] Out of range frameRange operand");

			((HdPackFrameRangeCondition*)condition.get())->Initialize(operandA, operandB);
			break;
		}
	}
	
	HdPackCondition *cond = condition.get();
	condition.release();
	_data->Conditions.emplace_back(unique_ptr<HdPackCondition>(cond));
	_conditionsByName[cond->Name] = cond;
}

void HdPackLoader::ProcessBackgroundTag(vector<string> &tokens, vector<HdPackCondition*> conditions)
{
	checkConstraint(tokens.size() >= 2, "[HDPack] Background tag should contain at least 2 parameters");
	HdPackBitmapInfo* bgFileData = nullptr;

	auto result = _backgroundsByName.find(tokens[0]);
	if(result == _backgroundsByName.end()) {
		_data->BackgroundFileData.push_back(unique_ptr<HdPackBitmapInfo>(new HdPackBitmapInfo()));
		bgFileData = _data->BackgroundFileData.back().get();
		bgFileData->PngName = tokens[0];

		if(!LoadFile(bgFileData->PngName, bgFileData->FileData)) {
			bgFileData = nullptr;
			_data->BackgroundFileData.pop_back();
		} else {
			_backgroundsByName[tokens[0]] = bgFileData;
		}
	} else {
		bgFileData = result->second;
	}

	HdBackgroundInfo backgroundInfo;
	if(bgFileData) {
		backgroundInfo.Data = bgFileData;
		if (_data->Version >= 105) {
			backgroundInfo.Brightness = (int)(std::stof(tokens[1]) * 255);
		} else {
			backgroundInfo.Brightness = (uint8_t)(std::stof(tokens[1]) * 255);
		}
		backgroundInfo.HorizontalScrollRatio = 0;
		backgroundInfo.VerticalScrollRatio = 0;
		backgroundInfo.Priority = 10;
		backgroundInfo.Left = 0;
		backgroundInfo.Top = 0;
		backgroundInfo.Conditions.reserve(conditions.size());

		for(HdPackCondition* condition : conditions) {
			switch(condition->GetConditionType()) {
				case HdPackConditionType::TileAtPos:
				case HdPackConditionType::SpriteAtPos:
				case HdPackConditionType::MemoryCheck:
				case HdPackConditionType::MemoryCheckConstant:
				case HdPackConditionType::FrameRange:
					backgroundInfo.Conditions.push_back(condition);
					break;

				default:
					MessageManager::Log("[HDPack] Invalid condition type for background: " + tokens[0]);
					return;
			}
		}

		if(tokens.size() > 2) {
			checkConstraint(_data->Version >= 101, "[HDPack] This feature requires version 101+ of HD Packs");

			backgroundInfo.HorizontalScrollRatio = std::stof(tokens[2]);
			if(tokens.size() > 3) {
				backgroundInfo.VerticalScrollRatio = std::stof(tokens[3]);
			}
			if(tokens.size() > 4) {
				checkConstraint(_data->Version >= 102, "[HDPack] This feature requires version 102+ of HD Packs");
				if(_data->Version >= 106) {
					backgroundInfo.Priority = std::stoi(tokens[4]);
					checkConstraint(backgroundInfo.Priority >= 0 && backgroundInfo.Priority < 40, "[HDPack] Invalid background priority value");
				} else {
					backgroundInfo.Priority = tokens[4] == "Y" ? 0 : 10;
				}
			}
			if(tokens.size() > 6) {
				checkConstraint(_data->Version >= 105, "[HDPack] This feature requires version 105+ of HD Packs");
				backgroundInfo.Left = std::max(0, std::stoi(tokens[5]));
				backgroundInfo.Top = std::max(0, std::stoi(tokens[6]));
			}
		}

		_data->Backgrounds.push_back(backgroundInfo);
	} else {
		MessageManager::Log("[HDPack] Error while loading background: " + tokens[0]);
	}
}

void HdPackLoader::ProcessAdditionTag(vector<string>& tokens)
{
	checkConstraint(_data->Version >= 107, "[HDPack] This feature requires version 107+ of HD Packs");

	_data->AdditionalSprites.push_back({});

	HdPackAdditionalSpriteInfo& info = _data->AdditionalSprites.back();
	ReadTileData(info.OriginalTile, tokens[0], tokens[1]);
	info.OffsetX = std::stoi(tokens[2]);
	info.OffsetY = std::stoi(tokens[3]);
	ReadTileData(info.AdditionalTile, tokens[4], tokens[5]);
}

int HdPackLoader::ProcessSoundTrack(string albumString, string trackString, string filename)
{
	int album = std::stoi(albumString);
	if(album < 0 || album > 255) {
		MessageManager::Log("[HDPack] Invalid album value: " + albumString);
		return -1;
	}

	int track = std::stoi(trackString);
	if(track < 0 || track > 255) {
		MessageManager::Log("[HDPack] Invalid track value: " + trackString);
		return -1;
	}

	if(!CheckFile(filename)) {
		MessageManager::Log("[HDPack] OGG file not found: " + filename);
		return -1;
	}

	return album * 256 + track;
}

void HdPackLoader::ProcessBgmTag(vector<string> &tokens)
{
	int trackId = ProcessSoundTrack(tokens[0], tokens[1], tokens[2]);
	if(trackId >= 0) {
		if(_loadFromZip) {
			VirtualFile file(_hdPackFolder, tokens[2]);
			_data->BgmFilesById[trackId] = file;
		} else {
			_data->BgmFilesById[trackId] = FolderUtilities::CombinePath(_hdPackFolder, tokens[2]);
		}
	}
}

void HdPackLoader::ProcessSfxTag(vector<string> &tokens)
{
	int trackId = ProcessSoundTrack(tokens[0], tokens[1], tokens[2]);
	if(trackId >= 0) {
		if(_loadFromZip) {
			VirtualFile file(_hdPackFolder, tokens[2]);
			_data->SfxFilesById[trackId] = file;
		} else {
			_data->SfxFilesById[trackId] = FolderUtilities::CombinePath(_hdPackFolder, tokens[2]);
		}
	}
}

vector<HdPackCondition*> HdPackLoader::ParseConditionString(string conditionString)
{
	FastString conditionName;
	vector<HdPackCondition*> conditions;
	conditions.reserve(3);

	auto processCondition = [&] {
		auto result = _conditionsByName.find(conditionName.ToString());
		if(result != _conditionsByName.end()) {
			conditions.push_back(result->second);
		} else {
			MessageManager::Log("[HDPack] Condition not found: " + string(conditionName.ToString()));
		}
		conditionName.Reset();
	};

	for(size_t i = 0, len = conditionString.size(); i < len; i++) {
		char c = conditionString[i];
		if(c == ' ' || c == '\n' || c == '\r' || c == '\t') {
			continue;
		} else if(c == '&') {
			processCondition();
		} else {
			conditionName.WriteSafe(c);
		}
	}

	processCondition();

	return conditions;
}

void HdPackLoader::LoadCustomPalette()
{
	vector<uint8_t> fileData;
	if(LoadFile("palette.dat", fileData)) {
		vector<uint32_t> paletteData;

		for(size_t i = 0; i < fileData.size(); i+= 3){
			paletteData.push_back(0xFF000000 | (fileData[i] << 16) | (fileData[i+1] << 8) | fileData[i+2]);
		}

		if(paletteData.size() == 0x40) {
			_data->Palette = paletteData;
		}
	}
}

void HdPackLoader::InitializeHdPack()
{
	for(unique_ptr<HdPackTileInfo> &tileInfo : _data->Tiles) {
		auto tiles = _data->TileByKey.find(tileInfo->GetKey(false));
		if(tiles == _data->TileByKey.end()) {
			_data->TileByKey[tileInfo->GetKey(false)] = vector<HdPackTileInfo*>();
		}
		_data->TileByKey[tileInfo->GetKey(false)].push_back(tileInfo.get());

		if(tileInfo->DefaultTile) {
			auto tilesForDefaultKey = _data->TileByKey.find(tileInfo->GetKey(true));
			if(tilesForDefaultKey == _data->TileByKey.end()) {
				_data->TileByKey[tileInfo->GetKey(true)] = vector<HdPackTileInfo*>();
			}
			_data->TileByKey[tileInfo->GetKey(true)].push_back(tileInfo.get());
		}
	}
}
