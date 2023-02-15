#include "Common.h"
#include "Core/Shared/Emulator.h"
#include "Core/Shared/Video/VideoRenderer.h"
#include "Core/Shared/Audio/SoundMixer.h"
#include "Core/Shared/Movies/MovieManager.h"

extern unique_ptr<Emulator> _emu;

extern "C"
{
	DllExport void __stdcall AviRecord(char* filename, RecordAviOptions options) { _emu->GetVideoRenderer()->StartRecording(filename, options); }
	DllExport void __stdcall AviStop() { _emu->GetVideoRenderer()->StopRecording(); }
	DllExport bool __stdcall AviIsRecording() { return _emu->GetVideoRenderer()->IsRecording(); }

	DllExport void __stdcall WaveRecord(char* filename) { _emu->GetSoundMixer()->StartRecording(filename); }
	DllExport void __stdcall WaveStop() { _emu->GetSoundMixer()->StopRecording(); }
	DllExport bool __stdcall WaveIsRecording() { return _emu->GetSoundMixer()->IsRecording(); }

	DllExport void __stdcall MoviePlay(char* filename) { _emu->GetMovieManager()->Play(string(filename)); }
	DllExport void __stdcall MovieStop() { _emu->GetMovieManager()->Stop(); }
	DllExport bool __stdcall MoviePlaying() { return _emu->GetMovieManager()->Playing(); }
	DllExport bool __stdcall MovieRecording() { return _emu->GetMovieManager()->Recording(); }
	DllExport void __stdcall MovieRecord(RecordMovieOptions options) { _emu->GetMovieManager()->Record(options); }
}