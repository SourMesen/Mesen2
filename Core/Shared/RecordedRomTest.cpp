#include "pch.h"

#include "Shared/RecordedRomTest.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/MessageManager.h"
#include "Shared/NotificationManager.h"
#include "Shared/Movies/MovieManager.h"
#include "Utilities/VirtualFile.h"
#include "Utilities/FolderUtilities.h"
#include "Utilities/md5.h"
#include "Utilities/ZipWriter.h"
#include "Utilities/ZipReader.h"
#include "Utilities/ArchiveReader.h"

RecordedRomTest::RecordedRomTest(Emulator* emu, bool inBackground)
{
	_emu = emu;
	_inBackground = inBackground;
	Reset();
}

RecordedRomTest::~RecordedRomTest()
{
	Reset();
}

void RecordedRomTest::SaveFrame()
{
	PpuFrameInfo frame = _emu->GetPpuFrame();

	uint8_t md5Hash[16];
	GetMd5Sum(md5Hash, frame.FrameBuffer, frame.FrameBufferSize);

	if(memcmp(_previousHash, md5Hash, 16) == 0 && _currentCount < 255) {
		_currentCount++;
	} else {
		uint8_t* hash = new uint8_t[16];
		memcpy(hash, md5Hash, 16);
		_screenshotHashes.push_back(hash);
		if(_currentCount > 0) {
			_repetitionCount.push_back(_currentCount);
		}
		_currentCount = 1;

		memcpy(_previousHash, md5Hash, 16);

		_signal.Signal();
	}
}

void RecordedRomTest::ValidateFrame()
{
	PpuFrameInfo frame = _emu->GetPpuFrame();

	uint8_t md5Hash[16];
	GetMd5Sum(md5Hash, frame.FrameBuffer, frame.FrameBufferSize);

	if(_currentCount == 0) {
		_currentCount = _repetitionCount.front();
		_repetitionCount.pop_front();
		_screenshotHashes.pop_front();
	}
	_currentCount--;

	if(memcmp(_screenshotHashes.front(), md5Hash, 16) != 0) {
		_badFrameCount++;
		_isLastFrameGood = false;
		//_console->BreakIfDebugging();
	} else {
		_isLastFrameGood = true;
	}
	
	if(_currentCount == 0 && _repetitionCount.empty()) {
		//End of test
		_runningTest = false;
		_signal.Signal();
	}
}

void RecordedRomTest::ProcessNotification(ConsoleNotificationType type, void* parameter)
{
	switch(type) {
		case ConsoleNotificationType::PpuFrameDone:
			if(_recording) {
				SaveFrame();
			} else if(_runningTest) {
				ValidateFrame();
			}
			break;
		
		default:
			break;
	}
}

void RecordedRomTest::Reset()
{
	memset(_previousHash, 0xFF, 16);
	
	_currentCount = 0;
	_repetitionCount.clear();

	for(uint8_t* hash : _screenshotHashes) {
		delete[] hash;
	}
	_screenshotHashes.clear();

	_runningTest = false;
	_recording = false;
	_badFrameCount = 0;
}

void RecordedRomTest::Record(string filename, bool reset)
{
	_emu->GetNotificationManager()->RegisterNotificationListener(shared_from_this());
	_filename = filename;

	string mrtFilename = FolderUtilities::CombinePath(FolderUtilities::GetFolderName(filename), FolderUtilities::GetFilename(filename, false) + ".mrt");
	_file.open(mrtFilename, ios::out | ios::binary);

	if(_file) {
		_emu->Lock();
		Reset();

		EmuSettings* settings = _emu->GetSettings();
		settings->GetSnesConfig().RamPowerOnState = RamState::AllZeros;
		settings->GetNesConfig().RamPowerOnState = RamState::AllZeros;
		settings->GetGameboyConfig().RamPowerOnState = RamState::AllZeros;
		settings->GetPcEngineConfig().RamPowerOnState = RamState::AllZeros;
		settings->GetSmsConfig().RamPowerOnState = RamState::AllZeros;
		settings->GetCvConfig().RamPowerOnState = RamState::AllZeros;
		settings->GetGbaConfig().RamPowerOnState = RamState::AllZeros;

		settings->GetSnesConfig().DisableFrameSkipping = true;
		settings->GetPcEngineConfig().DisableFrameSkipping = true;
		settings->GetGbaConfig().DisableFrameSkipping = true;

		settings->GetGbaConfig().SkipBootScreen = false;
				
		//Start recording movie alongside with screenshots
		RecordMovieOptions options;
		string movieFilename = FolderUtilities::CombinePath(FolderUtilities::GetFolderName(filename), FolderUtilities::GetFilename(filename, false) + ".mmo");
		memcpy(options.Filename, movieFilename.c_str(), std::min(1000, (int)movieFilename.size()));
		options.RecordFrom = reset ? RecordMovieFrom::StartWithSaveData : RecordMovieFrom::CurrentState;
		_emu->GetMovieManager()->Record(options);

		_recording = true;
		_emu->Unlock();
	}
}

