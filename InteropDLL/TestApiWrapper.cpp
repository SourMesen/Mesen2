#include "stdafx.h"
#include "../Core/RecordedRomTest.h"
#include "../Core/Console.h"

extern shared_ptr<Console> _console;
shared_ptr<RecordedRomTest> _recordedRomTest;

extern "C"
{
	DllExport int32_t __stdcall RunRecordedTest(char* filename, bool inBackground)
	{
		shared_ptr<RecordedRomTest> romTest(new RecordedRomTest(inBackground ? nullptr : _console));
		return romTest->Run(filename);
	}

	DllExport void __stdcall RomTestRecord(char* filename, bool reset)
	{
		_recordedRomTest.reset(new RecordedRomTest(_console));
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