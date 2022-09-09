#include "Common.h"
#include "Core/Shared/RecordedRomTest.h"
#include "Core/Shared/Emulator.h"

extern unique_ptr<Emulator> _emu;
shared_ptr<RecordedRomTest> _recordedRomTest;

extern "C"
{
	DllExport int32_t __stdcall RunRecordedTest(char* filename, bool inBackground)
	{
		if(inBackground) {
			unique_ptr<Emulator> emu(new Emulator());
			emu->Initialize();
			shared_ptr<RecordedRomTest> romTest(new RecordedRomTest(emu.get()));
			return romTest->Run(filename);
		} else {
			shared_ptr<RecordedRomTest> romTest(new RecordedRomTest(_emu.get()));
			return romTest->Run(filename);
		}
	}

	DllExport void __stdcall RomTestRecord(char* filename, bool reset)
	{
		_recordedRomTest.reset(new RecordedRomTest(_emu.get()));
		_recordedRomTest->Record(filename, reset);
	}
	
	DllExport void __stdcall RomTestStop()
	{
		if(_recordedRomTest) {
			_recordedRomTest->Stop();
			_recordedRomTest.reset();
		}
	}

	DllExport bool __stdcall RomTestRecording() { return _recordedRomTest != nullptr; }
}