#pragma once
#include "pch.h"
#include "Shared/MessageManager.h"
#include "Shared/Interfaces/IInputProvider.h"
#include "Shared/Movies/MovieTypes.h"
#include "Shared/Movies/MovieRecorder.h"
#include "Utilities/safe_ptr.h"

class VirtualFile;
class Emulator;

class IMovie : public IInputProvider
{
public:
	virtual ~IMovie() = default;

	virtual bool Play(VirtualFile& file) = 0;
	virtual void Stop() = 0;
	virtual bool IsPlaying() = 0;
};

class MovieManager
{
private:
	Emulator* _emu = nullptr;
	safe_ptr<IMovie> _player;
	safe_ptr<MovieRecorder> _recorder;

public:
	MovieManager(Emulator* emu);

	void Record(RecordMovieOptions options);
	void Play(VirtualFile file, bool silent = false);
	void Stop();
	bool Playing();
	bool Recording();
};