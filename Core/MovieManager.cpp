#include "stdafx.h"
#include <algorithm>
#include "Utilities/FolderUtilities.h"
#include "Utilities/VirtualFile.h"
#include "Utilities/ZipReader.h"
#include "Emulator.h"
#include "MovieManager.h"
#include "MesenMovie.h"
#include "MovieRecorder.h"

MovieManager::MovieManager(shared_ptr<Emulator> emu)
{
	_emu = emu;
}

void MovieManager::Record(RecordMovieOptions options)
{
	shared_ptr<MovieRecorder> recorder(new MovieRecorder(_emu));
	if(recorder->Record(options)) {
		_recorder = recorder;
	}
}

void MovieManager::Play(VirtualFile file, bool forTest)
{
	vector<uint8_t> fileData;
	if(file.IsValid() && file.ReadFile(fileData)) {
		shared_ptr<IMovie> player;
		if(memcmp(fileData.data(), "PK", 2) == 0) {
			//Mesen movie
			ZipReader reader;
			reader.LoadArchive(fileData);

			vector<string> files = reader.GetFileList();
			if(std::find(files.begin(), files.end(), "GameSettings.txt") != files.end()) {
				player.reset(new MesenMovie(_emu, forTest));
			}
		}

		if(player && player->Play(file)) {
			_player = player;
			if(!forTest) {
				MessageManager::DisplayMessage("Movies", "MoviePlaying", file.GetFileName());
			}
		}
	}
}

void MovieManager::Stop()
{
	_player.reset();
	_recorder.reset();
}

bool MovieManager::Playing()
{
	shared_ptr<IMovie> player = _player;
	return player && player->IsPlaying();
}

bool MovieManager::Recording()
{
	return _recorder != nullptr;
}
