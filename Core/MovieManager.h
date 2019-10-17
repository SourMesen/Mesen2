#pragma once
#include "stdafx.h"
#include "MessageManager.h"
#include "IInputProvider.h"
#include "MovieTypes.h"

class MovieRecorder;
class VirtualFile;
class Console;

class IMovie : public IInputProvider
{
public:
	virtual bool Play(VirtualFile &file) = 0;
	virtual bool IsPlaying() = 0;
};

class MovieManager
{
private:
	shared_ptr<Console> _console;
	shared_ptr<IMovie> _player;
	shared_ptr<MovieRecorder> _recorder;

public:
	MovieManager(shared_ptr<Console> console);

	void Record(RecordMovieOptions options);
	void Play(VirtualFile file, bool silent = false);
	void Stop();
	bool Playing();
	bool Recording();
};