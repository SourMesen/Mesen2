#pragma once
#include "pch.h"
#include "NES/HdPacks/HdData.h"
#include "Utilities/ZipReader.h"
#include "Utilities/VirtualFile.h"

enum class HdPackConditionOperator;

class HdPackLoader
{
public:
	static bool LoadHdNesPack(string definitionFile, HdPackData &outData);
	static bool LoadHdNesPack(VirtualFile &romFile, HdPackData &outData);

private:
	HdPackData* _data = nullptr;
	bool _loadFromZip = false;
	int _currentLine = 0;
	int _errorCount = 0;
	ZipReader _reader;
	string _hdPackDefinitionFile;
	string _hdPackFolder;
	unordered_map<string, HdPackCondition*> _conditionsByName;
	unordered_map<string, HdPackBitmapInfo*> _backgroundsByName;

	HdPackLoader();

	bool InitializeLoader(VirtualFile &romPath, HdPackData *data);
	bool LoadFile(string filename, vector<uint8_t> &fileData);
	bool CheckFile(string filename);

	bool LoadPack();
	void InitializeHdPack();
	void LoadCustomPalette();
	
	void ReadTileData(HdTileKey& key, string& tileData, string& palData);

	template<typename T> void AddGlobalCondition(string name);
	void InitializeGlobalConditions();

	//Video
	bool ProcessImgTag(string src);
	void ProcessPatchTag(vector<string> &tokens);
	void ProcessOverscanTag(vector<string> &tokens);
	void ProcessConditionTag(vector<string> &tokens, bool createInvertedCondition);
	HdPackConditionOperator ParseConditionOperator(string& opString);
	void ProcessTileTag(vector<string> &tokens, vector<HdPackCondition*> conditions);
	void ProcessBackgroundTag(vector<string> &tokens, vector<HdPackCondition*> conditions);
	void ProcessAdditionTag(vector<string>& tokens);
	void ProcessFallbackTag(vector<string>& tokens);
	void ProcessOptionTag(vector<string>& tokens);

	//Audio
	int ProcessSoundTrack(string albumString, string trackString, string filename);
	void ProcessBgmTag(vector<string> &tokens);
	void ProcessSfxTag(vector<string> &tokens);

	vector<HdPackCondition*> ParseConditionString(string conditionString);
	bool ParseBooleanValue(string value);
};