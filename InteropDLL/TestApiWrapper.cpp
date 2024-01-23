#include "Common.h"
#include "Core/Shared/RecordedRomTest.h"
#include "Core/Shared/Emulator.h"
#include "Core/Shared/EmuSettings.h"

extern unique_ptr<Emulator> _emu;
shared_ptr<RecordedRomTest> _recordedRomTest;

extern "C"
{
	DllExport RomTestResult __stdcall RunRecordedTest(char* filename, bool inBackground)
	{
		if(inBackground) {
			unique_ptr<Emulator> emu(new Emulator());
			emu->Initialize();
			emu->GetSettings()->SetFlag(EmulationFlags::ConsoleMode);
			shared_ptr<RecordedRomTest> romTest(new RecordedRomTest(emu.get(), true));
			return romTest->Run(filename);
		} else {
			shared_ptr<RecordedRomTest> romTest(new RecordedRomTest(_emu.get(), false));
			return romTest->Run(filename);
		}
	}

	DllExport uint32_t __stdcall RunTest(char* filename)
	{
		unique_ptr<Emulator> emu(new Emulator());
		emu->Initialize();
		emu->GetSettings()->SetFlag(EmulationFlags::ConsoleMode);
		emu->GetSettings()->GetGameboyConfig().Model = GameboyModel::Gameboy;
		emu->GetSettings()->GetGameboyConfig().RamPowerOnState = RamState::AllZeros;
		emu->LoadRom((VirtualFile)filename, VirtualFile());
		emu->GetSettings()->SetFlag(EmulationFlags::MaximumSpeed);

		while(emu->GetFrameCount() < 500) {
			std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(50));
		}

		uint8_t result = ((uint8_t*)emu->GetMemory(MemoryType::GbHighRam).Memory)[2];

		emu->Stop(false);
		emu->Release();

		return result;
	}

	DllExport void __stdcall RomTestRecord(char* filename, bool reset)
	{
		_recordedRomTest.reset(new RecordedRomTest(_emu.get(), false));
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