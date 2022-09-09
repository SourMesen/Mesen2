#pragma once

#include "pch.h"
#include <unordered_set>

class FolderUtilities
{
private:
	static string _homeFolder;
	static string _saveFolderOverride;
	static string _saveStateFolderOverride;
	static string _firmwareFolderOverride;
	static string _screenshotFolderOverride;
	static vector<string> _gameFolders;

public:
	static void SetHomeFolder(string homeFolder);
	static string GetHomeFolder();

	static void SetFolderOverrides(string saveFolder, string saveStateFolder, string screenshotFolder, string firmwareFolder);

	static void AddKnownGameFolder(string gameFolder);
	static vector<string> GetKnownGameFolders();

	static string GetSaveFolder();
	static string GetFirmwareFolder();
	static string GetSaveStateFolder();
	static string GetScreenshotFolder();
	static string GetHdPackFolder();
	static string GetDebuggerFolder();
	static string GetRecentGamesFolder();

	static vector<string> GetFolders(string rootFolder);
	static vector<string> GetFilesInFolder(string rootFolder, std::unordered_set<string> extensions, bool recursive);

	static string GetFilename(string filepath, bool includeExtension);
	static string GetExtension(string filename);
	static string GetFolderName(string filepath);

	static void CreateFolder(string folder);

	static string CombinePath(string folder, string filename);
};