RomTestResult RecordedRomTest::Run(string filename)
{
	RomTestResult result = {};
	_emu->GetNotificationManager()->RegisterNotificationListener(shared_from_this());

	EmuSettings* settings = _emu->GetSettings();
	string testName = FolderUtilities::GetFilename(filename, false);
	
	ZipReader zipReader;
	zipReader.LoadArchive(filename);
	vector<string> files = zipReader.GetFileList();
	string romFile = "";
	for(string& file : files) {
		if(file.length() > 7 && file.substr(0, 7) == "TestRom") {
			romFile = file;
		}
	}

	if(romFile.empty()) {
		result.ErrorCode = -4;
		return result;
	}

	VirtualFile testMovie(filename, "TestMovie.mmo");
	VirtualFile testRom(filename, romFile);

	stringstream testData;
	zipReader.GetStream("TestData.mrt", testData);

	if(testData && testMovie.IsValid() && testRom.IsValid()) {
		char header[3];
		testData.read((char*)&header, 3);
		if(memcmp((char*)&header, "MRT", 3) != 0) {
			//Invalid test file
			result.ErrorCode = -3;
			return result;
		}
		
		Reset();

		uint32_t hashCount;
		testData.read((char*)&hashCount, sizeof(uint32_t));
			
		for(uint32_t i = 0; i < hashCount; i++) {
			uint8_t repeatCount = 0;
			testData.read((char*)&repeatCount, sizeof(uint8_t));
			_repetitionCount.push_back(repeatCount);

			uint8_t* screenshotHash = new uint8_t[16];
			testData.read((char*)screenshotHash, 16);
			_screenshotHashes.push_back(screenshotHash);
		}

		_currentCount = _repetitionCount.front();
		_repetitionCount.pop_front();

		if(testName.compare("demo_pal") == 0 || testName.substr(0, 4).compare("pal_") == 0) {
			settings->GetNesConfig().Region = ConsoleRegion::Pal;
		} else {
			settings->GetNesConfig().Region = ConsoleRegion::Auto;
		}

		settings->GetSnesConfig().RamPowerOnState = RamState::AllZeros;
		settings->GetNesConfig().RamPowerOnState = RamState::AllZeros;
		settings->GetGameboyConfig().RamPowerOnState = RamState::AllZeros;
		settings->GetPcEngineConfig().RamPowerOnState = RamState::AllZeros;
		settings->GetSmsConfig().RamPowerOnState = RamState::AllZeros;
		settings->GetCvConfig().RamPowerOnState = RamState::AllZeros;
		settings->GetGbaConfig().RamPowerOnState = RamState::AllZeros;

		settings->GetSnesConfig().DisableFrameSkipping = true;
		settings->GetPcEngineConfig().DisableFrameSkipping = true;
		settings->GetGbaConfig().DisableFrameSkipping = true;
		
		settings->GetGbaConfig().SkipBootScreen = false;

		_emu->Lock();
		//Start playing movie
		if(_emu->LoadRom(testRom, VirtualFile(""))) {
			_emu->GetMovieManager()->Play(testMovie, true);
			settings->SetFlag(EmulationFlags::MaximumSpeed);

			_runningTest = true;
			_emu->Unlock();
			_emu->Resume();
			_signal.Wait();
			_emu->Stop(!_inBackground);
			_runningTest = false;
		} else {
			//Something went wrong when loading the rom
			_emu->Unlock();
			result.ErrorCode = -2;
			return result;
		}

		settings->ClearFlag(EmulationFlags::MaximumSpeed);

		result.ErrorCode = _badFrameCount;
		result.State = _badFrameCount == 0 ? RomTestState::Passed : (_isLastFrameGood ? RomTestState::PassedWithWarnings : RomTestState::Failed);

		return result;
	}

	result.ErrorCode = -1;
	return result;
}

void RecordedRomTest::Stop()
{
	if(_recording) {
		Save();
	}
	Reset();
}

void RecordedRomTest::Save()
{
	//Wait until the next frame is captured to end the recording
	_signal.Wait();
	_repetitionCount.push_back(_currentCount);
	_recording = false;

	//Stop playing/recording the movie
	_emu->GetMovieManager()->Stop();

	_file.write("MRT", 3);

	uint32_t hashCount = (uint32_t)_screenshotHashes.size();
	_file.write((char*)&hashCount, sizeof(uint32_t));
		
	for(uint32_t i = 0; i < hashCount; i++) {
		_file.write((char*)&_repetitionCount[i], sizeof(uint8_t));
		_file.write((char*)&_screenshotHashes[i][0], 16);
	}

	_file.close();

	ZipWriter writer;
	writer.Initialize(_filename);

	string mrtFilename = FolderUtilities::CombinePath(FolderUtilities::GetFolderName(_filename), FolderUtilities::GetFilename(_filename, false) + ".mrt");
	writer.AddFile(mrtFilename, "TestData.mrt");
	std::remove(mrtFilename.c_str());

	string mmoFilename = FolderUtilities::CombinePath(FolderUtilities::GetFolderName(_filename), FolderUtilities::GetFilename(_filename, false) + ".mmo");
	writer.AddFile(mmoFilename, "TestMovie.mmo");
	std::remove(mmoFilename.c_str());

	writer.AddFile(_emu->GetRomInfo().RomFile.GetFilePath(), "TestRom" + _emu->GetRomInfo().RomFile.GetFileExtension());
	
	writer.Save();

	MessageManager::DisplayMessage("Test", "TestFileSavedTo", FolderUtilities::GetFilename(_filename, true));
}