#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_set>
#include <filesystem>

namespace fs = std::filesystem;
using std::string;
using std::vector;

extern "C" {
	void __stdcall PgoRunTest(vector<string> testRoms, bool enableDebugger);
}

vector<string> GetFilesInFolder(string rootFolder, std::unordered_set<string> extensions)
{
	vector<string> files;
	vector<string> folders = { { rootFolder } };

	std::error_code errorCode;
	if(!fs::is_directory(fs::u8path(rootFolder), errorCode)) {
		return files;
	}

	for(string folder : folders) {
		for(fs::directory_iterator i(fs::u8path(folder.c_str())), end; i != end; i++) {
			string extension = i->path().extension().u8string();
			std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
			if(extensions.find(extension) != extensions.end()) {
				files.push_back(i->path().u8string());
			}
		}
	}

	return files;
}

int main(int argc, char* argv[])
{
	string romFolder = "../PGOGames";
	if(argc >= 2) {
		romFolder = argv[1];
	}

	vector<string> testRoms = GetFilesInFolder(romFolder, { {".sfc"} });
	PgoRunTest(testRoms, true);
	return 0;
}

