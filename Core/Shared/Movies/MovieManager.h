#pragma once
#include "stdafx.h"
#include "Shared/MessageManager.h"
#include "Shared/Interfaces/IInputProvider.h"
#include "Shared/Movies/MovieTypes.h"
#include "Shared/Movies/MovieRecorder.h"

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
	Emulator* _emu;
	shared_ptr<IMovie> _player;
	shared_ptr<MovieRecorder> _recorder;

public:
	MovieManager(Emulator* emu);

	void Record(RecordMovieOptions options);
	void Play(VirtualFile file, bool silent = false);
	void Stop();
	bool Playing();
	bool Recording();
};