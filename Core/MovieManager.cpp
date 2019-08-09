#include "stdafx.h"
#include "../Utilities/FolderUtilities.h"
#include "../Utilities/VirtualFile.h"
#include "../Utilities/ZipReader.h"
#include "MovieManager.h"
#include "MesenMovie.h"
#include "MovieRecorder.h"

shared_ptr<IMovie> MovieManager::_player;
shared_ptr<MovieRecorder> MovieManager::_recorder;

void MovieManager::Record(RecordMovieOptions options, shared_ptr<Console> console)
{
	shared_ptr<MovieRecorder> recorder(new MovieRecorder(console));
	if(recorder->Record(options)) {
		_recorder = recorder;
	}
}

void MovieManager::Play(VirtualFile file, shared_ptr<Console> console)
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
				player.reset(new MesenMovie(console));
			}
		}

		if(player && player->Play(file)) {
			_player = player;
			MessageManager::DisplayMessage("Movies", "MoviePlaying", file.GetFileName());
		}
	}
}

void MovieManager::Stop()
{
	_player.reset();

	if(_recorder) {
		_recorder->Stop();
		_recorder.reset();
	}
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
