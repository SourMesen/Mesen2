#pragma once
#include "stdafx.h"
#include "MessageManager.h"
#include "IInputProvider.h"
#include "MovieTypes.h"

class MovieRecorder;
class VirtualFile;
class Emulator;

class IMovie : public IInputProvider
{
public:
	virtual bool Play(VirtualFile &file) = 0;
	virtual bool IsPlaying() = 0;
};

class MovieManager
{
private:
	shared_ptr<Emulator> _emu;
	shared_ptr<IMovie> _player;
	shared_ptr<MovieRecorder> _recorder;

public:
	MovieManager(shared_ptr<Emulator> emu);

	void Record(RecordMovieOptions options);
	void Play(VirtualFile file, bool silent = false);
	void Stop();
	bool Playing();
	bool Recording();
};