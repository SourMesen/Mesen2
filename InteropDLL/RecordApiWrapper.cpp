#include "stdafx.h"
#include "../Core/Console.h"
#include "../Core/VideoRenderer.h"
#include "../Core/SoundMixer.h"

extern shared_ptr<Console> _console;
enum class VideoCodec;

extern "C"
{
	DllExport void __stdcall AviRecord(char* filename, VideoCodec codec, uint32_t compressionLevel) { _console->GetVideoRenderer()->StartRecording(filename, codec, compressionLevel); }
	DllExport void __stdcall AviStop() { _console->GetVideoRenderer()->StopRecording(); }
	DllExport bool __stdcall AviIsRecording() { return _console->GetVideoRenderer()->IsRecording(); }

	DllExport void __stdcall WaveRecord(char* filename) { _console->GetSoundMixer()->StartRecording(filename); }
	DllExport void __stdcall WaveStop() { _console->GetSoundMixer()->StopRecording(); }
	DllExport bool __stdcall WaveIsRecording() { return _console->GetSoundMixer()->IsRecording(); }
}