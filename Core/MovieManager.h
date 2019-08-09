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
	static shared_ptr<IMovie> _player;
	static shared_ptr<MovieRecorder> _recorder;

public:
	static void Record(RecordMovieOptions options, shared_ptr<Console> console);
	static void Play(VirtualFile file, shared_ptr<Console> console);
	static void Stop();
	static bool Playing();
	static bool Recording();
};